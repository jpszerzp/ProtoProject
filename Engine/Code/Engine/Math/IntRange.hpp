#pragma once

class IntRange
{
	//const IntRange Interpolate( const IntRange& start, const IntRange& end, float fractionTowardEnd );

public:
	int min;
	int max;

	IntRange();
	IntRange(int initMin, int initMax);
	IntRange(int initMinMax);

	bool operator==( const IntRange& compare ) const;

	int GetRandomInRange() const;

	void SetFromText( const char* text );
};

const IntRange Interpolate( const IntRange& start, const IntRange& end, float fractionTowardEnd );
bool DoRangesOverlap( const IntRange& a, const IntRange& b );