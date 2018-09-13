#pragma once

#include "Engine/Renderer/Renderable.hpp"

class TerrainMap;

class MapChunk
{
public:
	MapChunk();
	~MapChunk();

	void SetupChunkInMap(TerrainMap* map, IntVector2 chunkCoord);
	void SetupChunkRenderable(Vector2 bl, Vector2 tr, int meshNumX, int meshNumY);
	void Cleanup();

	float GetHeightFromPos(float posX, float posY);

public:
	TerrainMap*		m_terrainMap;
	Renderable*		m_renderable;
	IntVector2		m_chunkCoord;

	int m_meshPointNumX;
	int m_meshPointNumY;

	// per vert
	std::vector<float>   m_vertHeights;
	std::vector<Vector3> m_verts;
	
	// per triangles
	std::vector<Vector3> m_triangleCenterNormals;
	std::vector<Vector3> m_triangleCenters;

	// per triangle verts
	std::vector<Vector3> m_triangleVerts;
	std::vector<Vector2> m_triangleUVs;
	std::vector<Vector3> m_triangleNormals;
};