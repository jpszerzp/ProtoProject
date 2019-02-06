#pragma once

#include<vector>

#include "Engine/Math/Plane.hpp"
#include "Engine/Core/EngineCommon.hpp"

class ConvexHull
{
private:
	std::vector<Plane> m_planes;

public:
	ConvexHull(const std::vector<Plane>& planes);
	~ConvexHull(){}

	Plane GetPlane(uint idx);
	uint GetPlaneNum();
};