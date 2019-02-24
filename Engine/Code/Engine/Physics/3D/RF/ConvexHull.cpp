#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

ConvexHull::ConvexHull(const std::vector<Plane>& planes)
	: m_planes(planes)
{

}

Plane ConvexHull::GetPlane(uint idx) const
{
	ASSERT_OR_DIE(idx < m_planes.size(), "Plane idx out of range");

	return m_planes[idx];
}

uint ConvexHull::GetPlaneNum() const
{
	return (uint)m_planes.size();
}

