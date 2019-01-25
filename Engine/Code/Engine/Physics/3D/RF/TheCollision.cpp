#include "Engine/Physics/3D/RF/TheCollision.hpp"
#include "Engine/Math/MathUtils.hpp"

void Collision::SetBodies(CollisionRigidBody* first, CollisionRigidBody* second)
{
	m_bodies[0] = first;
	m_bodies[1] = second;
}

void Collision::CacheData(float deltaTime)
{
	if (!m_bodies[0])
		SwapRigidBodies();
	ASSERT_OR_DIE(m_bodies[0] != nullptr, "first body is empty when preparing for collision");

	// basis at contact
	ComputeContactCoord();

	// cache contact pos in world coord
	ComputeRelativePosWorldCoord();

	// closing velocity
	ComputeClosingVelocityContactCoord(deltaTime);
	
	// desired velocity
	ComputeDesiredDeltaVelocity(deltaTime);
}

void Collision::SwapRigidBodies()
{
	CollisionRigidBody* temp = m_bodies[0];
	m_bodies[0] = m_bodies[1];
	m_bodies[1] = temp;

	m_normal *= -1;
}

void Collision::ComputeContactCoord()
{
	Vector3 contact_tangent[2];

	if (abs(m_normal.x) > abs(m_normal.y))
	{
		const float s = 1.0f / sqrtf(m_normal.z * m_normal.z + m_normal.x * m_normal.x);

		contact_tangent[0].x = m_normal.z * s;
		contact_tangent[0].y = 0;
		contact_tangent[0].z = -m_normal.x * s;

		contact_tangent[1].x = m_normal.y * contact_tangent[0].x;
		contact_tangent[1].y = m_normal.z * contact_tangent[0].x - m_normal.x * contact_tangent[0].z;
		contact_tangent[1].z = -m_normal.y * contact_tangent[0].x;
	}
	else
	{
		const float s = 1.0f / sqrtf(m_normal.z * m_normal.z + m_normal.y * m_normal.y);

		contact_tangent[0].x = 0;
		contact_tangent[0].y = -m_normal.z*s;
		contact_tangent[0].z = m_normal.y*s;

		contact_tangent[1].x = m_normal.y * contact_tangent[0].z - m_normal.z * contact_tangent[0].y;
		contact_tangent[1].y = -m_normal.x * contact_tangent[0].z;
		contact_tangent[1].z = m_normal.x * contact_tangent[0].y;
	}

	m_to_world.SetBasis(m_normal, contact_tangent[0], contact_tangent[1]);
}

void Collision::ComputeRelativePosWorldCoord()
{
	// relative pos between contact and bodies
	m_relative_pos[0] = m_pos - m_bodies[0]->GetCenter();
	if (m_bodies[1] != nullptr) 
		m_relative_pos[1] = m_pos - m_bodies[1]->GetCenter();
}

void Collision::ComputeClosingVelocityContactCoord(float deltaTime)
{
	m_closing_vel = ComputeLocalVelocity(0, deltaTime);
	if (m_bodies[1])
		m_closing_vel -= ComputeLocalVelocity(1, deltaTime);
}

Vector3 Collision::ComputeLocalVelocity(uint idx, float deltaTime)
{
	CollisionRigidBody* rb = m_bodies[idx];

	Vector3 vel = rb->GetAngularVelocity().Cross(m_relative_pos[idx]);
	vel += rb->GetLinearVelocity();

	Vector3 contact_vel = m_to_world.MultiplyTranspose(vel);

	// force without reaction
	Vector3 acc_induced_vel = rb->GetLastFrameLinearAcc() * deltaTime;

	// velocity in contact coord
	acc_induced_vel = m_to_world.MultiplyTranspose(acc_induced_vel);

	acc_induced_vel.x = 0.f;

	contact_vel += acc_induced_vel;

	return contact_vel;
}

void Collision::ComputeDesiredDeltaVelocity(float deltaTime)
{
	static const float vel_limit = .25f;

	float vel_from_acc = 0.f;

	if (m_bodies[0]->IsAwake())
		vel_from_acc += DotProduct(m_bodies[0]->GetLastFrameLinearAcc() * deltaTime, m_normal);

	if (m_bodies[1] && m_bodies[1]->IsAwake())
		vel_from_acc -= DotProduct(m_bodies[1]->GetLastFrameLinearAcc() * deltaTime, m_normal);

	float real_restitution = m_mat.m_restitution;
	if (abs(m_closing_vel.x) < vel_limit)
		real_restitution = 0.f;

	m_desired_vel = -m_closing_vel.x - real_restitution * (m_closing_vel.x - vel_from_acc);
}

void Collision::CheckAwake()
{
	if (!m_bodies[1])
		return;

	bool body0awake = m_bodies[0]->IsAwake();
	bool body1awake = m_bodies[1]->IsAwake();

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

	for (unsigned i = 0; i < 2; i++) 
	{
		if (m_bodies[i])
		{
			Matrix33 inverseInertiaTensor;
			m_bodies[i]->GetIITWorld(&inverseInertiaTensor);

			Vector3 angularInertiaWorld = m_relative_pos[i].Cross(m_normal);
			angularInertiaWorld = inverseInertiaTensor * angularInertiaWorld;
			angularInertiaWorld = angularInertiaWorld.Cross(m_relative_pos[i]);
			angularInertia[i] = DotProduct(angularInertiaWorld, m_normal);

			linearInertia[i] = m_bodies[i]->GetInvMass();

			totalInertia += linearInertia[i] + angularInertia[i];
		}
	}

	for (unsigned i = 0; i < 2; i++)
	{
		if (m_bodies[i])
		{
			float sign = (i == 0) ? 1.f:-1.f;
			angularMove[i] = sign * penetration * (angularInertia[i] / totalInertia);
			linearMove[i] = sign * penetration * (linearInertia[i] / totalInertia);

			Vector3 projection = m_relative_pos[i];
			projection += (m_normal * (DotProduct(-m_relative_pos[i], m_normal)));

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

			if (angularMove[i] == 0)
			{
				angular[i].ToDefault();
			}
			else
			{
				Vector3 targetAngularDirection = m_relative_pos[i].Cross(m_normal);

				Matrix33 iit;
				m_bodies[i]->GetIITWorld(&iit);

				angular[i] = iit * targetAngularDirection * (angularMove[i] / angularInertia[i]);
			}

			linear[i] = m_normal * linearMove[i];

			Vector3 pos = m_bodies[i]->GetCenter();
			pos += (m_normal * linearMove[i]);
			m_bodies[i]->SetCenter(pos);

			Quaternion q = m_bodies[i]->GetOrientation();
			q.AddScaledVector(angular[i], 1.f);		// error: angular move instead of 1?
			m_bodies[i]->SetOrientation(q);

			if (!m_bodies[i]->IsAwake()) 
				m_bodies[i]->CacheData();
		}
	}
}

void Collision::ApplyVelocityChange(Vector3 linear[2], Vector3 angular[2])
{
	Matrix33 inverseInertiaTensor[2];
	m_bodies[0]->GetIITWorld(&inverseInertiaTensor[0]);
	if (m_bodies[1])
		m_bodies[1]->GetIITWorld(&inverseInertiaTensor[1]);

	Vector3 impulseContact;

	if (m_mat.m_friction == 0.f)
		impulseContact = ComputeFrictionlessImpulse(inverseInertiaTensor);
	else
		impulseContact = ComputeFrictionalImpulse(inverseInertiaTensor);

	Vector3 impulse = m_to_world * impulseContact;

	Vector3 impulsiveTorque = m_relative_pos[0].Cross(impulse);
	angular[0] = inverseInertiaTensor[0] * impulsiveTorque;
	linear[0].ToDefault();
	linear[0] += (impulse * m_bodies[0]->GetInvMass());

	m_bodies[0]->AddLinearVelocity(linear[0]);
	m_bodies[0]->AddAngularVelocity(angular[0]);

	if (m_bodies[1])
	{
		impulsiveTorque = impulse.Cross(m_relative_pos[1]);
		angular[1] = inverseInertiaTensor[1] * impulsiveTorque;
		linear[1].ToDefault();
		linear[1] += (impulse * -m_bodies[1]->GetInvMass());

		m_bodies[1]->AddLinearVelocity(linear[1]);
		m_bodies[1]->AddAngularVelocity(angular[1]);
	}
}

Vector3 Collision::ComputeFrictionlessImpulse(Matrix33* iit)
{
	Vector3 impulseContact;

	Vector3 deltaVelWorld = m_relative_pos[0].Cross(m_normal);
	deltaVelWorld = iit[0] * deltaVelWorld;
	deltaVelWorld = deltaVelWorld.Cross(m_relative_pos[0]);

	float deltaVelocity = DotProduct(deltaVelWorld, m_normal);

	deltaVelocity += m_bodies[0]->GetInvMass();

	if (m_bodies[1])
	{
		deltaVelWorld = m_relative_pos[1].Cross(m_normal);
		deltaVelWorld = iit[1] * deltaVelWorld;
		deltaVelWorld = deltaVelWorld.Cross(m_relative_pos[1]);

		deltaVelocity += DotProduct(deltaVelWorld, m_normal);

		deltaVelocity += m_bodies[1]->GetInvMass();
	}

	impulseContact.x = m_desired_vel / deltaVelocity;
	impulseContact.y = 0;
	impulseContact.z = 0;
	return impulseContact;
}

Vector3 Collision::ComputeFrictionalImpulse(Matrix33* iit)
{
	Vector3 impulseContact;
	float inverseMass = m_bodies[0]->GetInvMass();

	Matrix33 impulseToTorque;
	impulseToTorque.SetSkewSymmetric(m_relative_pos[0]);

	Matrix33 deltaVelWorld = impulseToTorque;
	deltaVelWorld *= iit[0];
	deltaVelWorld *= impulseToTorque;
	deltaVelWorld *= -1;

	if (m_bodies[1])
	{
		impulseToTorque.SetSkewSymmetric(m_relative_pos[1]);

		Matrix33 deltaVelWorld2 = impulseToTorque;
		deltaVelWorld2 *= iit[1];
		deltaVelWorld2 *= impulseToTorque;
		deltaVelWorld2 *= -1;

		deltaVelWorld += deltaVelWorld2;

		inverseMass += m_bodies[1]->GetInvMass();
	}

	Matrix33 deltaVelocity = m_to_world.Transpose();
	deltaVelocity *= deltaVelWorld;
	deltaVelocity *= m_to_world;

	deltaVelocity.Ix += inverseMass;
	deltaVelocity.Jy += inverseMass;
	deltaVelocity.Kz += inverseMass;

	Matrix33 impulseMatrix = deltaVelocity.Invert();

	Vector3 velKill(m_desired_vel, -m_closing_vel.y, -m_closing_vel.z);

	impulseContact = impulseMatrix * velKill;

	float planarImpulse = sqrtf(impulseContact.y * impulseContact.y + impulseContact.z * impulseContact.z );

	if (planarImpulse > impulseContact.x * m_mat.m_friction)
	{
		impulseContact.y /= planarImpulse;
		impulseContact.z /= planarImpulse;

		impulseContact.x = deltaVelocity.Ix + deltaVelocity.Jx * m_mat.m_friction * impulseContact.y + deltaVelocity.Kx * m_mat.m_friction * impulseContact.z;

		impulseContact.x = m_desired_vel / impulseContact.x;
		impulseContact.y *= m_mat.m_friction * impulseContact.x;
		impulseContact.z *= m_mat.m_friction * impulseContact.x;
	}

	return impulseContact;
}
