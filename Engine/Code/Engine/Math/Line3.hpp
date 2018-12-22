#pragma once

#include "Engine/Math/Vector3.hpp"

class LineSegment3
{
public:
	Vector3 start;
	Vector3 extent;		// end = start + extent
	//float t;			// [0, 1]

public:
	LineSegment3(Vector3 start, Vector3 end)
		: start(start), extent(end - start){}
	~LineSegment3(){}

	constexpr static float EPSILON = 0.01f;

	static float ClosestPointsSegments(const LineSegment3& seg1, const LineSegment3& seg2, float& t1, float& t2, Vector3& p1, Vector3& p2);
	//static float ClosestPointsSegmentsConstrained(const LineSegment3& seg1, const LineSegment3& seg2, Vector3& v1, Vector3& v2);
	static float ClosestPointPtSegment(const Vector3& pt, const LineSegment3& seg, Vector3& projected);
};

class Line3
{
public:
	Vector3 one;
	Vector3 the_other;

	Line3(){}
	Line3(const Vector3& end1, const Vector3& end2)
		: one(end1), the_other(end2) {}

	static Line3 FromVector3(const Vector3& dir);
};