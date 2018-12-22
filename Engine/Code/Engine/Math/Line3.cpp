#include "Engine/Math/Line3.hpp"
#include "Engine/Math/MathUtils.hpp"

/*
 * Computes closest points among two line segments, seg1 and seg2 are the segments,
 * t1 and t2 are extension param, whereas p1 and p2 are the closest points themselves
 * return value is square of the closest distance
 */
float LineSegment3::ClosestPointsSegments(const LineSegment3& seg1,
	const LineSegment3& seg2, float& t1, float& t2, Vector3& p1, Vector3& p2)
{
	// get extent vector
	Vector3 ext1 = seg1.extent;
	Vector3 ext2 = seg2.extent;
	
	// get parameters for gaussian elimination
	Vector3 r = seg1.start - seg2.start;
	float a = DotProduct(ext1, ext1);
	float e = DotProduct(ext2, ext2);
	float f = DotProduct(ext2, r);

	// if both segments degenerate to points
	if (a <= EPSILON && e <= EPSILON)
	{
		t1 = t2 = 0.f;
		p1 = seg1.start;
		p2 = seg2.start;
		return DotProduct(p1 - p2, p1 - p2);	// (squared) distance of segments becomes squared dist of points
	}
	if (a <= EPSILON)
	{
		// seg1 degenerates
		t1 = 0.f;
		t2 = f / e;
		t2 = ClampFloat(t2, 0.f, 1.f);
	}
	else
	{
		float c = DotProduct(ext1, r);
		if (e <= EPSILON)
		{
			// seg2 degenerates
			t2 = 0.f;
			t1 = ClampFloat(-c / a, 0.f, 1.f);
		}
		else
		{
			// neither seg degenerates
			float b = DotProduct(ext1, ext2);
			float denominator = a * e - b * b;

			if (denominator != 0.f)
				t1 = ClampFloat((b * f - c * e) / denominator, 0.f, 1.f);
			else
				t1 = 0.f;

			float nominator = b * t1 + f;
			if (nominator < 0.f)		// < 0, out of range
			{
				t2 = 0.f;									// clamp it
				t1 = ClampFloat(-c / a, 0.f, 1.f);			// and update seg1 parameter
			}
			else if (nominator > e)		// > 1, out of range 
			{
				t2 = 1.f;									// clamp it
				t1 = ClampFloat((b - c) / a, 0.f, 1.f);		// and update seg1 parameter
			}
			else
				t2 = nominator / e;
		}
	}

	p1 = seg1.start + ext1 * t1;
	p2 = seg2.start + ext2 * t2;
	return DotProduct(p1 - p2, p1 - p2);			// return length squared
}

/*
 * Need to consider special case where two segments are parallel
 */
/*
float LineSegment3::ClosestPointsSegmentsConstrained(const LineSegment3& seg1, const LineSegment3& seg2, Vector3& v1, Vector3& v2)
{
	// see p147 of RTC
	Vector3 dir1 = seg1.extent.GetNormalized();
	Vector3 dir2 = seg2.extent.GetNormalized();

	Vector3 r = seg1.start - seg2.start;

	float a = DotProduct(dir1, dir1);
	float b = DotProduct(dir1, dir2);
	float c = DotProduct(dir1, r);
	float e = DotProduct(dir2, dir2);
	float f = DotProduct(dir2, r);
	float d = a * e - b * b;

	if (d != 0.f)
	{
		float s = (b * f - c * e) / d;
		float t = (a * f - b * c) / d;

		v1 = seg1.start + dir1 * s;
		v2 = seg2.start + dir2 * t;
	}
	else
	{
		// two lines are parallel, pick a random one on line 1, then project that point to line 2
		// in this case we pick the middle piont for line 1
		Vector3 mid_projected;
		Vector3 mid = seg1.start + seg1.extent / 2.f;
		LineSegment3::ClosestPointPtSegment(mid, seg2, mid_projected);

		v1 = mid;
		v2 = mid_projected;
	}

	return (v1 - v2).GetLength();
}
*/

float LineSegment3::ClosestPointPtSegment(const Vector3& pt, const LineSegment3& seg, Vector3& projected)
{
	float t = DotProduct(pt - seg.start, seg.extent);
	if (t <= 0.f)
	{
		t = 0.f;
		projected = seg.start;
	}
	else
	{
		float denom = DotProduct(seg.extent, seg.extent);
		if (t >= denom)
		{
			t = 1.f;
			projected = seg.start + seg.extent;
		}
		else
		{
			t = t / denom;
			projected = seg.start + seg.extent * t;
		}
	}

	return t;
}

Line3 Line3::FromVector3(const Vector3& dir)
{
	Line3 aixs;

	aixs.one = Vector3::ZERO;
	aixs.the_other = aixs.one + dir;

	return aixs;
}
