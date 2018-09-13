#pragma once

#include <vector>
#include "Engine/Core/Image.hpp"

class SurfacePatch
{
public:
	std::vector<float>   m_heights;
	std::vector<Vector3> m_sourceVerts;
	std::vector<Vector3> m_verts;
	std::vector<Vector2> m_uvs;
	std::vector<Vector3> m_normals;
	Image*				 m_heightMap;
	Vector2			     m_bl;
	Vector2			     m_tr;
	int				     m_meshNumX;
	int				     m_meshNumY;
	float			     m_bottomHeight;
	float			     m_maxHeight;

public:
	SurfacePatch(Image* heightmap, Vector2 bl, Vector2 tr, int meshNumX, int meshNumY, float bottomHeight, float maxHeight);
	~SurfacePatch();

	float GetHeightFromMapCoord(float mapPosX, float mapPosY);
	float GetHeightFromUV(IntVector2 uv);
};