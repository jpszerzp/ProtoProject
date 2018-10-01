#include "Engine/Math/Line3.hpp"
#include "Engine/Math/MathUtils.hpp"

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

