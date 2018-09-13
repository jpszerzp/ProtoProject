#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"

IntRange::IntRange()
{

}

IntRange::IntRange(int initMin, int initMax)
{
	min = initMin;
	max = initMax;
}

IntRange::IntRange(int initMinMax)
{
	min = initMinMax;
	max = initMinMax;
};

const IntRange Interpolate( const IntRange& start, const IntRange& end, float fractionTowardEnd )
{
	int min = Interpolate(start.min, end.min, fractionTowardEnd);
	int max = Interpolate(start.max, end.max, fractionTowardEnd);

	return IntRange(min, max);
}

int IntRange::GetRandomInRange() const
{
	return GetRandomIntInRange(min, max);
}

bool DoRangesOverlap( const IntRange& a, const IntRange& b )
{
	if (a.max <= b.min)
	{
		return true;
	}

	return false;
}

void IntRange::SetFromText( const char* text )
{
	const char* tilde = strchr(text, '~');

	if (tilde != nullptr)
	{
		min = atoi(text);
		max = atoi(tilde + 1);
	}
	else
	{
		min = atoi(text);
		max = atoi(text);
	}
}

bool IntRange::operator==( const IntRange& compare ) const
{
	if ((min == compare.min) && (max == compare.max))
	{
		return true;
	}

	return false;
}