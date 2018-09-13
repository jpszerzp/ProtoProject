#include "Engine/Core/HeatMap.hpp"


HeatMap::HeatMap(int width, int height)
{
	m_dimension = IntVector2(width, height);

	// initialize heat value column major
	for (int widthIndex = 0; widthIndex < m_dimension.x; ++widthIndex)
	{
		for (int heightIndex = 0; heightIndex < m_dimension.y; ++heightIndex)
		{
			m_heatPerGridCell.push_back(9999.f);
		}
	}
}


HeatMap::~HeatMap()
{

}


void HeatMap::SetDefaultMap()
{
	for (int widthIndex = 0; widthIndex < m_dimension.x; ++widthIndex)
	{
		for (int heightIndex = 0; heightIndex < m_dimension.y; ++heightIndex)
		{
			int index = widthIndex * m_dimension.y + heightIndex;
			m_heatPerGridCell[index] = 9999.f;
		}
	}
}