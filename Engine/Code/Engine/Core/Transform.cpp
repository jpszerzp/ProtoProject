#include "Engine/Core/Transform.hpp"

Matrix44 sTransform::GetMatrixEulerTranspose() const
{
	Matrix44 scaleMatrix = Matrix44::MakeScale3D(m_scale.x, m_scale.y, m_scale.z);
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3DTranspose(m_euler);
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(m_position);

	// t * r * s
	Matrix44 res = Matrix44::IDENTITY;
	res.Append(translationMatrix);
	res.Append(rotationMatrix);
	res.Append(scaleMatrix);

	return res;
}

Matrix44 sTransform::ToWorld() const
{
	Matrix44 scaleMatrix = Matrix44::MakeScale3D(m_scale.x, m_scale.y, m_scale.z);
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3D(m_euler);
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(m_position);

	// trs
	Matrix44 res = Matrix44::IDENTITY;
	res.Append(translationMatrix);
	res.Append(rotationMatrix);
	res.Append(scaleMatrix);

	return res;
}

Matrix44 sTransform::ToLocalOrthogonal() const
{
	Matrix44 scaleMatrix = Matrix44::MakeScale3D(1.f / m_scale.x, 1.f / m_scale.y, 1.f / m_scale.z);
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3D(m_euler).Transpose();		// assumes bases are orthogonal
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(-m_position);

	// srt
	Matrix44 res = Matrix44::IDENTITY;
	res.Append(scaleMatrix);
	res.Append(rotationMatrix);
	res.Append(translationMatrix);

	return res;
}

Matrix44 sTransform::ToLocalGeneral() const
{
	Matrix44 scaleMatrix = Matrix44::MakeScale3D(1.f / m_scale.x, 1.f / m_scale.y, 1.f / m_scale.z);
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3D(m_euler).Invert();
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(-m_position);

	Matrix44 res = Matrix44::IDENTITY;
	res.Append(scaleMatrix);
	res.Append(rotationMatrix);
	res.Append(translationMatrix);

	return res;
}

Matrix44 sTransform::ToWorldNonScale() const
{
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3D(m_euler);
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(m_position);

	// tr
	Matrix44 res = Matrix44::IDENTITY;
	res.Append(translationMatrix);
	res.Append(rotationMatrix);

	return res;
}

Matrix44 sTransform::ToLocalOrtho() const
{
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3D(m_euler).Transpose();		// assumes bases are orthogonal
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(-m_position);

	// srt
	Matrix44 res = Matrix44::IDENTITY;
	res.Append(rotationMatrix);
	res.Append(translationMatrix);

	return res;
}

Matrix44 sTransform::ToLocalGen() const
{
	Matrix44 rotationMatrix = Matrix44::MakeRotationDegrees3D(m_euler).Invert();
	Matrix44 translationMatrix = Matrix44::MakeTranslation3D(-m_position);

	Matrix44 res = Matrix44::IDENTITY;
	res.Append(rotationMatrix);
	res.Append(translationMatrix);

	return res;
}

void sTransform::SetPosition(Vector3 pos)
{
	m_position = pos;
}

void sTransform::Translate( Vector3 offset )
{
	m_position += offset;
}

void sTransform::SetRotationEuler( Vector3 euler )
{
	m_euler = euler;
}

void sTransform::Rotate(Vector3 euler)
{
	m_euler += euler;
}

void sTransform::SetScale(Vector3 s)
{
	m_scale = s;
}

////////////////////////////////////////////// TRANSFORM ///////////////////////////////////////////////

Transform::Transform(Vector3 pos, Vector3 euler, Vector3 scale)
{
	m_localTransform = sTransform(pos, euler, scale);;
}

Transform::Transform()
{
	m_localTransform = sTransform();
}

Transform::~Transform()
{

}

Matrix44 Transform::GetLocalMatrix() const
{
	Matrix44 local;
	if (m_parentTransform == nullptr)
	{
		local = MAT_LOCAL_IDENTITY;
	}
	return local;
}


Matrix44 Transform::GetLocalRotationMatrix() const
{
	Matrix44 rot = Matrix44::MakeRotationDegrees3D(m_localTransform.m_euler);
	return rot;
}

Matrix44 Transform::GetLocalMatrixEulerTranspose() const
{
	return m_localTransform.GetMatrixEulerTranspose();
}


Matrix44 Transform::GetTRMatrix() const
{
	Matrix44 loc = Matrix44::MakeTranslation3D(m_localTransform.m_position);
	Matrix44 rot = Matrix44::MakeRotationDegrees3D(m_localTransform.m_euler);
	Matrix44 res = Matrix44::IDENTITY;
	res.Append(loc);
	res.Append(rot);

	return res;
}


Matrix44 Transform::GetWorldMatrix() const
{
	Matrix44 world;
	Matrix44 toWorld;
	if (m_parentTransform == nullptr)
	{
		toWorld = m_localTransform.ToWorld();
		Matrix44 local = GetLocalMatrix();

		world = toWorld * local;
	}
	return world;
}


Matrix44 Transform::GetWorldMatrixEulerTranspose() const
{
	Matrix44 worldMatrix;
	if (m_parentTransform != nullptr)
	{
		Matrix44 parentWorldMatrix = m_parentTransform->GetWorldMatrixEulerTranspose();
		Matrix44 localMatrix = GetLocalMatrixEulerTranspose();
		worldMatrix = parentWorldMatrix * localMatrix;
	}
	else
	{
		worldMatrix = GetLocalMatrixEulerTranspose();
	}

	return worldMatrix;
}


Vector3 Transform::TransformLocalToWorldPos(Vector3 local, Transform transform)
{
	Matrix44 toWorld = transform.m_localTransform.ToWorld();
	Vector4 localPos = local.ToVector4(1.f);
	Vector4 worldPos = toWorld * localPos;
	Vector3 world = worldPos.ToVector3();
	return world;
}

Vector3 Transform::TransformWorldToLocalPosOrthogonal(const Vector3& world, const Transform& transform)
{
	Matrix44 toLocal = transform.m_localTransform.ToLocalOrthogonal();
	Vector4 worldPos = world.ToVector4(1.f);
	Vector4 localPos = toLocal * worldPos;
	Vector3 local = localPos.ToVector3();
	return local;
}

Vector3 Transform::TransformWorldToLocalPosGeneral(const Vector3& world, const Transform& transform)
{
	Matrix44 toLocal = transform.m_localTransform.ToLocalGeneral();
	Vector4 worldPos = world.ToVector4(1.f);
	Vector4 localPos = toLocal * worldPos;
	Vector3 local = localPos.ToVector3();
	return local;
}

Vector3 Transform::LocalToWorldPos(Vector3 local, Transform transform)
{
	Matrix44 toWorld = transform.m_localTransform.ToWorldNonScale();
	Vector4 localPos = local.ToVector4(1.f);
	Vector4 worldPos = toWorld * localPos;
	Vector3 world = worldPos.ToVector3();
	return world;
}

Vector3 Transform::WorldToLocalOrthogonal(const Vector3& world, const Transform& transform)
{
	Matrix44 toLocal = transform.m_localTransform.ToLocalOrtho();
	Vector4 worldPos = world.ToVector4(1.f);
	Vector4 localPos = toLocal * worldPos;
	Vector3 local = localPos.ToVector3();
	return local;
}

Vector3 Transform::WorldToLocalGeneral(const Vector3& world, const Transform& transform)
{
	Matrix44 toLocal = transform.m_localTransform.ToLocalGen();
	Vector4 worldPos = world.ToVector4(1.f);
	Vector4 localPos = toLocal * worldPos;
	Vector3 local = localPos.ToVector3();
	return local;
}

void Transform::TransformRotationAtoBCoord(Matrix44& rotation, const Transform& A, const Transform& B)
{

}

Vector3 Transform::TransformDirToWorld(Vector3 dir_local, Transform transform)
{
	Matrix44 toWorld = transform.m_localTransform.ToWorld();
	Vector3 dir_world = toWorld.TransformDisplacement3D(dir_local);
	return dir_world;
}

Vector3 Transform::TransformDirToLocal(Vector3 dir_world, Transform transform)
{
	Matrix44 toWorld = transform.m_localTransform.ToWorld();	// try using to world this time :)
	Vector3 dir_local = toWorld.TransformDisplacementInverse3D(dir_world);
	return dir_local;
}

Vector3 Transform::GetWorldPosition() const
{
	Vector3 worldPosition;
	if (m_parentTransform == nullptr)
	{
		Matrix44 worldModel = GetWorldMatrix();
		worldPosition = Vector3(worldModel.Tx, worldModel.Ty, worldModel.Tz);
	}
	return worldPosition;
}


Vector3 Transform::GetWorldScale() const
{
	Vector3 worldScale;
	if (m_parentTransform == nullptr)
	{
		Matrix44 worldModel = GetWorldMatrix();
		worldScale = Vector3(worldModel.Ix, worldModel.Jy, worldModel.Kz);
	}
	return worldScale;
}

Vector3 Transform::GetWorldForward() const
{
	Vector4 worldForward;
	if (m_parentTransform == nullptr)
	{
		Matrix44 toWorld = m_localTransform.ToWorld();
		Vector4 localForward = GetLocalForward().ToVector4(0.f);
		worldForward = toWorld * localForward;
	}
	return worldForward.ToVector3();
}

Vector3 Transform::GetWorldUp() const
{
	Vector4 worldUp;
	if (m_parentTransform == nullptr)
	{
		Matrix44 toWorld = m_localTransform.ToWorld();
		Vector4 localUp = GetLocalUp().ToVector4(0.f);
		worldUp = toWorld * localUp;
	}
	return worldUp.ToVector3();
}

Vector3 Transform::GetWorldRight() const
{
	Vector4 worldRight;
	if (m_parentTransform == nullptr)
	{
		Matrix44 toWorld = m_localTransform.ToWorld();
		Vector4 localRight = GetLocalRight().ToVector4(0.f);
		worldRight = toWorld * localRight;
	}
	return worldRight.ToVector3();
}

void Transform::SetLocalPosition(Vector3 pos)
{
	m_localTransform.SetPosition(pos);
}

void Transform::TranslateLocal( Vector3 offset )
{
	m_localTransform.Translate(offset);
}


void Transform::SetLocalRotation( Vector3 euler )
{
	m_localTransform.SetRotationEuler(euler);
}

void Transform::RotateLocal( Vector3 euler )
{
	m_localTransform.Rotate(euler);
}


void Transform::SetLocalScale( Vector3 s )
{
	m_localTransform.SetScale(s);
}


void Transform::SetParentTransform(Transform* parentTransform)
{
	m_parentTransform = parentTransform;
}


Vector3 Transform::GetLocalForward() const
{
	return Vector3(0.f, 0.f, 1.f);
}


Vector3 Transform::GetLocalUp() const
{
	return Vector3(0.f, 1.f, 0.f);
}


Vector3 Transform::GetLocalRight() const
{
	return Vector3(1.f, 0.f, 0.f);
}

