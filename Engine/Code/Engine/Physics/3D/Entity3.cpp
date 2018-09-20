#include "Engine/Physics/3D/Entity3.hpp"

void Entity3::SetBoundSpherePos(Vector3 pos)
{
	m_sphereBoundTransform.SetLocalPosition(pos);
}

void Entity3::SetBoundBoxPos(Vector3 pos)
{
	m_boxBoundTransform.SetLocalPosition(pos);
}

void Entity3::SetMass(float mass)
{
	m_massData.m_mass = mass;
	m_massData.m_invMass = 1.f / mass;
}

void Entity3::SetLinearVelocity(float vel_x, float vel_y, float vel_z)
{
	m_linearVelocity = Vector3(vel_x, vel_y, vel_z);
}

void Entity3::SetLinearAcceleration(float acc_x, float acc_y, float acc_z)
{
	m_linearAcceleration = Vector3(acc_x, acc_y, acc_z);
}

void Entity3::SetNetForce(float f_x, float f_y, float f_z)
{
	m_netforce = Vector3(f_x, f_y, f_z);
}

bool Entity3::HasInfiniteMass() const
{
	MassData3 data = GetMassData3();
	if (data.m_invMass == 0.f)
		return true;
	return false;
}

void Entity3::UpdateBoundPrimitives()
{
	m_boundSphere.SetCenter(m_center);
	m_boundBox.SetCenter(m_center);
}

void Entity3::UpdateEntitiesTransforms()
{
	// update transform
	m_entityTransform.SetLocalPosition(m_center);
	m_sphereBoundTransform.SetLocalPosition(m_center);
	m_boxBoundTransform.SetLocalPosition(m_center);
}

void Entity3::VerletIntegrate(float deltaTime)
{
	// basic position verlet
	if (m_verlet_scheme == BASIC_VERLET)
	{
		// limitation: object of verlet needs to have a natural start (no initial velocity to kick off with)
		Vector3 curr_pos = m_center;

		m_linearAcceleration = m_netforce * m_massData.m_invMass;

		m_center += (m_center - m_lastCenter) + m_linearAcceleration * deltaTime * deltaTime;
		m_lastCenter = curr_pos;
	}
	// velocity verlet
	else
	{
		m_linearAcceleration = m_netforce * m_massData.m_invMass;

		m_halfStepVelocity = m_linearVelocity + m_linearAcceleration * .5f * deltaTime;
		
		m_center += m_halfStepVelocity * deltaTime;

		// todo: if the force is dependent on x and t, then update m_linearAcceleration here.

		m_linearVelocity = m_halfStepVelocity + m_linearAcceleration * .5f * deltaTime;
	}
}

void Entity3::EulerIntegrate(float deltaTime)
{
	// pos
	PositionIntegrate(deltaTime);

	// acc
	m_linearAcceleration = m_netforce * m_massData.m_invMass;

	// vel
	VelocityIntegrate(deltaTime);
}

void Entity3::Integrate(float deltaTime)
{
	if (!m_frozen)
	{
		if (!m_verlet)
			EulerIntegrate(deltaTime);
		else
			VerletIntegrate(deltaTime);
	}

	// reset net force
	if (!m_forcePersistent)
		ClearForce();
}

void Entity3::PositionIntegrate(float deltaTime)
{
	m_center += m_linearVelocity * deltaTime;

	// or, we also consider minimal change on position contributed by acceleration
	//m_center += m_linearVelocity * deltaTime + m_linearAcceleration * deltaTime * deltaTime * 0.5f;
}

void Entity3::VelocityIntegrate(float deltaTime)
{
	m_linearVelocity += m_linearAcceleration * deltaTime;
	if (m_considerDamp)
		m_linearVelocity *= powf(m_linearDamp, deltaTime);		// damp
}
