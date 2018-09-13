
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"

Disc2::Disc2(const Disc2& copyFrom)
{
	center = copyFrom.center;
	radius = copyFrom.radius;
}


Disc2::Disc2(float initialX, float initialY, float initialRadius)
	: center(Vector2(initialX, initialY))
	, radius(initialRadius)
{

}


Disc2::Disc2(const Vector2& initialCenter, float initialRadius)
	: center(initialCenter)
	, radius(initialRadius)
{

}


void Disc2::StretchToIncludePoint( float x, float y )
{
	Vector2 desiredPoint = Vector2(x, y);
	float distance = GetDistance(desiredPoint, center);

	if (radius < distance)
	{
		radius = distance;
	}
}


void Disc2::StretchToIncludePoint( const Vector2& point) 
{
	StretchToIncludePoint( point.x, point.y );
}


void Disc2::AddPaddingToRadius(float paddingRadius)
{
	float resultRadius = radius + paddingRadius;
	
	if (resultRadius > 0)
	{
		radius = resultRadius;
	}
}


void Disc2::Translate( const Vector2& translation )
{
	center = center + translation;
}


void Disc2::Translate( float translationX, float translationY )
{
	Translate(Vector2(translationX, translationY));
}


bool Disc2::IsPointInside( float x, float y ) const
{
	return ( radius >= ( GetDistance( center, Vector2( x, y ) ) ) );
}


bool Disc2::IsPointInside( const Vector2& point ) const
{
	return (IsPointInside( point.x, point.y ));
}


Disc2 Disc2::operator+( const Vector2& translation ) const
{
	Vector2 newCenter = center + translation;

	Disc2 newDisc2 = Disc2 (newCenter, radius);

	return newDisc2;
}


void Disc2::operator+=( const Vector2& translation )
{
	center += translation;
}


Disc2 Disc2::operator-( const Vector2& antiTranslation ) const
{
	Vector2 newCenter = center - antiTranslation;

	Disc2 newDisc2 = Disc2 (newCenter, radius);

	return newDisc2;
}


void Disc2::operator-=( const Vector2& antiTranslation )
{
	center -= antiTranslation;
}


const Disc2 Interpolate( const Disc2& start, const Disc2& end, float fractionTowardEnd )
{
	Vector2 center = Interpolate(start.center, end.center, fractionTowardEnd);
	float radius = Interpolate(start.radius, end.radius, fractionTowardEnd);

	return Disc2(center, radius);
}