#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Input/InputSystem.hpp"

SphereRB3::SphereRB3()
{
	// zero tensor
	// "zero" primitive
}

SphereRB3::SphereRB3(float mass, Sphere3 primitive, eMoveStatus moveStat) 
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_RIGID;

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_center;

	Vector3 rot = Vector3::ZERO;
	float radius = primitive.m_radius;
	Vector3 scale = Vector3(radius);
	m_entityTransform = Transform(m_center, rot, scale);

	float boundBoxDim = 2.f * radius;
	Vector3 boundBoxScale = Vector3(boundBoxDim);
	//m_sphereBoundTransform = m_entityTransform;
	m_boundSphere.m_transform = m_entityTransform;
	m_boxBoundTransform = Transform(m_center, rot, boundBoxScale);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = mass;
		m_massData.m_invMass = 1.f / mass;

		// initialize sphere inertia tensor as needed
		float factor = 0.4f * GetMass3() * m_primitive.GetRadius() * m_primitive.GetRadius();
		Vector3 tensor_i = Vector3(factor, 0.f, 0.f);
		Vector3 tensor_j = Vector3(0.f, factor, 0.f);
		Vector3 tensor_k = Vector3(0.f, 0.f, factor);
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

	m_boundSphere = BoundingSphere(m_center, m_primitive.m_radius);
	//m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);

	Vector3 boundBoxMin = m_center - boundBoxScale / 2.f;
	Vector3 boundBoxMax = m_center + boundBoxScale / 2.f;
	m_boundBox = AABB3(boundBoxMin, boundBoxMax);
	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);

	m_linearDamp = 1.f;
	m_angularDamp = 1.f;		
}

SphereRB3::~SphereRB3()
{

}

void SphereRB3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}

void SphereRB3::UpdateEntitiesTransforms()
{
	m_entityTransform.SetLocalPosition(m_center);

	// rot
	Matrix44 transMat;
	CacheTransform(transMat, m_center, m_orientation);
	Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(transMat);
	m_entityTransform.SetLocalRotation(euler);

	// assume scale is unchanged

	//m_sphereBoundTransform.SetLocalPosition(m_center);
	m_boundSphere.m_transform.SetLocalPosition(m_center);
	m_boxBoundTransform.SetLocalPosition(m_center);
}

void SphereRB3::UpdateInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_P))
		m_frozen = !m_frozen;
}

void SphereRB3::Integrate(float deltaTime)
{
	UpdateInput(deltaTime);

	CacheData();

	if (!m_frozen)
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
