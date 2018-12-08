#include "Engine/Core/Interval.hpp"
#include "Engine/Math/MathUtils.hpp"

/*
 * Tell if p1 is smaller than p2 ALONG the direction of this interval
 */
bool DirectionalInterval::IsPointSmaller(const Vector3& p1, const Vector3& p2) const
{
	Vector3 dir = m_max - m_min;
	
	Vector3 disp = p1 - p2;

	return (DotProduct(disp, dir) < 0.f);
}
