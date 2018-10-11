#include "Engine/Physics/3D/QuadRB3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

QuadRB3::QuadRB3(float mass, const Plane& primitive, 
	const Vector3& center, const Vector3& euler, const Vector3& scale,
	eMoveStatus moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_RIGID;

	m_linearVelocity = Vector3::ZERO;

	m_center = center;

	// entity transform
	m_entityTransform = Transform(m_center, euler, scale);

	// BV transform - sphere
	Vector3 boundSpherePos = m_center;
	Vector3 boundSphereRot = Vector3::ZERO;
	float boundSphereRadius = sqrtf(scale.x * scale.x + scale.y * scale.y) / 2.f;
	Vector3 boundSphereScale = Vector3(boundSphereRadius, boundSphereRadius, boundSphereRadius);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;

		TODO("Plane tensor");
	}
	else
	{
		// static rigid sphere mass
		m_massData.m_mass = mass;
		m_massData.m_invMass = 0.f;

		// static tensor
		m_massData.m_tensor = Matrix33::ZERO;
		m_massData.m_invTensor = Matrix33::ZERO;
	}

	// BV sphere primitive
	m_boundSphere = BoundingSphere(m_center, boundSphereRadius);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(boundSpherePos, boundSphereRot, boundSphereScale);

	m_linearDamp = 1.f;
	m_angularDamp = 1.f;
}

void QuadRB3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}

void QuadRB3::UpdatePrimitives()
{
	m_boundSphere.SetCenter(m_center);
	m_boundBox.SetCenter(m_center);

	// No concept of "center" for plane, hence no need to update

	TODO("Update rot for primitive, otherwise visual will look off");
}

void QuadRB3::UpdateTransforms()
{
	m_entityTransform.SetLocalPosition(m_center);

	// rot
	Matrix44 transMat;
	CacheTransform(transMat, m_center, m_orientation);
	Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(transMat);
	m_entityTransform.SetLocalRotation(euler);

	TODO("Assume scale unchanged. Safe?");

	m_boundSphere.m_transform.SetLocalPosition(m_center);
}

void QuadRB3::Integrate(float deltaTime)
{
	CacheData();

	if (m_frozen)
	{
		// acc
		m_lastAcc = m_linearAcceleration;
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

