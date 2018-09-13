#include "Engine/Core/MapChunk.hpp"
#include "Engine/Core/TerrainMap.hpp"
#include "Engine/Math/MathUtils.hpp"


MapChunk::MapChunk()
{
	m_chunkCoord = IntVector2(-1, -1);
	m_renderable = nullptr;
	m_terrainMap = nullptr;
}


MapChunk::~MapChunk()
{
	Cleanup();
}


void MapChunk::SetupChunkInMap(TerrainMap* map, IntVector2 chunkCoord)
{
	m_terrainMap = map;
	m_chunkCoord = chunkCoord;
}


void MapChunk::SetupChunkRenderable(Vector2 bl, Vector2 tr, int meshNumX, int meshNumY)
{
	m_meshPointNumX = meshNumX;
	m_meshPointNumY = meshNumY;

	Renderer* renderer = Renderer::GetInstance();

	Vector3 pos = Vector3::ZERO;
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Transform transform = Transform(pos, rot, scale);

	Rgba tint = Rgba::WHITE;
	Vector4 tintV4;
	tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);

	Material* material = renderer->CreateOrGetStagedMaterial("");

	float intervalX = (tr.x - bl.x) / (meshNumX - 1);
	float intervalY = (tr.y - bl.y) / (meshNumY - 1);
	for (int yIdx = 0; yIdx < meshNumY; ++yIdx)
	{
		for (int xIdx = 0; xIdx < meshNumX; ++xIdx)
		{
			float posX = bl.x + intervalX * xIdx;
			float posY = bl.y + intervalY * yIdx;

			float height = GetHeightFromPos(posX, posY);
			m_vertHeights.push_back(height);

			Vector3 vert = Vector3(posX, height, posY);
			m_verts.push_back(vert);
		}
	}

	for (int vertIdx = 0; vertIdx < m_verts.size(); ++vertIdx)
	{
		if (!(vertIdx + meshNumX >= m_verts.size()))
		{
			if ((vertIdx % meshNumX) != (meshNumX - 1))
			{
				uint blIdx = vertIdx;
				uint brIdx = blIdx + 1;
				uint tlIdx = blIdx + meshNumX;
				uint trIdx = tlIdx + 1;

				Vector3 b_l = m_verts[blIdx];
				Vector3 br = m_verts[brIdx];
				Vector3 tl = m_verts[tlIdx];
				Vector3 t_r = m_verts[trIdx];

				// bl triangle
				m_triangleVerts.push_back(b_l);
				m_triangleVerts.push_back(br);
				m_triangleVerts.push_back(tl);

				m_triangleUVs.push_back(Vector2(0.f, 0.f));
				m_triangleUVs.push_back(Vector2(1.f, 0.f));
				m_triangleUVs.push_back(Vector2(0.f, 1.f));

				Vector3 c1 = tl - b_l;
				Vector3 c2 = br - b_l;
				Vector3 normal = c1.Cross(c2);
				m_triangleNormals.push_back(normal);

				c1 = b_l - br;
				c2 = tl - br;
				normal = c1.Cross(c2);
				m_triangleNormals.push_back(normal);

				c1 = br - tl;
				c2 = b_l - tl;
				normal = c1.Cross(c2);
				m_triangleNormals.push_back(normal);

				// record this as planar normal (the plane where this triangle is on)
				m_triangleCenterNormals.push_back(normal);

				// barycentric center
				Vector3 barycentricCenter = ComputeTriangleCenter(b_l, br, tl);
				m_triangleCenters.push_back(barycentricCenter);

				// tr triangle
				m_triangleVerts.push_back(tl);
				m_triangleVerts.push_back(br);
				m_triangleVerts.push_back(t_r);

				m_triangleUVs.push_back(Vector2(0.f, 1.f));
				m_triangleUVs.push_back(Vector2(1.f, 0.f));
				m_triangleUVs.push_back(Vector2(1.f, 1.f));

				c1 = t_r - tl;
				c2 = br - tl;
				normal = c1.Cross(c2);
				m_triangleNormals.push_back(normal);

				c1 = tl - br;
				c2 = t_r - br;
				normal = c1.Cross(c2);
				m_triangleNormals.push_back(normal);

				c1 = br - t_r;
				c2 = tl - t_r;
				normal = c1.Cross(c2);
				m_triangleNormals.push_back(normal);

				// record this as planar normal (the plane where this triangle is on)
				m_triangleCenterNormals.push_back(normal);

				// barycentric center
				barycentricCenter = ComputeTriangleCenter(tl, br, t_r);
				m_triangleCenters.push_back(barycentricCenter);
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

	Mesh* mesh = Mesh::CreateTerrainImmediateFromChunk(this, VERT_LIT);

	m_renderable = new Renderable(material, mesh, transform, tintV4);
}


void MapChunk::Cleanup()
{
	delete m_renderable;
	m_renderable = nullptr;
}


float MapChunk::GetHeightFromPos(float posX, float posY)
{
	return m_terrainMap->GetTerrainHeightFromMappos(Vector2(posX, posY));
}