#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Quaternion.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <math.h>


const Vector3 Vector3::ZERO = Vector3(0.f, 0.f, 0.f);
const Vector3 Vector3::ONE = Vector3(1.f, 1.f, 1.f);
const Vector3 Vector3::UP = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::GRAVITY = Vector3(0.f, -9.81f, 0.f);
const Vector3 Vector3::HIGH_GRAVITY = Vector3(0.f, -19.62f, 0.f);
const Vector3 Vector3::INVALID = Vector3(-INFINITY);

//-----------------------------------------------------------------------------------------------
Vector3::Vector3( const Vector3& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
{

}


//-----------------------------------------------------------------------------------------------
Vector3::Vector3( float initialX, float initialY, float initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{

}


Vector3::Vector3(float initial)
	: x(initial)
	, y(initial)
	, z(initial)
{

}

//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator + ( const Vector3& vecToAdd ) const
{
	return Vector3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z ); // #MP1Fixme
}


const float Vector3::operator[](const int idx) const
{
	if (idx == 0)
		return x;
	else if (idx == 1)
		return y;
	else if (idx == 2)
		return z;
	else
	{
		ASSERT_OR_DIE(false, "Vector3 index must be 0, 1 or 2");
		return -INFINITY;
	}
}

float& Vector3::operator[](const int& idx)
{
	if (idx == 1)
		return x;

	if (idx == 2)
		return y;

	return z;
}

//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator-( const Vector3& vecToSubtract ) const
{
	return Vector3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator*( float uniformScale ) const
{
	return Vector3( x * uniformScale, y * uniformScale, z * uniformScale ); // #MP1Fixme
}

// changes current vector
const Vector3 Vector3::operator*(const Vector3& toMultiply) const
{
	Vector3 res;

	res.x = x * toMultiply.x;
	res.y = y * toMultiply.y;
	res.z = z * toMultiply.z;

	return res;
}

//-----------------------------------------------------------------------------------------------
const Vector3 Vector3::operator/( float inverseScale ) const
{
	return Vector3( x / inverseScale, y / inverseScale, z / inverseScale); // #MP1Fixme
}


const bool Vector3::operator<(const Vector3& compared) const
{
	float myLengthSqr = GetLengthSquared();
	float otherLengthSqr = compared.GetLengthSquared();
	bool comparison = (myLengthSqr < otherLengthSqr);
	return comparison;
}

const bool Vector3::operator<=(const Vector3& compared) const
{
	float myLengthSqr = GetLengthSquared();
	float otherLengthSqr = compared.GetLengthSquared();
	bool comparison = (myLengthSqr <= otherLengthSqr);
	return comparison;
}

const bool Vector3::operator>(const Vector3& compared) const
{
	float myLengthSqr = GetLengthSquared();
	float otherLengthSqr = compared.GetLengthSquared();
	bool comparison = (myLengthSqr > otherLengthSqr);
	return comparison;
}

const bool Vector3::operator>=(const Vector3& compared) const
{
	float myLengthSqr = GetLengthSquared();
	float otherLengthSqr = compared.GetLengthSquared();
	bool comparison = (myLengthSqr >= otherLengthSqr);
	return comparison;
}

//-----------------------------------------------------------------------------------------------
void Vector3::operator+=( const Vector3& vecToAdd )
{
	x = x + vecToAdd.x; // #MP1Fixme
	y = y + vecToAdd.y; // #MP1Fixme
	z = z + vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator-=( const Vector3& vecToSubtract )
{
	x = x - vecToSubtract.x; // #MP1Fixme
	y = y - vecToSubtract.y; // #MP1Fixme
	z = z - vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator*=( const float& uniformScale )
{
	x = x * uniformScale; // #MP1Fixme
	y = y * uniformScale; // #MP1Fixme
	z = z * uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator/=( const float& uniformDivisor )
{
	x = x / uniformDivisor; // #MP1Fixme
	y = y / uniformDivisor; // #MP1Fixme
	z = z / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector3::operator=( const Vector3& copyFrom )
{
	x = copyFrom.x; // #MP1Fixme
	y = copyFrom.y; // #MP1Fixme
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vector3 operator*( float uniformScale, const Vector3& vecToScale )
{
	return Vector3( uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
bool Vector3::operator==( const Vector3& compare ) const
{
	if ((x == compare.x) && (y == compare.y) && (z == compare.z))
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vector3::operator!=( const Vector3& compare ) const
{
	return !((*this) == compare); // #MP1Fixme
}


const Vector3 Vector3::operator-() const
{
	Vector3 flipped;
	
	flipped.x = -x;
	flipped.y = -y;
	flipped.z = -z;

	return flipped;
}


float Vector3::GetLength() const
{
	return sqrtf( (x * x ) + (y * y) + (z * z) );
}


float Vector3::GetLengthSquared() const
{
	return ( (x * x) + (y * y) + (z * z) );
}


float Vector3::NormalizeAndGetLength()
{
	float length = GetLength();

	float scale = 1.f / length;

	x = x * scale;
	y = y * scale;
	z = z * scale;

	return length;
}


void Vector3::Normalize()
{
	float length = GetLength();

	float scale = 1.f / length;

	x = x * scale;
	y = y * scale;
	z = z * scale;
}

Vector3 Vector3::GetNormalized() const
{
	float length = GetLength();

	float scale = 1.f / length;

	float newX = x * scale;
	float newY = y * scale;
	float newZ = z * scale;

	return Vector3(newX, newY, newZ);
}


Vector4 Vector3::ToVector4(float w) const
{
	return Vector4(x, y, z, w);
}


Vector3 Vector3::Cross(const Vector3& rhs) const
{
	float newX = y * rhs.z - z * rhs.y;
	float newY = z * rhs.x - x * rhs.z;
	float newZ = x * rhs.y - y * rhs.x;
	return Vector3(newX, newY, newZ);
}


void Vector3::Set(float newX, float newY, float newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}


void Vector3::ToDefault()
{
	x = 0.f;
	y = 0.f;
	z = 0.f;
}


Vector3 Vector3::RotateAboutAxisWithAngle(float angle, Vector3 axis)
{
	// pure quaternion for rotated vector - zero scalar
	Quaternion pureQuaternion(0.f, (*this));

	// normalize axis
	axis.NormalizeAndGetLength();

	// create quaternion
	Quaternion realQuaternion(angle, axis);

	// convert to unit norm
	realQuaternion.ConvertToUnitNormQuaternion();

	// inverse of quaternion
	Quaternion inverseQuaternion = realQuaternion.GetInverse();

	// rotate quaternion
	Quaternion rotatedQuaternion = realQuaternion * pureQuaternion * inverseQuaternion;

	// return vector part
    return rotatedQuaternion.m_imaginary;
}


const Vector3 GetProjectedVector( const Vector3& vectorToProject, const Vector3& projectOnto )
{
	Vector3 normalized = projectOnto.GetNormalized();
	float length = DotProduct(vectorToProject, normalized);

	return length * normalized;
}


void Vector3::Print()
{
	DebuggerPrintf("x: %.2f, y: %.2f, z: %.2f", x, y, z);
}


bool Vector3::AreVectorsNearlyIdentical(const Vector3& v1, const Vector3& v2)
{
	Vector3 disp = v1 - v2;
	float dist = disp.GetLength();

	return (dist < VEC3_IDENTITY_THRESHOLD);
}

bool Vector3::EqualByTolerance(const float& tolerance, const Vector3& v1, const Vector3& v2)
{
	if ((v1 - v2).GetLengthSquared() <= (tolerance * tolerance))
		return true;
	return false;
}

Vector3 GetMiddlePoint(const Vector3& min, const Vector3& max)
{
	float middleX = (min.x + max.x) / 2.f;
	float middleY = (min.y + max.y) / 2.f;
	float middleZ = (min.z + max.z) / 2.f;

	Vector3 middlePoint = Vector3(middleX, middleY, middleZ);

	return middlePoint;
}