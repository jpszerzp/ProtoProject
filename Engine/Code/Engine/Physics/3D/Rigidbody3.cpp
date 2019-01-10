#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Math/MathUtils.hpp"

Rigidbody3::Rigidbody3()
{
	InitializeRigid();
}

Rigidbody3::~Rigidbody3()
{

}

void Rigidbody3::InitializeRigid()
{
	// basic setup for a general rigid body
	SetQuaternionIdentity();
	m_angularVelocity = Vector3::ZERO;
	m_cachedTransform = Matrix44::IDENTITY;
	m_torqueAcc = Vector3::ZERO;
	m_angularDamp = 0.f;
}


void Rigidbody3::SetQuaternionIdentity()
{
	SetQuaternion(Quaternion::IDENTITY);
}

void Rigidbody3::SetQuaternion(const Quaternion& orientation)
{
	m_orientation = orientation;
	m_orientation.Normalize();
}

//void Rigidbody3::SetQuaternion(const float r, const float x, const float y, const float z)
//{
//	SetQuaternion(Quaternion(r, x, y, z));
//	m_orientation.Normalize();
//}

void Rigidbody3::SetAwake(bool awake)
{
	if (awake)
	{
		m_awake = true;
		m_motion = m_sleepThreshold * 2.f;		// so that this rigidbody will not fall asleep too fast
	}
	else
	{
		m_awake = false;
		m_linearVelocity.ToDefault();
		m_angularVelocity.ToDefault();
		// keep acceleration - this is optional and is more like a design choice
	}
}

void Rigidbody3::SetCanSleep(bool value)
{
	m_canSleep = value;
	
	// rigid body cannot sleep, yet is not awake either: wake it up
	if (!m_canSleep && !m_awake)
		SetAwake(true);
}

void Rigidbody3::CacheData()
{
	m_orientation.Normalize();
	CacheTransform(m_cachedTransform, m_center, m_orientation);
	CacheInverseInertiaTensorWorld(m_inverseInertiaTensorWorld, m_massData.m_invTensor, m_cachedTransform);
}

void Rigidbody3::CacheTransform(Matrix44& transform, const Vector3& position, const Quaternion& orientation)
{
	transform.Ix = 1.f - 2.f * orientation.m_imaginary.y * orientation.m_imaginary.y -
		2.f * orientation.m_imaginary.z * orientation.m_imaginary.z;
	transform.Jx = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.y -
		2.f * orientation.m_real * orientation.m_imaginary.z;
	transform.Kx = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.z +
		2.f * orientation.m_real * orientation.m_imaginary.y;
	transform.Tx = position.x;

	transform.Iy = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.y +
		2.f * orientation.m_real * orientation.m_imaginary.z;
	transform.Jy = 1.f - 2.f * orientation.m_imaginary.x * orientation.m_imaginary.x -
		2.f * orientation.m_imaginary.z * orientation.m_imaginary.z;
	transform.Ky = 2.f * orientation.m_imaginary.y * orientation.m_imaginary.z -
		2.f * orientation.m_real * orientation.m_imaginary.x;
	transform.Ty = position.y;

	transform.Iz = 2.f * orientation.m_imaginary.x * orientation.m_imaginary.z -
		2.f * orientation.m_real * orientation.m_imaginary.y;
	transform.Jz = 2.f * orientation.m_imaginary.y * orientation.m_imaginary.z +
		2.f * orientation.m_real * orientation.m_imaginary.x;
	transform.Kz = 1.f - 2.f * orientation.m_imaginary.x * orientation.m_imaginary.x -
		2.f * orientation.m_imaginary.y * orientation.m_imaginary.y;
	transform.Tz = position.z;
}

void Rigidbody3::CacheInverseInertiaTensorWorld(Matrix33& inv_inertia_tensor_world, const Matrix33& inv_inertia_tensor_body, const Matrix44& rot_mat)
{
	float t4 = rot_mat.Ix * inv_inertia_tensor_body.Ix +
		rot_mat.Jx * inv_inertia_tensor_body.Iy +
		rot_mat.Kx * inv_inertia_tensor_body.Iz;
	float t9 = rot_mat.Ix * inv_inertia_tensor_body.Jx +
		rot_mat.Jx * inv_inertia_tensor_body.Jy +
		rot_mat.Kx * inv_inertia_tensor_body.Jz;
	float t14 = rot_mat.Ix * inv_inertia_tensor_body.Kx +
		rot_mat.Jx * inv_inertia_tensor_body.Ky +
		rot_mat.Kx * inv_inertia_tensor_body.Kz;
	float t28 = rot_mat.Iy * inv_inertia_tensor_body.Ix +
		rot_mat.Jy * inv_inertia_tensor_body.Iy +
		rot_mat.Ky * inv_inertia_tensor_body.Iz;
	float t33 = rot_mat.Iy * inv_inertia_tensor_body.Jx+
		rot_mat.Jy * inv_inertia_tensor_body.Jy +
		rot_mat.Ky * inv_inertia_tensor_body.Jz;
	float t38 = rot_mat.Iy * inv_inertia_tensor_body.Kx +
		rot_mat.Jy * inv_inertia_tensor_body.Ky +
		rot_mat.Ky * inv_inertia_tensor_body.Kz;
	float t52 = rot_mat.Iz * inv_inertia_tensor_body.Ix +
		rot_mat.Jz * inv_inertia_tensor_body.Iy +
		rot_mat.Kz * inv_inertia_tensor_body.Iz;
	float t57 = rot_mat.Iz * inv_inertia_tensor_body.Jx +
		rot_mat.Jz * inv_inertia_tensor_body.Jy +
		rot_mat.Kz * inv_inertia_tensor_body.Jz;
	float t62 = rot_mat.Iz * inv_inertia_tensor_body.Kx +
		rot_mat.Jz * inv_inertia_tensor_body.Ky +
		rot_mat.Kz * inv_inertia_tensor_body.Kz;

	inv_inertia_tensor_world.Ix = t4 * rot_mat.Ix + t9 * rot_mat.Jx + t14 * rot_mat.Kx;
	inv_inertia_tensor_world.Jx = t4 * rot_mat.Iy + t9 * rot_mat.Jy + t14 * rot_mat.Ky;
	inv_inertia_tensor_world.Kx = t4 * rot_mat.Iz + t9 * rot_mat.Jz + t14 * rot_mat.Kz;
	inv_inertia_tensor_world.Iy = t28 * rot_mat.Ix + t33 * rot_mat.Jx + t38 * rot_mat.Kx;
	inv_inertia_tensor_world.Jy = t28 * rot_mat.Iy + t33 * rot_mat.Jy + t38 * rot_mat.Ky;
	inv_inertia_tensor_world.Ky = t28 * rot_mat.Iz + t33 * rot_mat.Jz + t38 * rot_mat.Kz;
	inv_inertia_tensor_world.Iz = t52 * rot_mat.Ix + t57 * rot_mat.Jx + t62 * rot_mat.Kx;
	inv_inertia_tensor_world.Jz = t52 * rot_mat.Iy + t57 * rot_mat.Jy + t62 * rot_mat.Ky;
	inv_inertia_tensor_world.Kz = t52 * rot_mat.Iz + t57 * rot_mat.Jz + t62 * rot_mat.Kz;
}


void Rigidbody3::AddForce(Vector3 force)
{
	m_netforce += force;

	m_awake = true;
}

void Rigidbody3::AddTorque(Vector3 torque)
{
	m_torqueAcc += torque;

	m_awake = true;
}

void Rigidbody3::AddForcePointObjectCoord(const Vector3& force, const Vector3& point)
{
	Vector3 pt_world = GetPointInWorld(point);
	AddForcePointWorldCoord(force, pt_world);
}


void Rigidbody3::AddForcePointWorldCoord(const Vector3& force, const Vector3 point_world)
{
	// Convert to coordinates relative to center of mass.
	Vector3 pt = point_world;
	pt -= m_center;

	m_netforce += force;
	m_torqueAcc += pt.Cross(force);			// impulse causes torque

	m_awake = true;
}

Vector3 Rigidbody3::GetPointInWorld(const Vector3& pt_local)
{
	Vector4 pt_local4 = Vector4(pt_local, 1.f);
	Vector4 pt_world4 = m_cachedTransform * pt_local4;
	return pt_world4.ToVector3();
}

void Rigidbody3::ClearAccs()
{
	m_netforce = Vector3::ZERO;			// linear force acc
	m_torqueAcc = Vector3::ZERO;		// angular torque acc
}

void Rigidbody3::Render(Renderer* renderer)
{
	// only draw bounding sphere on rigid for now
	if (m_drawBoundSphere)
		m_boundSphere.DrawBound(renderer);
}

void Rigidbody3::Integrate(float deltaTime)
{
	CacheData();

	if (!m_frozen)
	{
		// acc
		m_lastFrameLinearAcc = m_linearAcceleration;
		m_linearAcceleration = m_netforce * m_massData.m_invMass;
		Vector3 angularAcc = m_inverseInertiaTensorWorld * m_torqueAcc;

		// vel
		m_linearVelocity += m_linearAcceleration * deltaTime;
		m_angularVelocity += angularAcc * deltaTime;

		// damp on vel
		m_linearVelocity *= powf(m_linearDamp, deltaTime);	// damp 1 means no damp	
		m_angularVelocity *= powf(m_angularDamp, deltaTime);

		// pos
		m_center += m_linearVelocity * deltaTime;
		m_orientation.AddScaledVector(m_angularVelocity, deltaTime);
	}

	ClearAccs();
}

void Rigidbody3::Simulate(float deltaTime, Vector3& simulate_pos, Quaternion& simulate_orient)
{
	// simulated acc
	Vector3 linear_acc = m_netforce * m_massData.m_invMass;
	Vector3 ang_acc = m_inverseInertiaTensorWorld * m_torqueAcc;

	// simulated vel
	Vector3 linear_vel = m_linearVelocity + linear_acc * deltaTime;
	Vector3 ang_vel = m_angularVelocity + ang_acc * deltaTime;

	// clamp simulation
	linear_vel *= powf(m_linearDamp, deltaTime);
	ang_vel *= powf(m_angularDamp, deltaTime);

	// first-order Newton
	if (linear_acc.GetLength() < ACC_LIMIT && ang_acc.GetLength() < ACC_LIMIT)
	{
		simulate_pos = m_center + linear_vel * deltaTime;							// pos
		simulate_orient = m_orientation;
		simulate_orient.AddScaledVector(ang_vel, deltaTime);
	}
	// second-order Newton
	// used when either linear or angular acc goes too large - in this case use second-order is safer yet costly
	else
	{
		simulate_pos = m_center + (linear_vel * deltaTime + linear_acc * deltaTime * deltaTime * .5f);
		simulate_orient = m_orientation;
		simulate_orient.AddScaledVector(ang_vel, deltaTime);
		simulate_orient.AddScaledVector(ang_acc, deltaTime * deltaTime * .5f);
	}
}

void Rigidbody3::UpdateDynamicsCore(float usedTime)
{
	if (!m_frozen)
	{
		// acc
		m_lastFrameLinearAcc = m_linearAcceleration;
		m_linearAcceleration = m_netforce * m_massData.m_invMass;
		Vector3 angularAcc = m_inverseInertiaTensorWorld * m_torqueAcc;

		// vel
		m_linearVelocity += m_linearAcceleration * usedTime;
		m_angularVelocity += angularAcc * usedTime;

		// damp on vel
		m_linearVelocity *= powf(m_linearDamp, usedTime);	// damp: 1 means no damp	
		m_angularVelocity *= powf(m_angularDamp, usedTime);

		// first-order Newton
		if (m_linearAcceleration.GetLength() < ACC_LIMIT && angularAcc.GetLength() < ACC_LIMIT)
		{
			m_center += m_linearVelocity * usedTime;							// pos
			m_orientation.AddScaledVector(m_angularVelocity, usedTime);		// rot
		}
		// second-order Newton
		// used when either linear or angular acc goes too large - in this case use second-order is safer yet costly
		else
		{
			m_center += (m_linearVelocity * usedTime + m_linearAcceleration * usedTime * usedTime * .5f);
			m_orientation.AddScaledVector(m_angularVelocity, usedTime);
			m_orientation.AddScaledVector(angularAcc, usedTime * usedTime * .5f);
		}

		CacheData();
	}

	ClearAccs();
}

void Rigidbody3::UpdateSleepSystem(float usedTime)
{
	// updating sleep system
	if (m_canSleep)
	{
		float currentMotion = DotProduct(m_linearVelocity, m_linearVelocity) + DotProduct(m_angularVelocity, m_angularVelocity);

		float bias = powf(.5f, usedTime);
		m_motion = bias * m_motion + (1.f - bias) * currentMotion;

		if (m_motion < m_sleepThreshold) 
			SetAwake(false);
		else if (m_motion > 10.f * m_sleepThreshold) 
			m_motion = 10.f * m_sleepThreshold;		// clamp up to 10 times of threshold
	}
}
