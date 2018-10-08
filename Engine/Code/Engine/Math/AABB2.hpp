#pragma once

#include "Engine/Math/Vector2.hpp"
#include <string>
#include <vector>

class AABB2
{
public:
	Vector2 mins;
	Vector2 maxs;

	Vector2 upNormal;
	Vector2 downNormal;
	Vector2 leftNormal;
	Vector2 rightNormal;

	~AABB2(){}
	AABB2(){}
	AABB2(const AABB2& copy);
	explicit AABB2( float minX, float minY, float maxX, float maxY );
	explicit AABB2( const Vector2& mins, const Vector2& maxs );
	explicit AABB2( const Vector2& center, float radiusX, float radiusY );

	void StretchToIncludePoint( float x, float y );														// note: stretch, not move…
	void StretchToIncludePoint( const Vector2& point );													// note: stretch, not move…
	void AddPaddingToSides( float xPaddingRadius, float yPaddingRadius );
	void Translate( const Vector2& translation );														// move the box; similar to +=
	void Translate( float translationX, float translationY );

	bool IsPointInside( float x, float y ) const;														// is “x,y” within box’s interior?
	bool IsPointInside( const Vector2& point ) const;													// is “point” within box’s interior?
	Vector2 GetDimensions() const;																		// return a Vector2 of ( width, height )
	Vector2 GetCenter() const;																			// return the center position of the box
	std::vector<Vector2> GetCornersFromBLInCounterClockwise() const;									// return position of four corners of the box from bottom left in counter clockwise direction
	Vector2 GetClosestOnBoundPoint(Vector2 point) const;
	float GetRayIntersection(Vector2 origin, Vector2 direction);
	float GetRayIntersection(Vector2 originA, Vector2 endA, Vector2 originB, Vector2 endB);

	void operator+=( const Vector2& translation );														// move (translate) the box
	void operator-=( const Vector2& antiTranslation );
	AABB2 operator+( const Vector2& translation ) const;												// create a (temp) moved box
	AABB2 operator-( const Vector2& antiTranslation ) const;

	void SetFromText( std::string text );

	static const AABB2 ZERO_TO_ONE;
};

const AABB2 Interpolate( const AABB2& start, const AABB2& end, float fractionTowardEnd );