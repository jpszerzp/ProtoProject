#pragma once

class FloatRange
{
	//const FloatRange Interpolate( const FloatRange& start, const FloatRange& end, float fractionTowardEnd );

public:
	float min;
	float max;

	FloatRange();
	FloatRange(float initMin, float initMax);
	FloatRange(float initMinMax);

	bool operator==( const FloatRange& compare ) const;

	float GetRandomInRange() const;

	void SetFromText( const char* text );

};

bool DoRangesOverlap( const FloatRange& a, const FloatRange& b );
const FloatRange Interpolate( const FloatRange& start, const FloatRange& end, float fractionTowardEnd );