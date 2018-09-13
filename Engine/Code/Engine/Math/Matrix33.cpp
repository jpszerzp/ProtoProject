#include "Engine/Math/Matrix33.hpp"
#include "Engine/Math/MathUtils.hpp"

const Matrix33 Matrix33::IDENTITY = Matrix33();

Matrix33::Matrix33()
{
	Ix = 1.f; 	Jx = 0.f;	Kx = 0.f;	
	Iy = 0.f;	Jy = 1.f;	Ky = 0.f;	
	Iz = 0.f;	Jz = 0.f;	Kz = 1.f;	
}


Matrix33::Matrix33(const Vector3& i, const Vector3& j, const Vector3& k)
{
	Ix = i.x; Jx = j.x; Kx = k.x;
	Iy = i.y; Jy = j.y; Ky = k.y;
	Iz = i.z; Jz = j.z; Kz = k.z;
}

Vector3 Matrix33::operator*(const Vector3& rhs) const
{
	Vector3 row1 = Vector3(Ix, Jx, Kx);
	Vector3 row2 = Vector3(Iy, Jy, Ky);
	Vector3 row3 = Vector3(Iz, Jz, Kz);

	float x = DotProduct(row1, rhs);
	float y = DotProduct(row2, rhs);
	float z = DotProduct(row3, rhs);

	return Vector3(x, y, z);
}

void Matrix33::operator*(const float rhs)
{
	Ix *= rhs;		Jx *= rhs;		Kx *= rhs;
	Iy *= rhs;		Jy *= rhs;		Ky *= rhs;
	Iz *= rhs;		Jz *= rhs;		Kz *= rhs;
}

float Matrix33::GetDeterminant() const
{
	float det = Ix * Jy * Kz + Iy * Jz * Kx + Iz * Jx * Ky
		- Ix * Jz * Ky - Iz * Jy * Kx  - Iy * Jx * Kz;
	return det;
}

Matrix33 Matrix33::Invert() const
{
	// det
	Matrix33 res;
	
	res.Ix = Jy * Kz - Ky * Jz;
	res.Iy = Ky * Iz - Iy * Kz;
	res.Iz = Iy * Jz - Jy * Iz;
	res.Jx = Kx * Jz - Jx * Kz;
	res.Jy = Ix * Kz - Kx * Iz;
	res.Jz = Jx * Iz - Ix * Jz;
	res.Kx = Jx * Ky - Kx * Jy;
	res.Ky = Kx * Iy - Ix * Ky;
	res.Kz = Ix * Jy - Jx * Iy;

	float det = GetDeterminant();
	if (det == 0.f)
		return Matrix33::IDENTITY;
	float det_factor = 1.f / det;
	res * det_factor;
	
	return res;
}

void Matrix33::SetInverse(const Matrix33& mat)
{
	Matrix33 invertedMat = mat.Invert();
	Ix = invertedMat.Ix;	Jx = invertedMat.Jx;	Kx = invertedMat.Kx;
	Iy = invertedMat.Iy;	Jy = invertedMat.Jy;	Ky = invertedMat.Ky;
	Iz = invertedMat.Iz;	Jz = invertedMat.Jz;	Kz = invertedMat.Kz;
}

Matrix33 Matrix33::Transpose() const
{
	Vector3 newI = Vector3(Ix, Jx, Kx);
	Vector3 newJ = Vector3(Iy, Jy, Ky);
	Vector3 newK = Vector3(Iz, Jz, Kz);
	return Matrix33(newI, newJ, newK);
}
