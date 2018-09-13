#pragma once

class Vector3;

class Vector4
{
public:
	float x;
	float y;
	float z;
	float w;

	// Construction/Destruction
	~Vector4() {}											
	Vector4() {}								
	Vector4( const Vector4& copyFrom );						
	explicit Vector4( float initialX, float initialY, float initialZ, float initialW);		
	explicit Vector4(const Vector3& base, float initialW);

	// Operators
	const Vector4 operator+( const Vector4& vecToAdd ) const;		
	const Vector4 operator-( const Vector4& vecToSubtract ) const;	
	const Vector4 operator*( float uniformScale ) const;			
	const Vector4 operator/( float inverseScale ) const;			
	void operator+=( const Vector4& vecToAdd );						
	void operator-=( const Vector4& vecToSubtract );				
	void operator*=( const float uniformScale );					
	void operator/=( const float uniformDivisor );					
	void operator=( const Vector4& copyFrom );						
	bool operator==( const Vector4& compare ) const;				
	bool operator!=( const Vector4& compare ) const;		

	static const Vector4 ONE;
	static const Vector4 ZERO;

	Vector3 ToVector3() const;
};