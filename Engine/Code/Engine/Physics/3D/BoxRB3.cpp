#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"

BoxRB3::BoxRB3(float mass, const OBB3& primitive, const Vector3& euler, eMoveStatus status)
{
	m_primitive = primitive;
	m_moveStatus = status;
	m_bodyID = BODY_RIGID;

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_center;

	Vector3 scale = m_primitive.GetHalfExt() * 2.f;
	m_entityTransform = Transform(m_center, euler, scale);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = mass;
		m_massData.m_invMass = 1.f / mass;

		// initialize sphere inertia tensor as needed
		float factor_i = (1.f / 12.f) * GetMass3() * 
			(m_primitive.GetFullExtY() * m_primitive.GetFullExtY()
				+ m_primitive.GetFullExtZ() * m_primitive.GetFullExtZ());
		float factor_j = (1.f / 12.f) * GetMass3() * 
			(m_primitive.GetFullExtX() * m_primitive.GetFullExtX()
				+ m_primitive.GetFullExtZ() * m_primitive.GetFullExtZ());
		float factor_k = (1.f / 12.f) * GetMass3() * 
			(m_primitive.GetFullExtX() * m_primitive.GetFullExtX()
				+ m_primitive.GetFullExtY() * m_primitive.GetFullExtY());
		Vector3 tensor_i = Vector3(factor_i, 0.f, 0.f);
		Vector3 tensor_j = Vector3(0.f, factor_j, 0.f);
		Vector3 tensor_k = Vector3(0.f, 0.f, factor_k);
		Matrix33 tensor = Matrix33(tensor_i, tensor_j, tensor_k);

		// set tensor
		m_massData.m_tensor = tensor;
		m_massData.m_invTensor = tensor.Invert();
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

	float diagonal = m_primitive.GetDiagonalRadius();
	m_boundSphere = BoundingSphere(m_center, diagonal);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(m_center, euler, Vector3(diagonal));

	TODO("AABB3 BV later if necessary");

	m_linearDamp = 1.f;
	m_angularDamp = 1.f;
}

void BoxRB3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}

void BoxRB3::UpdateEntityPrimitive()
{
	m_primitive.m_center = m_center;

	TODO("Update rot for primitive, otherwise visual will look off");
}

void BoxRB3::UpdateInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_P)
		&& !DevConsoleIsOpen())
		m_frozen = !m_frozen;
}

void BoxRB3::UpdateEntitiesTransforms()
{
	m_entityTransform.SetLocalPosition(m_center);

	// rot
	Matrix44 transMat;
	CacheTransform(transMat, m_center, m_orientation);
	Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(transMat);
	m_entityTransform.SetLocalRotation(euler);

	// assume scale is unchanged

	m_boundSphere.m_transform.SetLocalPosition(m_center);
}

void BoxRB3::Integrate(float deltaTime)
{
	CacheData();

	if (m_frozen)
	{
		// acc
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