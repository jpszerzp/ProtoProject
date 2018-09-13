#include "Engine/Core/SurfacePatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


SurfacePatch::SurfacePatch(Image* heightmap, Vector2 bl, Vector2 tr, int meshNumX, int meshNumY, float bottomHeight, float maxHeight)
	: m_heightMap(heightmap), m_bl(bl), m_tr(tr), m_meshNumX(meshNumX), m_meshNumY(meshNumY), m_bottomHeight(bottomHeight), m_maxHeight(maxHeight)
{
	float intervalX = (tr.x - bl.x) / (m_meshNumX - 1);
	float intervalY = (tr.y - bl.y) / (m_meshNumY - 1);

	for (int yIdx = 0; yIdx < m_meshNumY; ++yIdx)
	{
		for (int xIdx = 0; xIdx < m_meshNumX; ++xIdx)
		{
			float mapPosX = bl.x + intervalX * xIdx;
			float mapPosY = bl.y + intervalY * yIdx;

			float height = GetHeightFromMapCoord(mapPosX, mapPosY);
			m_heights.push_back(height);

			Vector3 vert = Vector3(mapPosX, height, mapPosY);
			m_sourceVerts.push_back(vert);
		}
	}

	// verts
	for (int vertIdx = 0; vertIdx < m_sourceVerts.size(); ++vertIdx)
	{
		// top edge check
		if (!(vertIdx + m_meshNumX >= m_sourceVerts.size()))
		{
			// side edge check
			if ((vertIdx % m_meshNumX) != (m_meshNumX - 1))
			{
				uint blIdx = vertIdx;
				uint brIdx = blIdx + 1;
				uint tlIdx = blIdx + m_meshNumX;
				uint trIdx = tlIdx + 1;

				Vector3 b_l = m_sourceVerts[blIdx];
				Vector3 br = m_sourceVerts[brIdx];
				Vector3 tl = m_sourceVerts[tlIdx];
				Vector3 t_r = m_sourceVerts[trIdx];

				// bl triangle
				m_verts.push_back(b_l);
				m_verts.push_back(br);
				m_verts.push_back(tl);

				m_uvs.push_back(Vector2(0.f, 0.f));
				m_uvs.push_back(Vector2(1.f, 0.f));
				m_uvs.push_back(Vector2(0.f, 1.f));

				Vector3 c1 = tl - b_l;
				Vector3 c2 = br - b_l;
				Vector3 normal = c1.Cross(c2);
				m_normals.push_back(normal);

				c1 = b_l - br;
				c2 = tl - br;
				normal = c1.Cross(c2);
				m_normals.push_back(normal);

				c1 = br - tl;
				c2 = b_l - tl;
				normal = c1.Cross(c2);
				m_normals.push_back(normal);

				// tr triangle
				m_verts.push_back(tl);
				m_verts.push_back(br);
				m_verts.push_back(t_r);

				m_uvs.push_back(Vector2(0.f, 1.f));
				m_uvs.push_back(Vector2(1.f, 0.f));
				m_uvs.push_back(Vector2(1.f, 1.f));

				c1 = t_r - tl;
				c2 = br - tl;
				normal = c1.Cross(c2);
				m_normals.push_back(normal);

				c1 = tl - br;
				c2 = t_r - br;
				normal = c1.Cross(c2);
				m_normals.push_back(normal);

				c1 = br - t_r;
				c2 = tl - t_r;
				normal = c1.Cross(c2);
				m_normals.push_back(normal);
			}
			else
			{
				continue;
			}
		}
		else
		{
			break;
		}
	}
}


SurfacePatch::~SurfacePatch()
{

}


// say mapPosX is 23.5, mapPosY is 47.72, given map of width 100 and height 100
float SurfacePatch::GetHeightFromMapCoord(float mapPosX, float mapPosY)
{
	IntVector2 heightMapDim = m_heightMap->GetImageDimension();

	float cellU = RangeMapFloat(mapPosX, m_bl.x, m_tr.x, 0.f, (float)(heightMapDim.x - 1));
	float cellV = RangeMapFloat(mapPosY, m_bl.y, m_tr.y, (float)(heightMapDim.y - 1), 0.f);			// flip on Y

	int cellULeft = (int)floor(cellU);					
	int cellURight = (int)ceil(cellU);					

	int cellVLeft = (int)floor(cellV);
	int cellVRight = (int)ceil(cellV);

	IntVector2 uvBL = IntVector2(cellULeft, cellVRight);	
	IntVector2 uvBR = IntVector2(cellURight, cellVRight);  
	IntVector2 uvTL = IntVector2(cellULeft, cellVLeft);    
	IntVector2 uvTR = IntVector2(cellURight, cellVLeft);	

	float heightUVBL = GetHeightFromUV(uvBL);		
	float heightUVBR = GetHeightFromUV(uvBR);		
	float heightUVTL = GetHeightFromUV(uvTL);		
	float heightUVTR = GetHeightFromUV(uvTR);		

	// walk right ward at the bottom side
	float heightBLToBR = RangeMapFloat(cellU, (float)cellULeft, (float)cellURight, heightUVBL, heightUVBR);

	// before walking topward, need to know the desired height at top side
	float heightTLToTR = RangeMapFloat(cellU, (float)cellULeft, (float)cellURight, heightUVTL, heightUVTR);

	// and then walk toward that height to the top side
	float height = RangeMapFloat(cellV, (float)cellVRight, (float)cellVLeft, heightBLToBR, heightTLToTR);

	return height;
}


float SurfacePatch::GetHeightFromUV(IntVector2 uv)
{
	Rgba texel = m_heightMap->GetTexel(uv.x, uv.y);

	float heightRaw = static_cast<float>(texel.r);

	float height = RangeMapFloat(heightRaw, 0.f, 255.f, m_bottomHeight, m_maxHeight);

	return height;
}