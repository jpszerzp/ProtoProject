#include "Engine/Physics/3D/RF/CollisionEntity.hpp"
#include "Engine/Math/MathUtils.hpp"

#define SLEEP_THRESHOLD 1.5f

void CollisionEntity::Integrate(float)
{

}

void CollisionEntity::ClearAcc()
{

}

void CollisionEntity::SetSleepable(bool sleepable)
{
	m_sleepable = sleepable;

	if (!m_sleepable && !m_awake)
		// make sure it is awake if it is not sleepable and not awake
		SetAwake(true);
}

float CollisionEntity::GetRealTimeMotion() const
{
	return DotProduct(m_lin_vel, m_lin_vel);
}

void CollisionEntity::AddLinearVelocity(const Vector3& v)
{
	m_lin_vel += v;
}

void CollisionEntity::AddForce(const Vector3& f)
{
	m_net_force += f;
}

void CollisionRigidBody::Integrate(float deltaTime)
{
	if (!m_awake)
		return;

	deltaTime *= m_slow;

	m_last_lin_acc = m_lin_acc;						// base acc
	m_last_lin_acc += (m_net_force * m_inv_mass);
	Vector3 ang_acc = m_inv_tensor_world * m_net_torque;

	m_lin_vel += (m_last_lin_acc * deltaTime);
	m_ang_vel += (ang_acc * deltaTime);

	m_lin_vel *= powf(m_lin_damp, deltaTime);
	m_ang_vel *= powf(m_ang_damp, deltaTime);

	m_center += (m_lin_vel * deltaTime);
	m_orientation.AddScaledVector(m_ang_vel, deltaTime);

	CacheData();

	ClearAcc();

	if (m_sleepable)
	{
		float currentMotion = GetRealTimeMotion();

		float bias = powf(0.5, deltaTime);
		m_motion = bias * m_motion + (1 - bias) * currentMotion;

		if (m_motion < SLEEP_THRESHOLD) 
			SetAwake(false);
		else if (m_motion > 10 * SLEEP_THRESHOLD) 
			m_motion = 10 * SLEEP_THRESHOLD;
	}
}

void CollisionRigidBody::IntegrateEulerParticle(float dt)
{
	// make sure it is particle
	if (m_particle)
	{
		if (!m_awake)
			return;

		dt *= m_slow;

		// acc
		m_last_lin_acc = m_lin_acc;						// base acc
		m_last_lin_acc += (m_net_force * m_inv_mass);

		// vel
		m_lin_vel += (m_last_lin_acc * dt);
		m_lin_vel *= powf(m_lin_damp, dt);

		// pos 
		m_center += (m_lin_vel * dt);

		// cache
		CacheData();

		ClearAcc();

		if (m_sleepable)
		{
			float currentMotion = GetRealTimeMotion();

			float lerp = powf(0.5, dt);
			m_motion = lerp * m_motion + (1 - lerp) * currentMotion;	// lerp to real time motion

			if (m_motion < SLEEP_THRESHOLD) 
				SetAwake(false);
			else if (m_motion > 10 * SLEEP_THRESHOLD) 
				m_motion = 10 * SLEEP_THRESHOLD;
		}
	}
	else
		ASSERT_OR_DIE(false, "non-particles should not care if the integration is euler-based or verlet based");
}

void CollisionRigidBody::IntegrateVerletParticle(float dt)
{
	if (m_particle)
	{
		if (!m_awake)
			return;

		dt *= m_slow;

		// basic verlet or vel verlet
		if (m_verlet_p == VERLET_BASIC_P)
		{
			Vector3 p = m_center;

			// acc
			m_last_lin_acc = m_lin_acc;
			m_last_lin_acc += (m_net_force * m_inv_mass);

			// no vel

			// pos
			m_center += (m_center - m_last_center) + m_lin_acc * dt * dt;
			m_last_center = p;
		}
		else if (m_verlet_p == VERLET_VEL_P)
		{
			// acc
			m_last_lin_acc = m_lin_acc;		
			m_last_lin_acc += (m_net_force * m_inv_mass);

			// vel
			m_half_step_vel = m_lin_vel + m_lin_acc * .5f * dt;
			m_lin_vel = m_half_step_vel + m_lin_acc * .5f * dt;

			m_center += m_half_step_vel * dt;
		}

		// cache
		CacheData();

		ClearAcc();

		if (m_sleepable)
		{
			float currentMotion = GetRealTimeMotion();

			float lerp = powf(0.5, dt);
			m_motion = lerp * m_motion + (1 - lerp) * currentMotion;	// lerp to real time motion

			if (m_motion < SLEEP_THRESHOLD) 
				SetAwake(false);
			else if (m_motion > 10 * SLEEP_THRESHOLD) 
				m_motion = 10 * SLEEP_THRESHOLD;
		}
	}
	else
		ASSERT_OR_DIE(false, "non-particles should not care if the integration is euler-based or verlet based");
}

void CollisionRigidBody::ClearAcc()
{
	m_net_force.ToDefault();
	m_net_torque.ToDefault();
}

CollisionRigidBody::CollisionRigidBody(const float& mass, const Vector3& center, const Vector3& euler)
{
	// orientation from euler
	m_orientation = Quaternion::FromEuler(euler);

	SetCenter(center);

	m_mass = mass;
	m_inv_mass = 1.f / mass;
}

void CollisionRigidBody::CacheData()
{
	m_orientation.Normalize();

	CacheTransformMat(m_transform_mat, m_center, m_orientation);

	CacheIITWorld(m_inv_tensor_world, m_inv_tensor, m_transform_mat);
}

void CollisionRigidBody::CacheTransformMat(Matrix44& transform, const Vector3& position, const Quaternion& orientation)
{
	transform.Ix = 1.f - 2.f * orientation.m_imaginary.y * orientation.m_imaginary.y - 2.f * orientation.m_imaginary.z * orientation.m_imaginary.z;
	transform.Jx = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.y - 2.f * orientation.m_real * orientation.m_imaginary.z;
	transform.Kx = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.z + 2.f * orientation.m_real * orientation.m_imaginary.y;
	transform.Tx = position.x;

	transform.Iy = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.y + 2.f * orientation.m_real * orientation.m_imaginary.z;
	transform.Jy = 1.f - 2.f * orientation.m_imaginary.x * orientation.m_imaginary.x - 2.f * orientation.m_imaginary.z * orientation.m_imaginary.z;
	transform.Ky = 2.f * orientation.m_imaginary.y * orientation.m_imaginary.z - 2.f * orientation.m_real * orientation.m_imaginary.x;
	transform.Ty = position.y;

	transform.Iz = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.z - 2.f * orientation.m_real * orientation.m_imaginary.y;
	transform.Jz = 2.f * orientation.m_imaginary.y * orientation.m_imaginary.z + 2.f * orientation.m_real * orientation.m_imaginary.x;
	transform.Kz = 1.f - 2.f * orientation.m_imaginary.x * orientation.m_imaginary.x - 2.f * orientation.m_imaginary.y * orientation.m_imaginary.y;
	transform.Tz = position.z;
}

void CollisionRigidBody::CacheIITWorld(Matrix33& iitw, const Matrix33& iit, const Matrix44& transfrom_mat)
{
	float t4 = transfrom_mat.Ix * iit.Ix + transfrom_mat.Jx * iit.Iy + transfrom_mat.Kx * iit.Iz;
	float t9 = transfrom_mat.Ix * iit.Jx + transfrom_mat.Jx * iit.Jy + transfrom_mat.Kx * iit.Jz;
	float t14 = transfrom_mat.Ix * iit.Kx + transfrom_mat.Jx * iit.Ky + transfrom_mat.Kx * iit.Kz;
	float t28 = transfrom_mat.Iy * iit.Ix + transfrom_mat.Jy * iit.Iy + transfrom_mat.Ky * iit.Iz;
	float t33 = transfrom_mat.Iy * iit.Jx + transfrom_mat.Jy * iit.Jy + transfrom_mat.Ky * iit.Jz;
	float t38 = transfrom_mat.Iy * iit.Kx + transfrom_mat.Jy * iit.Ky + transfrom_mat.Ky * iit.Kz;
	float t52 = transfrom_mat.Iz * iit.Ix + transfrom_mat.Jz * iit.Iy + transfrom_mat.Kz * iit.Iz;
	float t57 = transfrom_mat.Iz * iit.Jx + transfrom_mat.Jz * iit.Jy + transfrom_mat.Kz * iit.Jz;
	float t62 = transfrom_mat.Iz * iit.Kx + transfrom_mat.Jz * iit.Ky + transfrom_mat.Kz * iit.Kz;

	iitw.Ix = t4 * transfrom_mat.Ix + t9 * transfrom_mat.Jx + t14 * transfrom_mat.Kx;
	iitw.Jx = t4 * transfrom_mat.Iy + t9 * transfrom_mat.Jy + t14 * transfrom_mat.Ky;
	iitw.Kx = t4 * transfrom_mat.Iz + t9 * transfrom_mat.Jz + t14 * transfrom_mat.Kz;
	iitw.Iy = t28 * transfrom_mat.Ix + t33 * transfrom_mat.Jx + t38 * transfrom_mat.Kx;
	iitw.Jy = t28 * transfrom_mat.Iy + t33 * transfrom_mat.Jy + t38 * transfrom_mat.Ky;
	iitw.Ky = t28 * transfrom_mat.Iz + t33 * transfrom_mat.Jz + t38 * transfrom_mat.Kz;
	iitw.Iz = t52 * transfrom_mat.Ix + t57 * transfrom_mat.Jx + t62 * transfrom_mat.Kx;
	iitw.Jz = t52 * transfrom_mat.Iy + t57 * transfrom_mat.Jy + t62 * transfrom_mat.Ky;
	iitw.Kz = t52 * transfrom_mat.Iz + t57 * transfrom_mat.Jz + t62 * transfrom_mat.Kz;
}

void CollisionRigidBody::SetAwake(bool awake)
{
	if (awake) 
	{
		m_awake = true;

		// Add a bit of motion to avoid it falling asleep immediately.
		m_motion = SLEEP_THRESHOLD * 1.05f;
	} 
	else 
	{
		m_awake = false;
		m_lin_vel.ToDefault();
		m_ang_vel.ToDefault();
	}
}

void CollisionRigidBody::GetIITWorld(Matrix33* iitw) const
{
	*iitw = m_inv_tensor_world;
}

float CollisionRigidBody::GetRealTimeMotion() const
{
	return DotProduct(m_lin_vel, m_lin_vel) + DotProduct(m_ang_vel, m_ang_vel);
}

void CollisionRigidBody::AddAngularVelocity(const Vector3& v)
{
	m_ang_vel += v;
}

void CollisionRigidBody::AddForcePointWorldCoord(const Vector3& force, const Vector3 point_world)
{
	// Convert to coordinates relative to center of mass.
	Vector3 pt = point_world;
	pt -= m_center;

	m_net_force += force;
	m_net_torque += pt.Cross(force);			// impulse causes torque

	// wake it up
	m_awake = true;
}
