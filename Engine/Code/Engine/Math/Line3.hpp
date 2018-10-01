#pragma once

#include "Engine/Math/Vector3.hpp"

class LineSegment3
{
public:
	Vector3 start;
	Vector3 extent;		// end = start + extent
	float t;			// [0, 1]

public:
	LineSegment3(Vector3 start, Vector3 end)
		: start(start), extent(end - start){}
	~LineSegment3(){}

	constexpr static float EPSILON = 0.01f;

	static float ClosestPointsSegments(const LineSegment3& seg1,
		const LineSegment3& seg2, float& t1, float& t2, Vector3& p1, Vector3& p2);
};