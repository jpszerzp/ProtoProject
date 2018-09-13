#pragma once

#include "Engine/Math/Vector2.hpp"

class IntVector2
{
public:
	int x;
	int y;

	IntVector2(int xInput, int yInput);
	IntVector2();
	~IntVector2();

	const IntVector2 operator-( const IntVector2& intVecToSubtract ) const;
	bool operator==( const IntVector2& compare ) const;
	bool operator!=( const IntVector2& compare ) const;

	//const IntVector2 Interpolate( const IntVector2& start, const IntVector2& end, float fractionTowardEnd );

	void SetFromText( const char* text );
	Vector2 ToVector2();

	static const IntVector2 ZERO;
};

const IntVector2 Interpolate( const IntVector2& start, const IntVector2& end, float fractionTowardEnd );

class UInt64Vector2
{
public:
	uint64_t ux;
	uint64_t uy;

	UInt64Vector2(uint64_t x, uint64_t y)
		: ux(x), uy(y) {}
	UInt64Vector2(){}
	~UInt64Vector2(){}
};