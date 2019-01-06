#include "Engine/Physics/3D/QuadRB3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

QuadRB3::QuadRB3(float mass, const Plane& primitive, 
	const Vector3& center, const Vector3& euler, const Vector3& scale, const eMoveStatus& moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_RIGID;

	m_linearVelocity = Vector3::ZERO;
	m_center = center;

	// entity transform
	m_entityTransform = Transform(m_center, euler, scale);
	
	// euler to quaternion
	Matrix44 rot_mat = Matrix44::MakeRotationDegrees3D(euler);
	Matrix33 rot_only = rot_mat.ExtractMat3();
	m_orientation = Quaternion::FromMatrix(rot_only);

	// BV transform - sphere
	Vector3 boundSpherePos = m_center;
	Vector3 boundSphereRot = Vector3::ZERO;
	float boundSphereRadius = sqrtf(scale.x * scale.x + scale.y * scale.y) / 2.f;
	Vector3 boundSphereScale = Vector3(boundSphereRadius, boundSphereRadius, boundSphereRadius);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = mass;
		m_massData.m_invMass = 1.f / mass;

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

	// update rot of plane is just updating the normal and offset, 
	// quite different from other primitives
	Vector3 new_forward_norm = m_entityTransform.GetWorldForward().GetNormalized();
	float new_offset = DotProduct(m_center, new_forward_norm);
	m_primitive.m_normal = new_forward_norm;
	m_primitive.m_offset = new_offset;
}

void QuadRB3::UpdateTransforms()
{
	m_entityTransform.SetLocalPosition(m_center);

	// with the updated quaternion, we want to update euler
	// meaning follow combo of functions return euler from quaternion
	Matrix44 rot_mat_44 = Quaternion::GetMatrixRotation(m_orientation);
	Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(rot_mat_44);
	m_entityTransform.SetLocalRotation(euler);

	m_boundSphere.m_transform.SetLocalPosition(m_center);
}

void QuadRB3::Integrate(float deltaTime)
{
	float usedTime = deltaTime;

	if (m_scheme == CONTINUOUS)
	{
		if (m_motionClamp)
			usedTime = m_motionClampTime;

		if (!m_awake) return;

		UpdateInput(usedTime);

		UpdateDynamicsCore(usedTime);

		if (m_scheme == CONTINUOUS && m_motionClamp)
			m_motionClamp = false;

		UpdateSleepSystem(usedTime);
	}
	else
	{
		usedTime *= m_slowed;

		if (!m_awake) return;

		UpdateInput(usedTime);

		UpdateDynamicsCore(usedTime);

		UpdateSleepSystem(usedTime);
	}
}

