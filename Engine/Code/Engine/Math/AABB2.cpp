#include "Engine/Math/AABB2.hpp"

const AABB2 AABB2::ZERO_TO_ONE = AABB2(0.f, 0.f, 1.f, 1.f);

AABB2::AABB2(const AABB2& copy)
{
	mins = copy.mins;
	maxs = copy.maxs;
}


AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: mins(Vector2(minX, minY))
	, maxs(Vector2(maxX, maxY))
{

}


AABB2::AABB2(const Vector2& mins, const Vector2& maxs)
	: mins(mins)
	, maxs(maxs)
{

}


AABB2::AABB2(const Vector2& center, float radiusX, float radiusY)
	: mins(Vector2(center.x - radiusX, center.y - radiusY))
	, maxs(Vector2(center.x + radiusX, center.y + radiusY))
{

}


void AABB2::StretchToIncludePoint(float x, float y)
{
	if (x < mins.x)
	{
		mins.x = x;
	}
	else if (x > maxs.x)
	{
		maxs.x = x;
	}

	if (y < mins.y)
	{
		mins.y = y;
	}
	else if (y > maxs.y)
	{
		maxs.y = y;
	}
}

void AABB2::StretchToIncludePoint( const Vector2& point )
{
	StretchToIncludePoint(point.x, point.y);
}


void AABB2::AddPaddingToSides( float xPaddingRadius, float yPaddingRadius )
{
	mins.x -= xPaddingRadius;
	maxs.x += xPaddingRadius;

	mins.y -= yPaddingRadius;
	maxs.y += yPaddingRadius;
}


void AABB2::Translate( const Vector2& translation )
{
	mins += translation;
	maxs += translation;
}


void AABB2::Translate( float translationX, float translationY )
{
	Translate(Vector2(translationX, translationY));
}


bool AABB2::IsPointInside( const Vector2& point ) const
{
	return !(point.x > maxs.x || point.x < mins.x || point.y > maxs.y || point.y < mins.y);
}


bool AABB2::IsPointInside( float x, float y ) const
{
	return IsPointInside(Vector2(x, y));
}


Vector2 AABB2::GetDimensions() const
{
	return Vector2(maxs.x - mins.x, maxs.y - mins.y);
}


Vector2 AABB2::GetCenter() const
{
	return ( mins + ( GetDimensions() / 2.f ) );
}


std::vector<Vector2> AABB2::GetCornersFromBLInCounterClockwise() const
{
	std::vector<Vector2> res;

	res.push_back(mins);
	res.push_back(mins + Vector2(GetDimensions().x, 0.f));
	res.push_back(maxs);
	res.push_back(maxs - Vector2(GetDimensions().x, 0.f));

	return res;
}


Vector2 AABB2::GetClosestOnBoundPoint(Vector2 point) const
{
	float minDist = abs(point.x - mins.x);
	Vector2 boundsPoint = Vector2(mins.x, point.y);
	if (abs(maxs.x - point.x) < minDist)
	{
		minDist = abs(maxs.x - point.x);
		boundsPoint = Vector2(maxs.x, point.y);
	}
	if (abs(maxs.y - point.y) < minDist)
	{
		minDist = abs(maxs.y - point.y);
		boundsPoint = Vector2(point.x, maxs.y);
	}
	if (abs(mins.y - point.y) < minDist)
	{
		minDist = abs(mins.y - point.y);
		boundsPoint = Vector2(point.x, mins.y);
	}
	return boundsPoint;
}

float AABB2::GetRayIntersection(Vector2 origin, Vector2 direction)
{
	Vector2 end = origin + direction;

	float minT = GetRayIntersection(origin, end, Vector2(mins.x, mins.y), Vector2(mins.x, maxs.y));
	float x;
	x = GetRayIntersection(origin, end, Vector2(mins.x, maxs.y), Vector2(maxs.x, maxs.y));
	if (x < minT)
	{
		minT = x;
	}
	x = GetRayIntersection(origin, end, Vector2(maxs.x, maxs.y), Vector2(maxs.x, mins.y));
	if (x < minT)
	{
		minT = x;
	}
	x = GetRayIntersection(origin, end, Vector2(maxs.x, mins.y), Vector2(mins.x, mins.y));
	if (x < minT)
	{
		minT = x;
	}

	return minT;
}

float AABB2::GetRayIntersection(Vector2 originA, Vector2 endA, Vector2 originB, Vector2 endB)
{
	Vector2 r = endA - originA;
	Vector2 s = endB - originB;

	float n = CrossProduct2(originB - originA, r);
	float d = CrossProduct2(r, s);

	if (n == 0.f && d == 0.f)
	{
		return INFINITY;
	}
	if (d == 0.f)
	{
		return INFINITY;
	}

	float u = n / d;
	float t = CrossProduct2(originB - originA, s) / d;
	if ((t >= 0.f) && (t <= 1.f) && (u >= 0.f) && (u <= 1.f))
	{
		return t;
	}
	return INFINITY;
}

void AABB2::operator+=(const Vector2& translation)
{
	mins += translation;
	maxs += translation;
}


void AABB2::operator-=(const Vector2& antiTranslation)
{
	mins -= antiTranslation;
	maxs -= antiTranslation;
}


AABB2 AABB2::operator+(const Vector2& translation) const
{
	Vector2 newMins = mins + translation;
	Vector2 newMaxs = maxs + translation;

	return AABB2(newMins, newMaxs);
}


AABB2 AABB2::operator-(const Vector2& antiTranslation) const
{
	Vector2 newMins = mins - antiTranslation;
	Vector2 newMaxs = maxs - antiTranslation;

	return AABB2(newMins, newMaxs);
}


const AABB2 Interpolate( const AABB2& start, const AABB2& end, float fractionTowardEnd )
{
	Vector2 min = Interpolate(start.mins, end.mins, fractionTowardEnd);
	Vector2 max = Interpolate(start.maxs, end.maxs, fractionTowardEnd);

	return AABB2(min, max);
}

void AABB2::SetFromText( std::string text )
{
	// min x
	mins.x = static_cast<float>(atof(text.c_str()));
	text = text.substr(text.find(',') + 1);

	// min y
	mins.y = static_cast<float>(atof(text.c_str()));
	text = text.substr(text.find(',') + 1);

	// max x
	maxs.x = static_cast<float>(atof(text.c_str()));
	text = text.substr(text.find(',') + 1);

	// max y
	maxs.y = static_cast<float>(atof(text.c_str()));
}