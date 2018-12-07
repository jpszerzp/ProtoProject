#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector3.hpp"

const Vector4 Vector4::ONE = Vector4(1.f, 1.f, 1.f, 1.f);
const Vector4 Vector4::ZERO = Vector4(0.f, 0.f, 0.f, 0.f);

Vector3 Vector4::ToVector3() const
{
	return Vector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
Vector4::Vector4( const Vector4& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
	, w( copy.w )
{

}


//-----------------------------------------------------------------------------------------------
Vector4::Vector4( float initialX, float initialY, float initialZ, float initialW )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
	, w ( initialW )
{

}


Vector4::Vector4(const Vector3& base, float initialW)
	: x(base.x), y(base.y), z(base.z), w(initialW)
{

}

//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator + ( const Vector4& vecToAdd ) const
{
	return Vector4( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator-( const Vector4& vecToSubtract ) const
{
	return Vector4( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator*( float uniformScale ) const
{
	return Vector4( x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector4 Vector4::operator/( float inverseScale ) const
{
	return Vector4( x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator+=( const Vector4& vecToAdd )
{
	x = x + vecToAdd.x; // #MP1Fixme
	y = y + vecToAdd.y; // #MP1Fixme
	z = z + vecToAdd.z;
	w = w + vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator-=( const Vector4& vecToSubtract )
{
	x = x - vecToSubtract.x; // #MP1Fixme
	y = y - vecToSubtract.y; // #MP1Fixme
	z = z - vecToSubtract.z;
	w = w - vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator*=( const float uniformScale )
{
	x = x * uniformScale; // #MP1Fixme
	y = y * uniformScale; // #MP1Fixme
	z = z * uniformScale;
	w = w * uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator/=( const float uniformDivisor )
{
	x = x / uniformDivisor; // #MP1Fixme
	y = y / uniformDivisor; // #MP1Fixme
	z = z / uniformDivisor;
	w = w / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector4::operator=( const Vector4& copyFrom )
{
	x = copyFrom.x; // #MP1Fixme
	y = copyFrom.y; // #MP1Fixme
	z = copyFrom.z;
	w = copyFrom.w;
}


//-----------------------------------------------------------------------------------------------
const Vector4 operator*( float uniformScale, const Vector4& vecToScale )
{
	return Vector4( uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z, uniformScale * vecToScale.w ); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
bool Vector4::operator==( const Vector4& compare ) const
{
	if ((x == compare.x) && (y == compare.y) && (z == compare.z) && (w == compare.w))
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vector4::operator!=( const Vector4& compare ) const
{
	return !((*this) == compare); // #MP1Fixme
}