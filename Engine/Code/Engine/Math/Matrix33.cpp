#include "Engine/Math/Matrix33.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const Matrix33 Matrix33::IDENTITY = Matrix33();
const Matrix33 Matrix33::ZERO = Matrix33(0.f);

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

Matrix33::Matrix33(const float* entries)
{
	Ix = entries[0];	Jx = entries[3];	Kx = entries[6];
	Iy = entries[1];	Jy = entries[4];	Ky = entries[7];
	Iz = entries[2];	Jz = entries[5];	Kz = entries[8];
}

Matrix33::Matrix33(float entry)
{
	Ix = entry; Jx = entry;	Kx = entry;	
	Iy = entry;	Jy = entry;	Ky = entry;	
	Iz = entry;	Jz = entry;	Kz = entry;	
}

void Matrix33::operator+=(const Matrix33& rhs)
{
	Ix += rhs.Ix; Jx += rhs.Jx; Kx += rhs.Kx;
	Iy += rhs.Iy; Jy += rhs.Jy; Ky += rhs.Ky;
	Iz += rhs.Iz; Jz += rhs.Jz; Kz += rhs.Kz;
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

Matrix33 Matrix33::operator*(const Matrix33& rhs) const
{
	Matrix33 res;

	Vector3 my_row1 = Vector3(Ix, Jx, Kx);
	Vector3 my_row2 = Vector3(Iy, Jy, Ky);
	Vector3 my_row3 = Vector3(Iz, Jz, Kz);

	Vector3 other_col1 = Vector3(rhs.Ix, rhs.Iy, rhs.Iz);
	Vector3 other_col2 = Vector3(rhs.Jx, rhs.Jy, rhs.Jz);
	Vector3 other_col3 = Vector3(rhs.Kx, rhs.Ky, rhs.Kz);

	res.Ix = DotProduct(my_row1, other_col1);
	res.Iy = DotProduct(my_row2, other_col1);
	res.Iz = DotProduct(my_row3, other_col1);
	res.Jx = DotProduct(my_row1, other_col2);
	res.Jy = DotProduct(my_row2, other_col2);
	res.Jz = DotProduct(my_row3, other_col2);
	res.Kx = DotProduct(my_row1, other_col3);
	res.Ky = DotProduct(my_row2, other_col3);
	res.Kz = DotProduct(my_row3, other_col3);

	return res;
}

void Matrix33::operator*=(const Matrix33& rhs)
{
	Vector3 my_row1 = Vector3(Ix, Jx, Kx);
	Vector3 my_row2 = Vector3(Iy, Jy, Ky);
	Vector3 my_row3 = Vector3(Iz, Jz, Kz);

	Vector3 other_col1 = Vector3(rhs.Ix, rhs.Iy, rhs.Iz);
	Vector3 other_col2 = Vector3(rhs.Jx, rhs.Jy, rhs.Jz);
	Vector3 other_col3 = Vector3(rhs.Kx, rhs.Ky, rhs.Kz);

	Ix = DotProduct(my_row1, other_col1);
	Iy = DotProduct(my_row2, other_col1);
	Iz = DotProduct(my_row3, other_col1);
	Jx = DotProduct(my_row1, other_col2);
	Jy = DotProduct(my_row2, other_col2);
	Jz = DotProduct(my_row3, other_col2);
	Kx = DotProduct(my_row1, other_col3);
	Ky = DotProduct(my_row2, other_col3);
	Kz = DotProduct(my_row3, other_col3);
}

void Matrix33::operator*=(const float scale)
{
	Ix *= scale; Jx *= scale; Kx *= scale;
	Iy *= scale; Jy *= scale; Ky *= scale;
	Iz *= scale; Jz *= scale; Kz *= scale;
}


const float Matrix33::operator[](const int idx) const
{
	switch (idx)
	{
	case 0: return Ix;
	case 1: return Iy;
	case 2: return Iz;
	case 3: return Jx;
	case 4: return Jy;
	case 5: return Jz;
	case 6: return Kx;
	case 7: return Ky;
	case 8: return Kz;
	default:
		ASSERT_OR_DIE(false, "Invalid matrix entry index");
	}
}

float Matrix33::GetDeterminant() const
{
	float det = Ix * Jy * Kz + Iy * Jz * Kx + Iz * Jx * Ky
		- Ix * Jz * Ky - Iz * Jy * Kx  - Iy * Jx * Kz;
	return det;
}

float Matrix33::GetTrace() const
{
	return Ix + Jy + Kz;
}

void Matrix33::SetRight(Vector3 right)
{
	Ix = right.x; Iy = right.y; Iz = right.z;
}

void Matrix33::SetUp(Vector3 up)
{
	Jx = up.x; Jy = up.y; Jz = up.z;
}

void Matrix33::SetForward(Vector3 forward)
{
	Kx = forward.x; Ky = forward.y; Kz = forward.z;
}

void Matrix33::SetBasis(Vector3 right, Vector3 up, Vector3 forward)
{
	SetRight(right);
	SetUp(up);
	SetForward(forward);
}

void Matrix33::SetSkewSymmetric(const Vector3& v)
{
	Ix = 0.f;    Jx = -v.z;   Kx = v.y;
	Iy = v.z;    Jy = 0.f;    Ky = -v.x;
	Iz = -v.y;   Jz = v.x;    Kz = 0.f;
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

Vector3 Matrix33::MultiplyTranspose(const Vector3& v) const
{
	Matrix33 t = this->Transpose();
	return t * v;
}

Matrix33 Matrix33::FromEuler(const Vector3& euler)
{
	Matrix33 res = IDENTITY;

	//// yaw pitch roll - column major
	//res.Ix = CosDegrees(euler.y) * CosDegrees(euler.z) + SinDegrees(euler.y) * SinDegrees(euler.x) * SinDegrees(euler.z);
	//res.Iy = -CosDegrees(euler.y) * SinDegrees(euler.z) + SinDegrees(euler.y) * SinDegrees(euler.x) * CosDegrees(euler.z);
	//res.Iz = SinDegrees(euler.y) * CosDegrees(euler.x);

	//res.Jx = SinDegrees(euler.z) * CosDegrees(euler.x);
	//res.Jy = CosDegrees(euler.z) * CosDegrees(euler.x);
	//res.Jz = -SinDegrees(euler.x);

	//res.Kx = -SinDegrees(euler.y) * CosDegrees(euler.z) + CosDegrees(euler.y) * SinDegrees(euler.x) * SinDegrees(euler.z);
	//res.Ky = SinDegrees(euler.z) * SinDegrees(euler.y) + CosDegrees(euler.y) * SinDegrees(euler.x) * CosDegrees(euler.z);
	//res.Kz = CosDegrees(euler.y) * CosDegrees(euler.x);

	// yaw pitch roll
	Vector3 pitch_x = Vector3(1.f, 0.f, 0.f);
	Vector3 pitch_y = Vector3(0.f, CosDegrees(euler.x), SinDegrees(euler.x));
	Vector3 pitch_z = Vector3(0.f, -SinDegrees(euler.x), CosDegrees(euler.x));
	Matrix33 pitch = Matrix33(pitch_x, pitch_y, pitch_z);

	Vector3 yaw_x = Vector3(CosDegrees(euler.y), 0.f, -SinDegrees(euler.y));
	Vector3 yaw_y = Vector3(0.f, 1.f, 0.f);
	Vector3 yaw_z = Vector3(SinDegrees(euler.y), 0.f, CosDegrees(euler.y));
	Matrix33 yaw = Matrix33(yaw_x, yaw_y, yaw_z);

	Vector3 roll_x = Vector3(CosDegrees(euler.z), SinDegrees(euler.z), 0.f);
	Vector3 roll_y = Vector3(-SinDegrees(euler.z), CosDegrees(euler.z), 0.f);
	Vector3 roll_z = Vector3(0.f, 0.f, 1.f);
	Matrix33 roll = Matrix33(roll_x, roll_y, roll_z);

	res = roll * res;
	res = pitch * res;
	res = yaw * res;

	return res;
}
