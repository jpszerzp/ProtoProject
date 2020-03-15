//#include "Engine/Physics/3D/SphereRB3.hpp"
//#include "Engine/Input/InputSystem.hpp"
//#include "Engine/Core/Console/DevConsole.hpp"
//#include "Engine/Math/MathUtils.hpp"
//
//SphereRB3::SphereRB3()
//{
//	// zero tensor
//	// "zero" primitive
//}
//
//SphereRB3::SphereRB3(float mass, Sphere3 primitive, Vector3 euler, eMoveStatus moveStat) 
//{
//	m_primitive = primitive;
//	m_moveStatus = moveStat;
//	m_bodyID = BODY_RIGID;
//
//	m_linearVelocity = Vector3::ZERO;
//	m_center = primitive.m_center;
//
//	float radius = primitive.m_radius;
//	Vector3 scale = Vector3(radius);
//	m_entityTransform = Transform(m_center, euler, scale);
//
//	// euler to quaternion
//	Matrix44 rot_mat = Matrix44::MakeRotationDegrees3D(euler);
//	Matrix33 rot_only = rot_mat.ExtractMat3();
//	m_orientation = Quaternion::FromMatrix(rot_only);
//
//	float boundBoxDim = 2.f * radius;
//	Vector3 boundBoxScale = Vector3(boundBoxDim);
//	m_boxBoundTransform = Transform(m_center, euler, boundBoxScale);
//
//	if (m_moveStatus != MOVE_STATIC)
//	{
//		m_massData.m_mass = mass;
//		m_massData.m_invMass = 1.f / mass;
//
//		// initialize sphere inertia tensor as needed
//		float factor = 0.4f * mass * primitive.m_radius * primitive.m_radius;
//		Vector3 tensor_i = Vector3(factor, 0.f, 0.f);
//		Vector3 tensor_j = Vector3(0.f, factor, 0.f);
//		Vector3 tensor_k = Vector3(0.f, 0.f, factor);
//		Matrix33 tensor = Matrix33(tensor_i, tensor_j, tensor_k);
//
//		// set tensor
//		m_massData.m_tensor = tensor;
//		m_massData.m_invTensor = tensor.GetInverse();
//	}
//	else
//	{
//		// static rigid sphere mass
//		m_massData.m_mass = mass;
//		m_massData.m_invMass = 0.f;
//
//		// static tensor
//		m_massData.m_tensor = Matrix33::ZERO;
//		m_massData.m_invTensor = Matrix33::ZERO;
//	}
//
//	m_boundSphere = BoundingSphere(m_center, primitive.m_radius);
//	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
//	m_boundSphere.m_transform = m_entityTransform;
//
//	Vector3 boundBoxMin = m_center - boundBoxScale / 2.f;
//	Vector3 boundBoxMax = m_center + boundBoxScale / 2.f;
//	m_boundBox = AABB3(boundBoxMin, boundBoxMax);
//	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);
//
//	m_linearDamp = 1.f;
//	m_angularDamp = 1.f;		
//}
//
//SphereRB3::~SphereRB3()
//{
//
//}
//
//void SphereRB3::SetEntityForPrimitive()
//{
//	m_primitive.SetEntity(this);
//}
//
//void SphereRB3::UpdatePrimitives()
//{
//	m_boundSphere.SetCenter(m_center);
//	m_boundBox.SetCenter(m_center);
//
//	m_primitive.SetCenter(m_center);
//}
//
//void SphereRB3::UpdateTransforms()
//{
//	m_entityTransform.SetLocalPosition(m_center);
//
//	// rot
//	Matrix44 rot_mat_44 = Quaternion::GetMatrixRotation(m_orientation);
//	Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(rot_mat_44);
//	m_entityTransform.SetLocalRotation(euler);
//
//	// assume scale is unchanged
//
//	m_boundSphere.m_transform.SetLocalPosition(m_center);
//	m_boxBoundTransform.SetLocalPosition(m_center);
//}
//
//void SphereRB3::UpdateInput(float)
//{
//	InputSystem* input = InputSystem::GetInstance();
//
//	// if this is not a constrained sphere - not involved in any constraints, hit P will freeze/unfreeze it
//	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_P) && !m_constrained)
//		m_frozen = !m_frozen;
//
//	// if this is a constrained sphere - involved in some constraint, hit NUMPAD_3 will freeze/unfreeze it
//	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3) && m_constrained)
//		m_frozen = !m_frozen;
//}
//
//void SphereRB3::Integrate(float deltaTime)
//{
//	float usedTime = deltaTime;
//
//	usedTime *= m_slowed;
//
//	if (!m_awake) return;
//
//	UpdateInput(usedTime);
//
//	UpdateDynamicsCore(usedTime);
//
//	UpdateSleepSystem(usedTime);
//}
