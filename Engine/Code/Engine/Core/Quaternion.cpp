#include "Engine/Core/Quaternion.hpp"
#include "Engine/Math/MathUtils.hpp"

const Quaternion Quaternion::IDENTITY = Quaternion(1.f, 0.f, 0.f, 0.f);		// no rotation

Quaternion::Quaternion()
	: m_real(0.f), m_imaginary(Vector3::ZERO)
{
	// quaternion zero identity
}


Quaternion::~Quaternion()
{

}


Quaternion::Quaternion(float scalar, Vector3 vector)
	: m_real(scalar), m_imaginary(vector)
{

}


Quaternion::Quaternion(const Quaternion& copy)
{
	m_real = copy.m_real;
	m_imaginary = copy.m_imaginary;
}


Quaternion::Quaternion(float real, float x, float y, float z)
	: m_real(real), m_imaginary(Vector3(x, y, z))
{

}

Quaternion& Quaternion::operator=(const Quaternion& copy)
{
	m_real = copy.m_real;
	m_imaginary = copy.m_imaginary;

	return *this;
}


Quaternion Quaternion::operator+(const Quaternion& rhs) const 
{
	Quaternion quaternion;

	quaternion.m_real = m_real + rhs.m_real;
	quaternion.m_imaginary = m_imaginary + rhs.m_imaginary;

	return quaternion;
}


Quaternion Quaternion::operator-(const Quaternion& rhs) const 
{
	Quaternion quaternion;

	quaternion.m_real = m_real - rhs.m_real;
	quaternion.m_imaginary = m_imaginary - rhs.m_imaginary;

	return quaternion;
}


Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
	float real = m_real * rhs.m_real - DotProduct(m_imaginary, rhs.m_imaginary);
	Vector3 imaginary = rhs.m_imaginary * m_real + m_imaginary * rhs.m_real + m_imaginary.Cross(rhs.m_imaginary);

	return Quaternion(real, imaginary);
}


Quaternion Quaternion::operator*(const float scalar) const 
{
	float real = m_real * scalar;
	Vector3 imaginary = m_imaginary * scalar;

	return Quaternion(real, imaginary);
}


void Quaternion::operator+=(const Quaternion& rhs)
{
	m_real += rhs.m_real;
	m_imaginary += rhs.m_imaginary;
}


void Quaternion::operator-=(const Quaternion& rhs)
{
	m_real -= rhs.m_real;
	m_imaginary -= rhs.m_imaginary;
}


void Quaternion::operator*=(const Quaternion& rhs)
{
	Quaternion copy = *this;

	Quaternion multiply = copy * rhs;

	m_real = multiply.m_real;
	m_imaginary = multiply.m_imaginary;
}


void Quaternion::operator*=(const float scalar)
{
	m_real *= scalar;
	m_imaginary *= scalar;
}


float Quaternion::GetNorm() const
{
	float realSquare = m_real * m_real;
	float imaginarySquare = DotProduct(m_imaginary, m_imaginary);

	return sqrtf(realSquare + imaginarySquare);
}


float Quaternion::GetNormSqr() const
{
	float realSquare = m_real * m_real;
	float imaginarySquare = DotProduct(m_imaginary, m_imaginary);

	return realSquare + imaginarySquare;
}

void Quaternion::Normalize()
{
	float norm = GetNorm();

	if (norm != 0.f)
	{
		float normMultiplier = 1.f / norm;

		m_real *= normMultiplier;
		m_imaginary *= normMultiplier;
	}
	else 
	{
		// zero length quaternion, use non-rotation quaternion
		// real part is 1
		m_real = 1.f;
	}
}


void Quaternion::ConvertToUnitNormQuaternion()
{
	float rad = ConvertDegreesToRadians(m_real);
	m_imaginary.NormalizeAndGetLength();
	
	m_real = cosf(rad * 0.5f);
	m_imaginary = m_imaginary * sinf(rad * 0.5f);
}


void Quaternion::RotateByVector(const Vector3& vec)
{
	Quaternion q = Quaternion(0.f, vec);
	(*this) *= q;
}

void Quaternion::AddScaledVector(const Vector3& vec, float scale)
{
	Vector3 scaledVec = Vector3(vec.x * scale, vec.y * scale, vec.z * scale);
	Quaternion quat = Quaternion(0.f, scaledVec);
	quat *= *this;
	m_real += quat.m_real * 0.5f;
	m_imaginary.x += quat.m_imaginary.x * 0.5f;
	m_imaginary.y += quat.m_imaginary.y * 0.5f;
	m_imaginary.z += quat.m_imaginary.z * 0.5f;
}

Quaternion Quaternion::GetConjugate() const
{
	float real = m_real;
	Vector3 imaginary = m_imaginary * (-1.f);

	return Quaternion(real, imaginary);
}


Quaternion Quaternion::GetInverse() const
{
	float norm = GetNorm();
	float multiplier = 1.f / (norm * norm);

	Quaternion conjugate = GetConjugate();

	float real = conjugate.m_real * multiplier;
	Vector3 imaginary = conjugate.m_imaginary * multiplier;

	return Quaternion(real, imaginary);
}

Matrix44 Quaternion::GetMatrixRotation(const Quaternion& q)
{
	float x = q.m_imaginary.x;
	float y = q.m_imaginary.y;
	float z = q.m_imaginary.z;
	float w = q.m_real;

	Matrix44 res;
	res.Ix = 1.f - (2.f * y * y + 2 * z * z);
	res.Iy = 2.f * x * y - 2.f * z * w;
	res.Iz = 2.f * x * z + 2.f * y * w;
	//res.Iw = 0.f;
	res.Jx = 2.f * x * y + 2.f * z * w;
	res.Jy = 1.f - (2.f * x * x + 2.f * z * z);
	res.Jz = 2.f * y * z - 2.f * x * w;
	//res.Jw = 0.f;
	res.Kx = 2.f * x * z - 2.f * y * w;
	res.Ky = 2.f * y * z + 2.f * x * w;
	res.Kz = 1.f - (2.f * x * x + 2.f * y * y);
	//res.Kw = 0.f;

	// T column remains unchanged: [0, 0, 0, 1]

	return res;
}

Matrix44 Quaternion::GetMatrixWithPosition(const Quaternion& q, const Vector3& pos)
{
	float x = q.m_imaginary.x;
	float y = q.m_imaginary.y;
	float z = q.m_imaginary.z;
	float w = q.m_real;

	Matrix44 res;
	res.Ix = 1.f - (2.f * y * y + 2 * z * z);
	res.Iy = 2.f * x * y - 2.f * z * w;
	res.Iz = 2.f * x * z + 2.f * y * w;
	//res.Iw = 0.f;
	res.Jx = 2.f * x * y + 2.f * z * w;
	res.Jy = 1.f - (2.f * x * x + 2.f * z * z);
	res.Jz = 2.f * y * z - 2.f * x * w;
	//res.Jw = 0.f;
	res.Kx = 2.f * x * z - 2.f * y * w;
	res.Ky = 2.f * y * z + 2.f * x * w;
	res.Kz = 1.f - (2.f * x * x + 2.f * y * y);
	//res.Kw = 0.f;
	res.Tx = pos.x;
	res.Ty = pos.y;
	res.Tz = pos.z;
	//res.Tw = 1.f;

	return res;
}

Quaternion Quaternion::FromMatrix(const Matrix33& rot)
{
	Quaternion res;

	float m11 = rot.Ix;
	float m12 = rot.Jx;
	float m13 = rot.Kx;
	float m21 = rot.Iy;
	float m22 = rot.Jy;
	float m23 = rot.Ky;
	float m31 = rot.Iz;
	float m32 = rot.Jz;
	float m33 = rot.Kz;

	//float w = 0.f;
	//float x = 0.f;
	//float y = 0.f;
	//float z = 0.f;

	float w_sqrf = m11 + m22 + m33;
	float x_sqrf = m11 - m22 - m33;
	float y_sqrf = m22 - m11 - m33;
	float z_sqrf = m33 - m11 - m22;

	int idx = 0;
	float biggest = w_sqrf;
	if (x_sqrf > biggest)
	{
		biggest = x_sqrf;
		idx = 1;
	}
	if (y_sqrf > biggest)
	{
		biggest = y_sqrf;
		idx = 2;
	}
	if (z_sqrf > biggest)
	{
		biggest = z_sqrf;
		idx = 3;
	}

	float big_sqrt = sqrtf(biggest + 1.f) * .5f;
	float mult = .25f / big_sqrt;

	switch (idx)
	{
	case 0:
	{
		res.m_real = big_sqrt;
		res.m_imaginary.x = (m23 - m32) * mult;
		res.m_imaginary.y = (m31 - m13) * mult;
		res.m_imaginary.z = (m12 - m21) * mult;
	}
		break;
	case 1:
	{
		res.m_imaginary.x = big_sqrt;
		res.m_real = (m23 - m32) * mult;
		res.m_imaginary.y = (m12 + m21) * mult;
		res.m_imaginary.z = (m31 + m13) * mult;
	}
		break;
	case 2:
	{
		res.m_imaginary.y = big_sqrt;
		res.m_real = (m31 - m13) * mult;
		res.m_imaginary.x = (m12 + m21) * mult;
		res.m_imaginary.z = (m23 + m32) * mult;
	}
		break;
	case 3:
	{
		res.m_imaginary.z = big_sqrt;
		res.m_real = (m12 - m21) * mult;
		res.m_imaginary.x = (m31 + m13) * mult;
		res.m_imaginary.y = (m23 + m32) * mult;
	}
		break;
	default:
		break;
	}

	return res;
}
