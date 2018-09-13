#pragma once

#include "Engine/Math/IntVector2.hpp"

#include <vector>

class HeatMap
{
private:
	std::vector<float> m_heatPerGridCell;
	IntVector2 m_dimension;

public:
	HeatMap(int width, int height);
	~HeatMap();

	float GetHeat(size_t index) const { return m_heatPerGridCell[index]; }
	void SetHeat(size_t index, float heatVal) { m_heatPerGridCell[index] = heatVal; }

	void SetDefaultMap();
};