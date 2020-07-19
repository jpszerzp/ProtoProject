#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"

Camera::Camera(Vector3 pos)
{
	m_view = Matrix44();
	m_proj = Matrix44();

	// Default transform: position - pos, euler - ZERO, scale - ONE
	m_transform.SetLocalPosition(pos);
	m_transform.SetLocalRotation(Vector3::ZERO);
	m_transform.SetLocalScale(Vector3::ONE);
}

Camera::~Camera()
{

}

void Camera::SetProjectionOrtho(float sizeX, float sizeY, float theNear, float theFar)
{
	// center at (0, 0)
	Vector2 bl = Vector2(-sizeX * .5f, -sizeY * .5f);
	Vector2 tr = Vector2(sizeX * .5f, sizeY * .5f);
	SetProjectionOrtho(bl, tr, theNear, theFar);
}

void Camera::SetProjectionOrtho(Vector2 bl, Vector2 tr, float theNear, float theFar )
{
	Matrix44 res = Matrix44();

	res.Ix = 2.f / (tr.x - bl.x);
	res.Jy = 2.f / (tr.y - bl.y);
	res.Kz = -2.f / (theFar - theNear);
	res.Tx = -(tr.x + bl.x) / (tr.x - bl.x);
	res.Ty = -(tr.y + bl.y) / (tr.y - bl.y);
	res.Tz = -(theFar + theNear) / (theFar - theNear);

	m_proj = res;
}

void Camera::SetProjectionPerspective(float fovDegrees, float aspect, float nz, float fz)
{
	float d = 1.f / tanf(ConvertDegreesToRadians(fovDegrees) / 2.f);
	float q = 1.f / (fz - nz); 

	Vector4 i = Vector4( d / aspect,   0.f,		0.f,					0.f );  
	Vector4 j = Vector4( 0.f,          d,		0.f,					0.f ); 
	Vector4 k = Vector4( 0.f,          0.f,		(nz + fz) * q,			1.f );
	Vector4 t = Vector4( 0.f,          0.f,		-2.f * fz * nz * q,		0.f ); 

	m_proj = Matrix44( i, j, k, t ); 
}

void Camera::SetProjectionPerspective(float t, float b, float l, float r, float nz, float fz)
{
	Matrix44 res = Matrix44();

	res.Ix = 2.f * nz / (r - l);
	res.Jy = 2.f * nz / (t - b);
	res.Kx = (r + l) / (r - l);
	res.Ky = (t + b) / (t - b);
	res.Kz = -(fz + nz) / (fz - nz);
	res.Kw = -1.f;
	res.Tz = -2.f * fz * nz / (fz - nz);
	res.Tw = 0.f;

	m_proj = res;
}

void Camera::Finalize()
{
	m_output.Finalize();
}

Vector3 Camera::GetLocalForward() const
{
	Matrix44 modelMatrix = m_transform.GetLocalMatrixEulerTranspose();
	Vector3 forward = Vector3(modelMatrix.Kx, modelMatrix.Ky, modelMatrix.Kz);
	return forward;
}

Vector3 Camera::GetLocalUp() const
{
	Matrix44 modelMatrix = m_transform.GetLocalMatrixEulerTranspose();
	Vector3 up = Vector3(modelMatrix.Jx, modelMatrix.Jy, modelMatrix.Jz);
	return up;
}

Vector3 Camera::GetLocalRight() const
{
	Matrix44 modelMatrix = m_transform.GetLocalMatrixEulerTranspose();
	Vector3 right = Vector3(modelMatrix.Ix, modelMatrix.Iy, modelMatrix.Iz);
	return right;
}

Vector3 Camera::GetWorldForward() const
{
	Matrix44 worldModelMatrix = m_transform.GetWorldMatrixEulerTranspose();
	Vector3 forward = Vector3(worldModelMatrix.Kx, worldModelMatrix.Ky, worldModelMatrix.Kz);
	return forward;
}

Vector3 Camera::GetWorldUp() const
{
	Matrix44 worldModelMatrix = m_transform.GetWorldMatrixEulerTranspose();
	Vector3 up = Vector3(worldModelMatrix.Jx, worldModelMatrix.Jy, worldModelMatrix.Jz);
	return up;
}

Vector3 Camera::GetWorldRight() const
{
	Matrix44 worldModelMatrix = m_transform.GetWorldMatrixEulerTranspose();
	Vector3 right = Vector3(worldModelMatrix.Ix, worldModelMatrix.Iy, worldModelMatrix.Iz);
	return right;
}

Vector3 Camera::GetWorldPosition() const
{
	Matrix44 worldModelMatrix = m_transform.GetWorldMatrixEulerTranspose();
	Vector3 translation = Vector3(worldModelMatrix.Tx, worldModelMatrix.Ty, worldModelMatrix.Tz);
	return translation;
}

Matrix44 Camera::LookAtLocal(Vector3 position, Vector3 target, Vector3 up)
{
	Vector3 forward = target - position;
	forward.NormalizeAndGetLength();   

	Vector3 right = up.Cross(forward);
	right.NormalizeAndGetLength();

	Vector3 upDir = forward.Cross(right); 

	Matrix44 matrix;

	// transposed (for camera)
	float translationX = -right.x * position.x - right.y * position.y - right.z * position.z;
	float translationY = -upDir.x * position.x - upDir.y * position.y - upDir.z * position.z;
	float translationZ = -forward.x * position.x - forward.y * position.y - forward.z * position.z;
	float entries[16] = { right.x, upDir.x, forward.x, 0.f, 
		right.y, upDir.y, forward.y, 0.f, 
		right.z, upDir.z, forward.z, 0.f, 
		translationX, translationY, translationZ, 1.f
	};

	matrix.SetValues(entries);
	return matrix;
}