#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"

FloatRange::FloatRange()
{

}

FloatRange::FloatRange(float initMin, float initMax)
{
	min = initMin;
	max = initMax;
}

FloatRange::FloatRange(float initMinMax)
{
	min = initMinMax;
	max = initMinMax;
}

const FloatRange Interpolate( const FloatRange& start, const FloatRange& end, float fractionTowardEnd )
{
	float min = Interpolate(start.min, end.min, fractionTowardEnd);
	float max = Interpolate(start.max, end.max, fractionTowardEnd);

	return FloatRange(min, max);
}

float FloatRange::GetRandomInRange() const
{
	return GetRandomFloatInRange(min, max);
}

bool DoRangesOverlap( const FloatRange& a, const FloatRange& b )
{
	if (a.max <= b.min)
	{
		return true;
	}

	return false;
}

void FloatRange::SetFromText( const char* text )
{
	const char* tilde = strchr(text, '~');

	if (tilde != nullptr)
	{
		min = static_cast<float>(atof(text));
		max = static_cast<float>(atof(tilde + 1));
	}
	else
	{
		min = static_cast<float>(atof(text));
		max = static_cast<float>(atof(text));
	}
}

bool FloatRange::operator==( const FloatRange& compare ) const
{
	if ((min == compare.min) && (max == compare.max))
	{
		return true;
	}

	return false;
}
