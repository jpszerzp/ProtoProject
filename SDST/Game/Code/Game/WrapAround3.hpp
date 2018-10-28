#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/GameObject.hpp"

#include <vector>

class WrapAround
{
public:
	AABB3 m_bounds;
	std::vector<GameObject*> m_gos;
	
public:
	WrapAround(Vector3 min, Vector3 max);
	~WrapAround();

	void Update();
};