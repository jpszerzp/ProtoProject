#pragma once
#include "Engine/Math/Vector3.hpp"
#include <string>

enum Quadrant
{
	First,
	Second,
	Third,
	Fourth,
	OnAxis,
	NUM_QUADRANT
};

class IntVector2;

//-----------------------------------------------------------------------------------------------
class Vector2
{
public:
	// Construction/Destruction
	~Vector2() {}											// destructor: do nothing (for speed)
	Vector2() {}											// default constructor: do nothing (for speed)
	Vector2( const Vector2& copyFrom );						// copy constructor (from another vec2)
	explicit Vector2( float initialX, float initialY );		// explicit constructor (from x, y)
	explicit Vector2( const float& initialXY );

	// Operators
	const Vector2 operator+( const Vector2& vecToAdd ) const;		// vec2 + vec2
	const Vector2 operator-( const Vector2& vecToSubtract ) const;	// vec2 - vec2
	const Vector2 operator*( float uniformScale ) const;			// vec2 * float
	const Vector2 operator/( float inverseScale ) const;			// vec2 / float
	void operator+=( const Vector2& vecToAdd );						// vec2 += vec2
	void operator-=( const Vector2& vecToSubtract );				// vec2 -= vec2
	void operator*=( const float uniformScale );					// vec2 *= float
	void operator/=( const float uniformDivisor );					// vec2 /= float
	void operator=( const Vector2& copyFrom );						// vec2 = vec2
	bool operator==( const Vector2& compare ) const;				// vec2 == vec2
	bool operator!=( const Vector2& compare ) const;				// vec2 != vec2

	friend const Vector2 operator*( float uniformScale, const Vector2& vecToScale );	// float * vec2
	const Vector2 operator- () const;

	float		GetLength() const;
	float		GetLengthSquared() const;
	float		NormalizeAndGetLength();
	float		GetOrientationDegrees() const;
	float		GetOrientationDegrees0To360() const;
	float		GetDistanceToLineSegment(Vector2 lineStart, Vector2 lineEnd) const;
	Vector2		GetNormalized() const;
	Vector2		RotateDegree2D(float deg);
	Vector3		ToVector3(float z);
	IntVector2  ToIntVec2();

	static float	ComputeDegOfVector2s(Vector2& a, Vector2& b);
	static Vector2	MakeDirectionAtDegrees(float degrees);
	static Vector2	ConvertQuadToVec2(Quadrant quad);
	static Vector2	GetRandomPositionVector2(float widthRange, float heightRange);
	static Quadrant ConvertVec2ToQuad(Vector2 loc);
	static Quadrant ComputeQuadrantAtDegrees(float degrees);

	void SetFromText( const char* text );
	
	float x;
	float y;
	static const Vector2 ZERO;
	static const Vector2 ONE;
};

void DecomposeVectorIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ );
const Vector2 GetPerpendicularVector(const Vector2& vectorToProject, const Vector2& projectOnto);
const Vector2 GetProjectedVector( const Vector2& vectorToProject, const Vector2& projectOnto );
const Vector2 GetTransformedIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ );
const Vector2 GetTransformedOutOfBasis( const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ );
const Vector2 Reflect(const Vector2& inVec, const Vector2& normal);
const Vector2 Interpolate( const Vector2& start, const Vector2& end, float fractionTowardEnd );
const std::string ToString(const Vector2& vector);
const float	CrossProduct2(Vector2 p1, Vector2 p2);