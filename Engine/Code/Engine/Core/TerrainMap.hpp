#pragma once

#include "Engine/Core/MapChunk.hpp"
#include "Engine/Math/AABB3.hpp"

#include <vector>

#define RAYMARCH_EPSILON 0.001f

class TerrainMap
{
public:
	TerrainMap();
	~TerrainMap();

	void LoadFromImage(const Image& image, const AABB2& extents, int meshNumX, int meshNumY,
		float minHeight, float maxHeight, IntVector2 chunkCoords);
	void FreeAllChunks();

	int			GetTriangleIdxOfChunk(Vector2 xz);
	int			GetChunkIdxFromCoord(IntVector2 chunkCoord);
	float		GetTerrainHeightFromMappos(Vector2 xz);
	float		GetTerrainHeightFromUV(IntVector2 uv);
	AABB2		GetChunkExtents(IntVector2 chunkCoord);
	AABB2		GetMapExtents();
	IntVector2  GetChunkCoord(Vector2 xz);
	Vector3		GetTriangleNormal(Vector2 xz);
	Vector3		GetTriangleCenter(Vector2 xz);
	MapChunk*   GetChunk(IntVector2 chunkCoord);

	int		Raymarch(std::vector<RaycastHit3>& hits,
		RaycastHit3& usedHit, int caredHit, Ray3& ray, bool debug);
	bool	IsBelowTerrain(Vector3 point);
	bool	IsWithinExtents(Vector2 xz);
	float	GetAbsVerticalDistanceToTerrain(Vector3 point);
	float   GetVerticalDistanceToTerrain(Vector3 point);

public:
	AABB2				   m_mapExtents; 
	AABB3				   m_mapBound;
	float				   m_mapMinHeight; 
	float				   m_mapMaxHeight; 
	int					   m_meshNumX;
	int					   m_meshNumY;

	std::vector<MapChunk*> m_chunks;
	IntVector2			   m_chunkCoords;

	Image				   m_image; 
};