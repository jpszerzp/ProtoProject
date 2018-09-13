#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"

const IntVector2 IntVector2::ZERO = IntVector2(0, 0);

IntVector2::IntVector2(int xInput, int yInput)
{
	x = xInput;
	y = yInput;
}


IntVector2::IntVector2()
{

}


IntVector2::~IntVector2()
{

}


const IntVector2 IntVector2::operator-( const IntVector2& intVecToSubtract ) const
{
	return IntVector2(x - intVecToSubtract.x, y - intVecToSubtract.y);
}


bool IntVector2::operator==( const IntVector2& compare ) const
{
	if ((x == compare.x) && (y == compare.y))
	{
		return true;
	}

	return false;
}


bool IntVector2::operator!=( const IntVector2& compare ) const
{
	return !((*this) == compare);
}


const IntVector2 Interpolate( const IntVector2& start, const IntVector2& end, float fractionTowardEnd )
{
	int interpX = Interpolate(start.x, end.x, fractionTowardEnd);
	int interpY = Interpolate(start.y, end.y, fractionTowardEnd);

	return IntVector2(interpX, interpY);
}


void IntVector2::SetFromText( const char* text )
{
	const char* comma = strchr(text, ',');

	x = atoi(text);
	y = atoi(comma + 1);
}


Vector2 IntVector2::ToVector2()
{
	return Vector2(static_cast<float>(x), static_cast<float>(y));
}