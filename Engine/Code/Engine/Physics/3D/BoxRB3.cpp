#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

BoxRB3::BoxRB3(float mass, const OBB3& primitive, const Vector3& euler, eMoveStatus status)
{
	m_primitive = primitive;
	m_moveStatus = status;
	m_bodyID = BODY_RIGID;

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_center;

	Vector3 scale = primitive.GetHalfExt() * 2.f;
	m_entityTransform = Transform(m_center, euler, scale);

	// euler to quaternion
	Matrix44 rot_mat = Matrix44::MakeRotationDegrees3D(euler);
	Matrix33 rot_only = rot_mat.ExtractMat3();
	m_orientation = Quaternion::FromMatrix(rot_only);
	
	// ignore bounding box for this, use bounding sphere

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = mass;
		m_massData.m_invMass = 1.f / mass;

		// initialize sphere inertia tensor as needed
		float factor_i = (1.f / 12.f) * GetMass3() * 
			(primitive.GetFullExtY() * primitive.GetFullExtY()
				+ primitive.GetFullExtZ() * primitive.GetFullExtZ());
		float factor_j = (1.f / 12.f) * GetMass3() * 
			(primitive.GetFullExtX() * primitive.GetFullExtX()
				+ primitive.GetFullExtZ() * primitive.GetFullExtZ());
		float factor_k = (1.f / 12.f) * GetMass3() * 
			(primitive.GetFullExtX() * primitive.GetFullExtX()
				+ primitive.GetFullExtY() * primitive.GetFullExtY());
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

	float diagonal = primitive.GetDiagonalHalf();
	m_boundSphere = BoundingSphere(m_center, diagonal);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(m_center, euler, Vector3(diagonal));

	m_linearDamp = 1.f;
	m_angularDamp = 1.f;
}

void BoxRB3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}

void BoxRB3::UpdatePrimitives()
{
	m_boundSphere.SetCenter(m_center);

	// update the box primitive itself
	// what needs to update in obb3: center, forward, up, right
	m_primitive.SetCenter(m_center);
	m_primitive.SetForward(m_entityTransform.GetWorldForward());
	m_primitive.SetUp(m_entityTransform.GetWorldUp());
	m_primitive.SetRight(m_entityTransform.GetWorldRight());

	// update face vert and edge
	m_primitive.UpdateFace();
	m_primitive.UpdateVertAndEdge();
}

void BoxRB3::UpdateInput(float)
{
	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_P) && !DevConsoleIsOpen() && !m_constrained)
		m_frozen = !m_frozen;

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3) && m_constrained)
		m_frozen = !m_frozen;
}

void BoxRB3::UpdateTransforms()
{
	m_entityTransform.SetLocalPosition(m_center);

	// rot
	Matrix44 rot_mat_44 = Quaternion::GetMatrixRotation(m_orientation);
	Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(rot_mat_44);
	m_entityTransform.SetLocalRotation(euler);

	// assume scale is unchanged

	m_boundSphere.m_transform.SetLocalPosition(m_center);
}

void BoxRB3::Integrate(float deltaTime)
{
	//float usedTime = deltaTime * m_slowed;

	//if (!m_awake) return;

	//UpdateInput(usedTime);

	//if (!m_frozen)
	//{
	//	// acc
	//	m_lastFrameLinearAcc = m_linearAcceleration;
	//	m_linearAcceleration = m_netforce * m_massData.m_invMass;
	//	Vector3 angularAcc = m_inverseInertiaTensorWorld * m_torqueAcc;

	//	// vel
	//	m_linearVelocity += m_linearAcceleration * usedTime;
	//	m_angularVelocity += angularAcc * usedTime;

	//	// damp on vel
	//	m_linearVelocity *= powf(m_linearDamp, usedTime);	// damp 1 means no damp	
	//	m_angularVelocity *= powf(m_angularDamp, usedTime);

	//	// first-order Newton
	//	if (m_linearAcceleration.GetLength() < ACC_LIMIT && angularAcc.GetLength() < ACC_LIMIT)
	//	{
	//		m_center += m_linearVelocity * usedTime;							// pos
	//		m_orientation.AddScaledVector(m_angularVelocity, usedTime);		// rot
	//	}
	//	// second-order Newton
	//	// used when either linear or angular acc goes too large - in this case use second-order is safer yet costly
	//	else
	//	{
	//		m_center += (m_linearVelocity * usedTime + m_linearAcceleration * usedTime * usedTime * .5f);
	//		m_orientation.AddScaledVector(m_angularVelocity, usedTime);
	//		m_orientation.AddScaledVector(angularAcc, usedTime * usedTime * .5f);
	//	}

	//	CacheData();
	//}

	//ClearAccs();

	//// updating sleep system
	//if (m_canSleep)
	//{
	//	float currentMotion = DotProduct(m_linearVelocity, m_linearVelocity) + DotProduct(m_angularVelocity, m_angularVelocity);

	//	float bias = powf(.5f, usedTime);
	//	m_motion = bias * m_motion + (1.f - bias) * currentMotion;

	//	float clampMax = 10.f * m_sleepThreshold;
	//	if (m_motion < m_sleepThreshold) 
	//		SetAwake(false);
	//	else if (m_motion > clampMax) 
	//		m_motion = clampMax;		// clamp up to 10 times of threshold
	//}

	float usedTime = deltaTime;

	usedTime *= m_slowed;

	if (!m_awake) return;

	UpdateInput(usedTime);

	UpdateDynamicsCore(usedTime);

	UpdateSleepSystem(usedTime);
}
