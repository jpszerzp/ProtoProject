#include "Engine/Core/TerrainMap.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

TerrainMap::TerrainMap()
{

}


TerrainMap::~TerrainMap()
{
	FreeAllChunks();
}


void TerrainMap::LoadFromImage(const Image& image, const AABB2& extents, int meshNumX, int meshNumY,
	float minHeight, float maxHeight, IntVector2 chunkCoords)
{
	m_meshNumX = meshNumX;
	m_meshNumY = meshNumY;
	m_image = image;
	m_mapExtents = extents;
	m_mapMinHeight = minHeight;
	m_mapMaxHeight = maxHeight;
	m_chunkCoords = chunkCoords;

	int chunkXNum = m_chunkCoords.x + 1;
	int chunkYNum = m_chunkCoords.y + 1;

	float chunkWidth = extents.GetDimensions().x / chunkXNum;
	float chunkHeight = extents.GetDimensions().y / chunkYNum;

	int meshNumAverageXEndpointsExclusive = (int)(floor((meshNumX - (chunkXNum + 1)) / chunkXNum));
	int meshNumXEndpointsInclusiveNonends = meshNumAverageXEndpointsExclusive + 2;
	int meshNumXEndpointsInclusiveEnds = meshNumX - (meshNumAverageXEndpointsExclusive * (chunkXNum - 1) + (chunkXNum + 1) - 1) + 1;

	int meshNumAverageYEndpointsExclusive = (int)(floor((meshNumY - (chunkYNum + 1)) / chunkYNum));
	int meshNumYEndpointsInclusiveNonends = meshNumAverageYEndpointsExclusive + 2;
	int meshNumYEndpointsInclusiveEnds = meshNumY - (meshNumAverageYEndpointsExclusive * (chunkYNum - 1) + (chunkYNum + 1) - 1) + 1;

	for (int yIdx = 0; yIdx < chunkYNum; ++yIdx)
	{
		for (int xIdx = 0; xIdx < chunkXNum; ++xIdx)
		{
			// initialize chunks with this coord
			IntVector2 chunkCoord = IntVector2(xIdx, yIdx);

			Vector2 mapBL = extents.mins;

			float chunkBLX = mapBL.x + xIdx * chunkWidth;
			float chunkBLY = mapBL.y + yIdx * chunkHeight;
			Vector2 chunkBL = Vector2(chunkBLX, chunkBLY);

			float chunkTRX = chunkBLX + chunkWidth;
			float chunkTRY = chunkBLY + chunkHeight;
			Vector2 chunkTR = Vector2(chunkTRX, chunkTRY);

			int meshNum_X = (xIdx == (chunkXNum - 1)) ? meshNumXEndpointsInclusiveEnds : meshNumXEndpointsInclusiveNonends;
			int meshNum_Y = (yIdx == (chunkYNum - 1)) ? meshNumYEndpointsInclusiveEnds : meshNumYEndpointsInclusiveNonends;

			MapChunk* chunk = new MapChunk();
			chunk->SetupChunkInMap(this, chunkCoord);
			chunk->SetupChunkRenderable(chunkBL, chunkTR, meshNum_X, meshNum_Y);

			m_chunks.push_back(chunk);
		}
	}

	// all chunks set up, find map bound
	float mapRealMaxHeight = -INFINITY;
	float mapRealMinHeight = INFINITY;
	for each (MapChunk* chunk in m_chunks)
	{
		for each (Vector3 vert in chunk->m_verts)
		{
			if (vert.y > mapRealMaxHeight)
			{
				mapRealMaxHeight = vert.y;
			}

			if (vert.y < mapRealMinHeight)
			{
				mapRealMinHeight = vert.y;
			}
		}
	}
	Vector2 mapBoundMinXZ = extents.mins;
	Vector2 mapBoundMaxXZ = extents.maxs;
	Vector3 mapBoundMin = Vector3(mapBoundMinXZ.x, mapRealMinHeight, mapBoundMinXZ.y);
	Vector3 mapBoundMax = Vector3(mapBoundMaxXZ.x, mapRealMaxHeight, mapBoundMaxXZ.y);
	AABB3 mapBound = AABB3(mapBoundMin, mapBoundMax);
	m_mapBound = mapBound;
}


void TerrainMap::FreeAllChunks()
{
	for each (MapChunk* chunk in m_chunks)
	{
		delete chunk;
		chunk = nullptr;
	}
}


// transition method to compute height
float TerrainMap::GetTerrainHeightFromMappos(Vector2 xz)
{
	float mapPosX = xz.x;
	float mapPosY = xz.y;

	Vector2 mapBL = m_mapExtents.mins;
	Vector2 mapTR = m_mapExtents.maxs;

	IntVector2 heightmapDim = m_image.GetImageDimension();

	float cellU = RangeMapFloat(mapPosX, mapBL.x, mapTR.x, 0.f, (float)(heightmapDim.x - 1));
	float cellV = RangeMapFloat(mapPosY, mapBL.y, mapTR.y, (float)(heightmapDim.y - 1), 0.f);

	int cellULeft = (int)floor(cellU);
	int cellURight = (int)ceil(cellU);

	int cellVLeft = (int)floor(cellV);
	int cellVRight = (int)ceil(cellV);

	IntVector2 uvBL = IntVector2(cellULeft, cellVRight);	
	IntVector2 uvBR = IntVector2(cellURight, cellVRight);  
	IntVector2 uvTL = IntVector2(cellULeft, cellVLeft);    
	IntVector2 uvTR = IntVector2(cellURight, cellVLeft);	

	float heightUVBL = GetTerrainHeightFromUV(uvBL);		
	float heightUVBR = GetTerrainHeightFromUV(uvBR);		
	float heightUVTL = GetTerrainHeightFromUV(uvTL);		
	float heightUVTR = GetTerrainHeightFromUV(uvTR);		

	float heightBLToBR = RangeMapFloat(cellU, (float)cellULeft, (float)cellURight, heightUVBL, heightUVBR);
	float heightTLToTR = RangeMapFloat(cellU, (float)cellULeft, (float)cellURight, heightUVTL, heightUVTR);
	float height = RangeMapFloat(cellV, (float)cellVRight, (float)cellVLeft, heightBLToBR, heightTLToTR);

	return height;
}


AABB2 TerrainMap::GetChunkExtents(IntVector2 chunkCoord)
{
	int coordX = chunkCoord.x;
	int coordY = chunkCoord.y;

	int numChunkX = m_chunkCoords.x + 1;
	int numChunkY = m_chunkCoords.y + 1;

	float mapHorizontalLength = m_mapExtents.GetDimensions().x;
	float mapVerticalLength = m_mapExtents.GetDimensions().y;

	float chunkWidth = mapHorizontalLength / (float)numChunkX;
	float chunkHeight = mapVerticalLength / (float)numChunkY;

	float intervalX = (float)coordX * chunkWidth;
	float intervalY = (float)coordY * chunkHeight;

	Vector2 chunkMin = m_mapExtents.mins + Vector2(intervalX, intervalY);
	Vector2 chunkMax = chunkMin + Vector2(chunkWidth, chunkHeight);

	AABB2 chunkExtents = AABB2(chunkMin, chunkMax);

	return chunkExtents;
}


AABB2 TerrainMap::GetMapExtents()
{
	return m_mapExtents;
}

MapChunk* TerrainMap::GetChunk(IntVector2 chunkCoord)
{
	int idx = GetChunkIdxFromCoord(chunkCoord);

	MapChunk* chunk = m_chunks[idx];

	return chunk;
}


float TerrainMap::GetTerrainHeightFromUV(IntVector2 uv)
{
	Rgba texel = m_image.GetTexel(uv.x, uv.y);

	float heightRaw = static_cast<float>(texel.r);

	float height = RangeMapFloat(heightRaw, 0.f, 255.f, m_mapMinHeight, m_mapMaxHeight);

	return height;
}

/*
float TerrainMap::GetTerrainMaxHeight()
{
	float maxHeight = -INFINITY;

	for each (MapChunk* chunk in m_chunks)
	{
		for each (Vector3 vert in chunk->m_verts)
		{
			if (vert.y > maxHeight)
			{
				maxHeight = vert.y;
			}
		}
	}

	return maxHeight;
}
*/

IntVector2 TerrainMap::GetChunkCoord(Vector2 xz)
{
	float mapBLX = m_mapExtents.mins.x;
	float mapBLY = m_mapExtents.mins.y;

	float xInterval = xz.x - mapBLX;
	float yInterval = xz.y - mapBLY;

	int chunkNumX = m_chunkCoords.x + 1;
	int chunkNumY = m_chunkCoords.y + 1;

	float chunkWidth = m_mapExtents.GetDimensions().x / (float)chunkNumX;
	float chunkHeight = m_mapExtents.GetDimensions().y / (float)chunkNumY;

	float numChunkXf = xInterval / chunkWidth;
	float numChunkYf = yInterval / chunkHeight;

	float float_pointer_for_integral_part_x;
	float float_pointer_for_integral_part_y;

	modf(numChunkXf, &float_pointer_for_integral_part_x);
	modf(numChunkYf, &float_pointer_for_integral_part_y);

	int integralX = (int)(float_pointer_for_integral_part_x);
	int integralY = (int)(float_pointer_for_integral_part_y);

	IntVector2 chunkCoord = IntVector2(integralX, integralY);

	return chunkCoord;
}


int TerrainMap::GetTriangleIdxOfChunk(Vector2 xz)
{
	IntVector2 chunkCoord = GetChunkCoord(xz);
	AABB2 chunkExtents = GetChunkExtents(chunkCoord);

	float intervalXWithinChunk = xz.x - chunkExtents.mins.x;
	float intervalYWithinChunk = xz.y - chunkExtents.mins.y;

	int chunkIdx = GetChunkIdxFromCoord(chunkCoord);
	MapChunk* chunk = m_chunks[chunkIdx];

	int meshBlockNumX = chunk->m_meshPointNumX - 1;
	int meshBlockNumY = chunk->m_meshPointNumY - 1;

	float blockWidth = chunkExtents.GetDimensions().x / (float)meshBlockNumX;
	float blockHeight = chunkExtents.GetDimensions().y / (float)meshBlockNumY;

	float passedMeshBlockNumXf = intervalXWithinChunk / blockWidth;
	float passedMeshBlockNumYf = intervalYWithinChunk / blockHeight;

	float fp_for_int_mb_coord_x;
	float fp_for_int_mb_coord_y;

	float frac_mb_x = modf(passedMeshBlockNumXf, &fp_for_int_mb_coord_x);
	float frac_mb_y = modf(passedMeshBlockNumYf, &fp_for_int_mb_coord_y);

	int int_coord_x = (int)(fp_for_int_mb_coord_x);
	int int_coord_y = (int)(fp_for_int_mb_coord_y);

	int numTriangleMeshBlockX = 2 * meshBlockNumX;

	int numPassedTriangle = int_coord_y * numTriangleMeshBlockX + int_coord_x * 2;

	int idxLastPassedTriangle = numPassedTriangle - 1;

	int idxBaseTriangle = idxLastPassedTriangle + 1;

	int resIdx = -1;

	float distanceToLeftEdge = blockWidth * frac_mb_x;
	float distanceToRightEdge = blockWidth - distanceToLeftEdge;
	float distanceToBottomEdge = blockHeight * frac_mb_y;

	float standardDistanceToBottomEdge = (blockHeight / blockWidth) * distanceToRightEdge;

	if (distanceToBottomEdge >= standardDistanceToBottomEdge)
	{
		resIdx = idxBaseTriangle + 1;
	}
	else
	{
		resIdx = idxBaseTriangle;
	}

	return resIdx;
}


int TerrainMap::GetChunkIdxFromCoord(IntVector2 chunkCoord)
{
	int y = chunkCoord.y;
	int x = chunkCoord.x;

	int chunkNumX = m_chunkCoords.x + 1;

	int idx = y * chunkNumX + (x + 1) - 1;

	return idx;
}


Vector3 TerrainMap::GetTriangleNormal(Vector2 xz)
{
	IntVector2 chunkCoord = GetChunkCoord(xz);
	
	MapChunk* chunk = GetChunk(chunkCoord);

	int triangleIdxOfChunk = GetTriangleIdxOfChunk(xz);

	Vector3 normal = chunk->m_triangleCenterNormals[triangleIdxOfChunk];

	return normal;
}


Vector3 TerrainMap::GetTriangleCenter(Vector2 xz)
{
	IntVector2 chunkCoord = GetChunkCoord(xz);

	MapChunk* chunk = GetChunk(chunkCoord);

	int triangleIdxOfChunk = GetTriangleIdxOfChunk(xz);

	Vector3 center = chunk->m_triangleCenters[triangleIdxOfChunk];

	return center;
}


// To debug, use limited frames and longer lifetime points rather than lots of frames with short lifetimes
int TerrainMap::Raymarch(std::vector<RaycastHit3>& hits,
	RaycastHit3& usedHit, int, Ray3& ray, bool debug)
{
	//Break this into a number of steps, using a fixed step size (we used half the distance between two vertices (.5f * m_cell_size.x).
	float mapWidth = m_mapExtents.GetDimensions().x;
	float mapHeight = m_mapExtents.GetDimensions().y;
	float meshPointInvervalX = mapWidth / (float)(m_meshNumX - 1);
	float meshPointInvervalY = mapHeight / (float)(m_meshNumY - 1);
	float stepDistance = min(meshPointInvervalY, meshPointInvervalX) / 2.f;

	int res = 0;

	//First, figure out the part of the ray we care about (ie, where does it intersect the bounds of the terrain;
	//If we don't hit the bounds, early out without a hit.
	//If we only have one hit, add the ray's origin as the first contact, and proceed with the algorithm. (we were inside the bounding box)
	std::vector<RaycastHit3> hitsWithMapBound;
	int hitNum = ray.IntersectAABB3(hitsWithMapBound, m_mapBound);

	Vector3 startOfIntersection;
	Vector3 endOfIntersection;

	if (hitNum == 0)
	{
		return res;
	}
	else if (hitNum == 1)
	{
		startOfIntersection = ray.m_start;
		endOfIntersection = hitsWithMapBound[0].m_hitLocation;
	}
	else if (hitNum == 2)
	{
		// guarantee that in the vector of raycast hit,
		// the hits are actually ordered from above to below
		startOfIntersection = hitsWithMapBound[0].m_hitLocation;
		endOfIntersection = hitsWithMapBound[1].m_hitLocation;
	}

	if (debug)
	{
		//DebugRenderPoint(100.f, 10.f, startOfIntersection, Rgba::MEGENTA, Rgba::MEGENTA, DEBUG_RENDER_USE_DEPTH);
		//DebugRenderPoint(100.f, 10.f, endOfIntersection, Rgba::YELLOW, Rgba::YELLOW, DEBUG_RENDER_USE_DEPTH);
	}

	//If our first point is below the terrain,
	//we throw out the ray (may revisit and have it step until we're above).
	if (IsBelowTerrain(startOfIntersection))
	{
		// FOR NOW just abandon directly
		return res;
	}

	//If our first point is above, we start the algorithm;
	else
	{
		// start actual stepping
		Vector3 abovePoint = startOfIntersection;
		Vector3 belowPoint;

		Vector3 stepDirection = ray.m_normalizedDirection;
		Vector3 stepPoint = abovePoint;

		while (!IsBelowTerrain(stepPoint))
		{
			//Keep track of where you're coming from (above_point)
			abovePoint = stepPoint;

			//Step along the ray until we detect we're below the terrain.
			Vector3 stepDisplacement = stepDirection * stepDistance;
			stepPoint = abovePoint + stepDisplacement;

			//If we reach the end of our ray and still don't have a point below the terrain,
			//we didn't hit it, so early out.
			if (ray.IsPt1ExceedingPt2AlongRay(stepPoint, endOfIntersection))
			{
				return res;
			}
		}

		//Once we get our first below point - we have a hit
		//close in on it (second phase of the algorithm)
		belowPoint = stepPoint;
		Vector3 pointOfInterest = GetMiddlePoint(abovePoint, belowPoint);

		//Keep having your search distance, moving your point to the middle.
		//If we're below, we bring our max in, otherwise bring our min in.
		while (GetAbsVerticalDistanceToTerrain(pointOfInterest) > RAYMARCH_EPSILON)
		{
			if (IsBelowTerrain(pointOfInterest))
			{ 
				belowPoint = pointOfInterest; 
			} 
			else 
			{ 
				abovePoint = pointOfInterest; 
			}

			pointOfInterest = GetMiddlePoint(abovePoint, belowPoint);
		}

		if (debug)
		{
			//DebugRenderPoint(100.f, 100.f, pointOfInterest, Rgba::DARK_BROWN, Rgba::DARK_BROWN, DEBUG_RENDER_USE_DEPTH);
		}

		//Do this until we're "close enough" to the terrain. That is our intersection point.
		res++;
		float t = ray.ReverseEvaluate(pointOfInterest);
		RaycastHit3 theHit = RaycastHit3(true, pointOfInterest, t);
		hits.push_back(theHit);

		// for now, record the first hit with terrain as used hit
		if (res == 1)
		{
			usedHit = theHit;
		}
	}

	// for now, this only returns the first hit of ray with terrain no matter what
	return res;
}


bool TerrainMap::IsBelowTerrain(Vector3 point)
{
	bool underTerrain;

	float rawDistance = GetVerticalDistanceToTerrain(point);

	if (rawDistance < 0.f)
	{
		underTerrain = true;
	}
	else
	{
		underTerrain = false;
	}

	return underTerrain;
}


bool TerrainMap::IsWithinExtents(Vector2 xz)
{
	bool within = m_mapExtents.IsPointInside(xz);
	return within;
}

float TerrainMap::GetAbsVerticalDistanceToTerrain(Vector3 point)
{
	float rawDistance = GetVerticalDistanceToTerrain(point);

	float absDistance = abs(rawDistance);

	return absDistance;
}

float TerrainMap::GetVerticalDistanceToTerrain(Vector3 point)
{
	Vector2 pointXZ = Vector2(point.x, point.z);

	float mapHeight = GetTerrainHeightFromMappos(pointXZ);
	float pointHeight = point.y;

	float distance = pointHeight - mapHeight;

	return distance;
}
