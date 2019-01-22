#include "Engine/Physics/3D/RF/TheCollision.hpp"
#include "Engine/Math/MathUtils.hpp"

void Collision::SetBodies(CollisionRigidBody* first, CollisionRigidBody* second)
{
	m_bodies[0] = first;
	m_bodies[1] = second;
}

void Collision::PrepareInternal(float duration)
{
	if (!m_bodies[0])
		SwapBodies();
	ASSERT_OR_DIE(m_bodies[0] != nullptr, "first body is empty when preparing for collision");

	// basis at contact
	CalculateContactBasis();

	// relative pos between contact and bodies
	m_relative_pos[0] = m_pos - m_bodies[0]->GetCenter();
	if (m_bodies[1]) 
		m_relative_pos[1] = m_pos - m_bodies[1]->GetCenter();

	m_closing_vel = CalculateLocalVelocity(0, duration);
	if (m_bodies[1])
		m_closing_vel -= CalculateLocalVelocity(1, duration);
	
	CalculateDesiredDeltaVelocity(duration);
}

void Collision::SwapBodies()
{
	m_normal *= -1;

	CollisionRigidBody* temp = m_bodies[0];
	m_bodies[0] = m_bodies[1];
	m_bodies[1] = temp;
}

void Collision::CalculateContactBasis()
{
	Vector3 contact_tangent[2];

	if (abs(m_normal.x) > abs(m_normal.y))
	{
		// Scaling factor to ensure the results are normalised
		const float s = 1.0f / sqrtf(m_normal.z * m_normal.z + m_normal.x * m_normal.x);

		// The new X-axis is at right angles to the world Y-axis
		contact_tangent[0].x = m_normal.z * s;
		contact_tangent[0].y = 0;
		contact_tangent[0].z = -m_normal.x * s;

		// The new Y-axis is at right angles to the new X- and Z- axes
		contact_tangent[1].x = m_normal.y * contact_tangent[0].x;
		contact_tangent[1].y = m_normal.z * contact_tangent[0].x - m_normal.x * contact_tangent[0].z;
		contact_tangent[1].z = -m_normal.y * contact_tangent[0].x;
	}
	else
	{
		// Scaling factor to ensure the results are normalised
		const float s = 1.0f / sqrtf(m_normal.z * m_normal.z + m_normal.y * m_normal.y);

		// The new X-axis is at right angles to the world X-axis
		contact_tangent[0].x = 0;
		contact_tangent[0].y = -m_normal.z*s;
		contact_tangent[0].z = m_normal.y*s;

		// The new Y-axis is at right angles to the new X- and Z- axes
		contact_tangent[1].x = m_normal.y * contact_tangent[0].z - m_normal.z*contact_tangent[0].y;
		contact_tangent[1].y = -m_normal.x * contact_tangent[0].z;
		contact_tangent[1].z = m_normal.x * contact_tangent[0].y;
	}

	m_to_world.SetBasis(m_normal, contact_tangent[0], contact_tangent[1]);
}

Vector3 Collision::CalculateLocalVelocity(uint idx, float duration)
{
	CollisionRigidBody* rb = m_bodies[idx];

	Vector3 vel = rb->GetAngularVelocity().Cross(m_relative_pos[idx]);
	vel += rb->GetLinearVelocity();

	Vector3 contact_vel = m_to_world.MultiplyTranspose(vel);

	// force without reaction
	Vector3 acc_induced_vel = rb->GetLastFrameLinearAcc() * duration;

	// velocity in contact coord
	acc_induced_vel = m_to_world.MultiplyTranspose(acc_induced_vel);

	acc_induced_vel.x = 0.f;

	contact_vel += acc_induced_vel;

	return contact_vel;
}

void Collision::CalculateDesiredDeltaVelocity(float duration)
{
	static const float vel_limit = .25f;

	float vel_from_acc = 0.f;

	if (m_bodies[0]->IsAwake())
		vel_from_acc += DotProduct(m_bodies[0]->GetLastFrameLinearAcc() * duration, m_normal);

	if (m_bodies[1] && m_bodies[1]->IsAwake())
		vel_from_acc -= DotProduct(m_bodies[1]->GetLastFrameLinearAcc() * duration, m_normal);

	float real_restitution = m_restitution;
	if (abs(m_closing_vel.x) < vel_limit)
		real_restitution = 0.f;

	m_desired_vel = -m_closing_vel.x - real_restitution * (m_closing_vel.x - vel_from_acc);
}

void Collision::CheckAwakeState()
{
	// Collisions with the world never cause a body to wake up.
	if (!m_bodies[1])
		return;

	bool body0awake = m_bodies[0]->IsAwake();
	bool body1awake = m_bodies[1]->IsAwake();

	// Wake up only the sleeping one
	if (body0awake ^ body1awake)
	{
		if (body0awake) 
			m_bodies[1]->SetAwake(true);
		else 
			m_bodies[0]->SetAwake(true);
	}
}

void Collision::ApplyPositionChange(Vector3 linear[2], Vector3 angular[2], float penetration)
{
	const float angularLimit = 0.2f;
	float angularMove[2];
	float linearMove[2];
	
	float totalInertia = 0;
	float linearInertia[2];
	float angularInertia[2];

	// We need to work out the inertia of each object in the direction
	// of the contact normal, due to angular inertia only.
	for (unsigned i = 0; i < 2; i++) 
	{
		if (m_bodies[i])
		{
			Matrix33 inverseInertiaTensor;
			m_bodies[i]->GetIITWorld(&inverseInertiaTensor);

			// Use the same procedure as for calculating frictionless
			// velocity change to work out the angular inertia.
			Vector3 angularInertiaWorld = m_relative_pos[i].Cross(m_normal);
			angularInertiaWorld = inverseInertiaTensor * angularInertiaWorld;
			angularInertiaWorld = angularInertiaWorld.Cross(m_relative_pos[i]);
			angularInertia[i] = DotProduct(angularInertiaWorld, m_normal);

			// The linear component is simply the inverse mass
			linearInertia[i] = m_bodies[i]->GetInvMass();

			// Keep track of the total inertia from all components
			totalInertia += linearInertia[i] + angularInertia[i];

			// We break the loop here so that the totalInertia value is
			// completely calculated (by both iterations) before
			// continuing.
		}
	}

	// Loop through again calculating and applying the changes
	for (unsigned i = 0; i < 2; i++)
	{
		if (m_bodies[i])
		{
			// The linear and angular movements required are in proportion to
			// the two inverse inertias.
			float sign = (i == 0) ? 1.f:-1.f;
			angularMove[i] = sign * penetration * (angularInertia[i] / totalInertia);
			linearMove[i] = sign * penetration * (linearInertia[i] / totalInertia);

			// To avoid angular projections that are too great (when mass is large
			// but inertia tensor is small) limit the angular move.
			Vector3 projection = m_relative_pos[i];
			projection += (m_normal * (DotProduct(-m_relative_pos[i], m_normal)));

			// Use the small angle approximation for the sine of the angle (i.e.
			// the magnitude would be sine(angularLimit) * projection.magnitude
			// but we approximate sine(angularLimit) to angularLimit).
			float maxMagnitude = angularLimit * projection.GetLength();

			if (angularMove[i] < -maxMagnitude)
			{
				float totalMove = angularMove[i] + linearMove[i];
				angularMove[i] = -maxMagnitude;
				linearMove[i] = totalMove - angularMove[i];
			}
			else if (angularMove[i] > maxMagnitude)
			{
				float totalMove = angularMove[i] + linearMove[i];
				angularMove[i] = maxMagnitude;
				linearMove[i] = totalMove - angularMove[i];
			}

			// We have the linear amount of movement required by turning
			// the rigid body (in angularMove[i]). We now need to
			// calculate the desired rotation to achieve that.
			if (angularMove[i] == 0)
			{
				// Easy case - no angular movement means no rotation.
				angular[i].ToDefault();
			}
			else
			{
				// Work out the direction we'd like to rotate in.
				Vector3 targetAngularDirection = m_relative_pos[i].Cross(m_normal);

				Matrix33 iit;
				m_bodies[i]->GetIITWorld(&iit);

				// Work out the direction we'd need to rotate to achieve that
				angular[i] = iit * targetAngularDirection * (angularMove[i] / angularInertia[i]);
			}

			// Velocity change is easier - it is just the linear movement
			// along the contact normal.
			linear[i] = m_normal * linearMove[i];

			// Now we can start to apply the values we've calculated.
			// Apply the linear movement
			Vector3 pos = m_bodies[i]->GetCenter();
			pos += (m_normal * linearMove[i]);
			m_bodies[i]->SetCenter(pos);

			// And the change in orientation
			Quaternion q = m_bodies[i]->GetOrientation();
			q.AddScaledVector(angular[i], 1.f);		// error: angular move instead of 1?
			m_bodies[i]->SetOrientation(q);

			// We need to calculate the derived data for any body that is
			// asleep, so that the changes are reflected in the object's
			// data. Otherwise the resolution will not change the position
			// of the object, and the next collision detection round will
			// have the same penetration.
			if (!m_bodies[i]->IsAwake()) 
				m_bodies[i]->CacheData();
		}
	}
}

void Collision::ApplyVelocityChange(Vector3 linear[2], Vector3 angular[2])
{
	// Get hold of the inverse mass and inverse inertia tensor, both in
	// world coordinates.
	Matrix33 inverseInertiaTensor[2];
	m_bodies[0]->GetIITWorld(&inverseInertiaTensor[0]);
	if (m_bodies[1])
		m_bodies[1]->GetIITWorld(&inverseInertiaTensor[1]);

	// We will calculate the impulse for each contact axis
	Vector3 impulseContact;

	if (m_friction == 0.f)
	{
		// Use the short format for frictionless contacts
		impulseContact = CalculateFrictionlessImpulse(inverseInertiaTensor);
	}
	else
	{
		// Otherwise we may have impulses that aren't in the direction of the
		// contact, so we need the more complex version.
		impulseContact = CalculateFrictionImpulse(inverseInertiaTensor);
	}

	// Convert impulse to world coordinates
	Vector3 impulse = m_to_world * impulseContact;

	// Split in the impulse into linear and rotational components
	Vector3 impulsiveTorque = m_relative_pos[0].Cross(impulse);
	angular[0] = inverseInertiaTensor[0] * impulsiveTorque;
	linear[0].ToDefault();
	linear[0] += (impulse * m_bodies[0]->GetInvMass());

	// Apply the changes
	m_bodies[0]->AddLinearVelocity(linear[0]);
	m_bodies[0]->AddAngularVelocity(angular[0]);

	if (m_bodies[1])
	{
		// Work out body one's linear and angular changes
		impulsiveTorque = impulse.Cross(m_relative_pos[1]);
		angular[1] = inverseInertiaTensor[1] * impulsiveTorque;
		linear[1].ToDefault();
		linear[1] += (impulse * -m_bodies[1]->GetInvMass());

		// And apply them.
		m_bodies[1]->AddLinearVelocity(linear[1]);
		m_bodies[1]->AddAngularVelocity(angular[1]);
	}
}

Vector3 Collision::CalculateFrictionlessImpulse(Matrix33* iit)
{
	Vector3 impulseContact;

	// Build a vector that shows the change in velocity in
	// world space for a unit impulse in the direction of the contact
	// normal.
	Vector3 deltaVelWorld = m_relative_pos[0].Cross(m_normal);
	deltaVelWorld = iit[0] * deltaVelWorld;
	deltaVelWorld = deltaVelWorld.Cross(m_relative_pos[0]);

	// Work out the change in velocity in contact coordiantes.
	float deltaVelocity = DotProduct(deltaVelWorld, m_normal);

	// Add the linear component of velocity change
	deltaVelocity += m_bodies[0]->GetInvMass();

	// Check if we need to the second body's data
	if (m_bodies[1])
	{
		// Go through the same transformation sequence again
		deltaVelWorld = m_relative_pos[1].Cross(m_normal);
		deltaVelWorld = iit[1] * deltaVelWorld;
		deltaVelWorld = deltaVelWorld.Cross(m_relative_pos[1]);

		// Add the change in velocity due to rotation
		deltaVelocity += DotProduct(deltaVelWorld, m_normal);

		// Add the change in velocity due to linear motion
		deltaVelocity += m_bodies[1]->GetInvMass();
	}

	// Calculate the required size of the impulse
	impulseContact.x = m_desired_vel / deltaVelocity;
	impulseContact.y = 0;
	impulseContact.z = 0;
	return impulseContact;
}

Vector3 Collision::CalculateFrictionImpulse(Matrix33* iit)
{
	Vector3 impulseContact;
	float inverseMass = m_bodies[0]->GetInvMass();

	// The equivalent of a cross product in matrices is multiplication
	// by a skew symmetric matrix - we build the matrix for converting
	// between linear and angular quantities.
	Matrix33 impulseToTorque;
	impulseToTorque.SetSkewSymmetric(m_relative_pos[0]);

	// Build the matrix to convert contact impulse to change in velocity
	// in world coordinates.
	Matrix33 deltaVelWorld = impulseToTorque;
	deltaVelWorld *= iit[0];
	deltaVelWorld *= impulseToTorque;
	deltaVelWorld *= -1;

	// Check if we need to add body two's data
	if (m_bodies[1])
	{
		// Set the cross product matrix
		impulseToTorque.SetSkewSymmetric(m_relative_pos[1]);

		// Calculate the velocity change matrix
		Matrix33 deltaVelWorld2 = impulseToTorque;
		deltaVelWorld2 *= iit[1];
		deltaVelWorld2 *= impulseToTorque;
		deltaVelWorld2 *= -1;

		// Add to the total delta velocity.
		deltaVelWorld += deltaVelWorld2;

		// Add to the inverse mass
		inverseMass += m_bodies[1]->GetInvMass();
	}

	// Do a change of basis to convert into contact coordinates.
	Matrix33 deltaVelocity = m_to_world.Transpose();
	deltaVelocity *= deltaVelWorld;
	deltaVelocity *= m_to_world;

	// Add in the linear velocity change
	deltaVelocity.Ix += inverseMass;
	deltaVelocity.Jy += inverseMass;
	deltaVelocity.Kz += inverseMass;

	// Invert to get the impulse needed per unit velocity
	Matrix33 impulseMatrix = deltaVelocity.Invert();

	// Find the target velocities to kill
	Vector3 velKill(m_desired_vel, -m_closing_vel.y, -m_closing_vel.z);

	// Find the impulse to kill target velocities
	impulseContact = impulseMatrix * velKill;

	// Check for exceeding friction
	float planarImpulse = sqrtf(impulseContact.y*impulseContact.y 
		+ impulseContact.z*impulseContact.z );

	if (planarImpulse > impulseContact.x * m_friction)
	{
		// We need to use dynamic friction
		impulseContact.y /= planarImpulse;
		impulseContact.z /= planarImpulse;

		impulseContact.x = deltaVelocity.Ix +
			deltaVelocity.Jx*m_friction*impulseContact.y +
			deltaVelocity.Kx*m_friction*impulseContact.z;

		impulseContact.x = m_desired_vel / impulseContact.x;
		impulseContact.y *= m_friction * impulseContact.x;
		impulseContact.z *= m_friction * impulseContact.x;
	}

	return impulseContact;
}
