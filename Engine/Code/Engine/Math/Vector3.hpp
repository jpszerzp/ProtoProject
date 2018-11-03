#pragma once

#include "Engine/Math/Vector4.hpp"

#define VEC3_IDENTITY_THRESHOLD 0.00001f

class Vector3
{
public:
	float x;
	float y;
	float z;

	// Construction/Destruction
	~Vector3() {}											
	Vector3() {}						
	Vector3( const Vector3& copyFrom );						
	explicit Vector3( float initialX, float initialY, float initialZ );		
	explicit Vector3(float initial);

	// Operators
	const Vector3 operator+( const Vector3& vecToAdd ) const;		
	const Vector3 operator-( const Vector3& vecToSubtract ) const;	
	const Vector3 operator*( float uniformScale ) const;			
	const Vector3 operator/( float inverseScale ) const;			
	const bool operator<(const Vector3& compared) const;	// for std::set
	const bool operator<=(const Vector3& compared) const;
	const bool operator>(const Vector3& compared) const;
	const bool operator>=(const Vector3& compared) const;
	void operator+=( const Vector3& vecToAdd );						
	void operator-=( const Vector3& vecToSubtract );				
	void operator*=( const float uniformScale );					
	void operator/=( const float uniformDivisor );					
	void operator=( const Vector3& copyFrom );						
	bool operator==( const Vector3& compare ) const;				
	bool operator!=( const Vector3& compare ) const;	
	const float operator[](const int idx) const;
	const Vector3 operator-() const;
 
	float	GetLength() const;
	float	GetLengthSquared() const;
	float	NormalizeAndGetLength();
	void	Normalize();
	Vector3 Cross(const Vector3& rhs) const;
	Vector3 GetNormalized() const;

	Vector4 ToVector4(float w) const;

	void Set(float newX, float newY, float newZ);
	void ToDefault();

	Vector3 RotateAboutAxisWithAngle(float angle, Vector3 axis);

	void Print();

	static bool AreVectorsNearlyIdentical(const Vector3& v1, const Vector3& v2);

	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 UP;
	static const Vector3 GRAVITY;
	static const Vector3 HIGH_GRAVITY;
	static const Vector3 INVALID;
};

const Vector3 GetProjectedVector( const Vector3& vectorToProject, const Vector3& projectOnto );
Vector3 GetMiddlePoint(const Vector3& min, const Vector3& max);
