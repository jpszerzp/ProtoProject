#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/ModelLoader.hpp"
#include "Engine/Core/MapChunk.hpp"
#include "Engine/Core/Util/DataUtils.hpp"

Mesh::Mesh()
{

}


Mesh::~Mesh()
{
	
}


void Mesh::SetIndices( uint count, size_t stride )
{
	m_ibo.m_indexStride = (uint)stride;
	m_ibo.m_indexCount = count;
}


void Mesh::SetVertices( uint count, size_t stride )
{
	m_vbo.m_vertexStride = (uint)stride;
	m_vbo.m_vertexCount = count;
}


void Mesh::SetDrawInstruction( eDrawPrimitiveType type, bool use_indices, 
	uint start_index, uint elem_count )
{
	m_instruction.primitive_type = type;
	m_instruction.using_indices = use_indices;
	m_instruction.start_index = start_index;
	m_instruction.elem_count = elem_count;
}


Mesh* Mesh::CreatePoint(eVertexType type)
{
	Vector3 pos = Vector3::ZERO;

	MeshBuilder mb;

	mb.Begin(DRAW_POINT, true);

	mb.SetColor(Rgba::WHITE);
	mb.SetUV(Vector2(0.f, 0.f));
	uint idx = mb.PushVertex(pos);

	mb.AddPoint(idx);

	mb.End();

	return mb.CreateMesh(type, DRAW_POINT);
}


Mesh* Mesh::CreatePointImmediate2D(eVertexType type, Vector2 pos, Rgba color)
{
	Vector3 position = pos.ToVector3(0.f);

	MeshBuilder mb;

	mb.Begin(DRAW_POINT, true);

	mb.SetColor(color);
	mb.SetUV(Vector2::ZERO);
	uint idx = mb.PushVertex(position);

	mb.AddPoint(idx);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_POINT);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateLineImmediate(eVertexType type, const Vector3& startPos,
	const Vector3& endPos, const Rgba& color)
{
	MeshBuilder mb;

	mb.Begin( DRAW_LINE, true );
	mb.SetColor(color);

	mb.SetUV(Vector2(0.f, 0.f));
	uint idx = mb.PushVertex(startPos);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(endPos);

	mb.AddLine(idx, idx + 1);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_LINE);
	mesh->m_immediate = true;
	return mesh;
}


Mesh* Mesh::CreatePointImmediate(eVertexType type, const Vector3& pos, const Rgba& color)
{
	MeshBuilder mb;

	mb.Begin(DRAW_POINT, true);
	mb.SetColor(color);

	mb.SetUV(Vector2::ZERO);
	uint idx = mb.PushVertex(pos);

	mb.AddPoint(idx);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_POINT);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateQuad(eVertexType type)
{
	MeshBuilder mb;

	Vector3 center = Vector3::ZERO;
	Vector3 size = Vector3::ONE;

	Vector3 bl = center + Vector3(-size.x / 2.f, -size.y / 2.f, 0.f);
	Vector3 br = center + Vector3(size.x / 2.f, -size.y / 2.f, 0.f);
	Vector3 tl = center + Vector3(-size.x / 2.f, size.y / 2.f, 0.f);
	Vector3 tr = center + Vector3(size.x / 2.f, size.y / 2.f, 0.f);

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	// Quad
	mb.SetNormal(Vector3(0.f, 0.f, -1.f));
	mb.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));

	mb.SetUV(Vector2(0.f, 0.f));
	uint idx = mb.PushVertex(bl);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(br);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(tl);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(tr);

	mb.AddQuad(idx, idx + 1, idx + 2, idx + 3);

	mb.End();

	return mb.CreateMesh(type, DRAW_TRIANGLE);
}


Mesh* Mesh::CreateQuadTextured(eVertexType type, Vector2 uvBL, Vector2 uvBR, Vector2 uvTL, Vector2 uvTR)
{
	MeshBuilder mb;

	Vector3 center = Vector3::ZERO;
	Vector3 size = Vector3::ONE;

	Vector3 bl = center + Vector3(-size.x / 2.f, -size.y / 2.f, 0.f);
	Vector3 br = center + Vector3(size.x / 2.f, -size.y / 2.f, 0.f);
	Vector3 tl = center + Vector3(-size.x / 2.f, size.y / 2.f, 0.f);
	Vector3 tr = center + Vector3(size.x / 2.f, size.y / 2.f, 0.f);

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	// Quad
	mb.SetNormal(Vector3(0.f, 0.f, -1.f));
	mb.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));

	mb.SetUV(uvBL);
	uint idx = mb.PushVertex(bl);

	mb.SetUV(uvBR);
	mb.PushVertex(br);

	mb.SetUV(uvTL);
	mb.PushVertex(tl);

	mb.SetUV(uvTR);
	mb.PushVertex(tr);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	mb.End();

	return mb.CreateMesh(type, DRAW_TRIANGLE);
}


Mesh* Mesh::CreateQuadImmediate(eVertexType type, const Vector3& bl, const Vector3& br, 
	const Vector3& tl, const Vector3& tr, const Rgba& tint)
{
	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(tint);

	mb.SetUV(Vector2::ZERO);
	uint idx = mb.PushVertex(bl);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(br);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(tl);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(tr);

	mb.AddQuad(idx, idx + 1, idx + 2, idx + 3);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreatePolygonImmedidate(eVertexType type, const Vector2& bl, const Vector2& br, const Vector2& tl, const Vector2& tr, Rgba color)
{
	Vector3 blV3 = Vector3(bl.x, bl.y, 0.f);
	Vector3 brV3 = Vector3(br.x, br.y, 0.f);
	Vector3 tlV3 = Vector3(tl.x, tl.y, 0.f);
	Vector3 trV3 = Vector3(tr.x, tr.y, 0.f);

	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(color);

	mb.SetUV(Vector2(0.f, 0.f));
	uint idx = mb.PushVertex(blV3);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(brV3);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(tlV3);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(trV3);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateQuadTexturedImmediate(eVertexType type, 
	Vector3 drawmin, Vector3 up, Vector3 right, 
	AABB2 glyphbound, float cellHeight, float cellWidth)
{
	Vector3 normRight = right.GetNormalized();
	Vector3 normUp = up.GetNormalized();

	MeshBuilder mb;
	
	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	Vector2 uv_bl = glyphbound.mins;
	Vector3 bl = drawmin;
	mb.SetUV(uv_bl);
	uint idx = mb.PushVertex(bl);

	Vector2 uv_br = glyphbound.mins + Vector2(glyphbound.GetDimensions().x, 0.f);
	Vector3 br = drawmin + normRight * cellWidth;
	mb.SetUV(uv_br);
	mb.PushVertex(br);

	Vector2 uv_tl = glyphbound.mins + Vector2(0.f, glyphbound.GetDimensions().y);
	Vector3 tl = drawmin + normUp * cellHeight;
	mb.SetUV(uv_tl);
	mb.PushVertex(tl);

	Vector2 uv_tr = glyphbound.maxs;
	Vector3 tr = drawmin + normUp * cellHeight + normRight * cellWidth;
	mb.SetUV(uv_tr);
	mb.PushVertex(tr);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}


Mesh* Mesh::CreateCube(eVertexType type)
{
	MeshBuilder mb; 

	Vector3 center = Vector3::ZERO;
	Vector3 size = Vector3::ONE;
	
	Vector3 blb = center - (size / 2.f);
	Vector3 brb	= center + Vector3(size.x / 2.f, -size.y / 2.f, -size.z / 2.f);
	Vector3 trb	= center + Vector3(size.x / 2.f, size.y / 2.f, -size.z / 2.f);
	Vector3 tlb	= center + Vector3(-size.x / 2.f, size.y / 2.f, -size.z / 2.f);
	Vector3 blf	= center + Vector3(-size.x / 2.f, -size.y / 2.f, size.z / 2.f);
	Vector3 brf	= center + Vector3(size.x / 2.f, -size.y / 2.f, size.z / 2.f);
	Vector3 trf	= center + (size / 2.f);
	Vector3 tlf	= center + Vector3(-size.x / 2.f, size.y / 2.f, size.z / 2.f);

	mb.Begin( DRAW_TRIANGLE, true );
	mb.SetColor( Rgba::WHITE ); 

	// Back Face
	mb.SetNormal(Vector3(0.f, 0.f, -1.f));
	mb.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));

	mb.SetUV( Vector2(0.f, 0.f) );
	uint idx = mb.PushVertex( blb ); 

	mb.SetUV( Vector2(1.f, 0.f) );
	mb.PushVertex( brb );

	mb.SetUV( Vector2(0.f, 1.f) );
	mb.PushVertex( tlb ); 

	mb.SetUV( Vector2(1.f, 1.f) );
	mb.PushVertex( trb ); 

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	// Front Face
	mb.SetNormal(Vector3(0.f, 0.f, 1.f));
	mb.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));

	mb.SetUV( Vector2(0.f, 0.f) );
	idx = mb.PushVertex( brf ); 

	mb.SetUV( Vector2(1.f, 0.f) );
	mb.PushVertex( blf ); 

	mb.SetUV( Vector2(0.f, 1.f) );
	mb.PushVertex( trf ); 

	mb.SetUV( Vector2(1.f, 1.f) );
	mb.PushVertex( tlf ); 

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	// Left face
	mb.SetNormal(Vector3(-1.f, 0.f, 0.f));
	mb.SetTangent(Vector4(0.f, 0.f, 1.f, 1.f));

	mb.SetUV(Vector2(0.f, 0.f));
	idx = mb.PushVertex(blf);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(blb);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(tlf);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(tlb);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	// Right face
	mb.SetNormal(Vector3(1.f, 0.f, 0.f));
	mb.SetTangent(Vector4(0.f, 0.f, -1.f, 1.f));

	mb.SetUV(Vector2(0.f, 0.f));
	idx = mb.PushVertex(brb);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(brf);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(trb);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(trf);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	// Top face
	mb.SetNormal(Vector3(0.f, 1.f, 0.f));
	mb.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));

	mb.SetUV(Vector2(0.f, 0.f));
	idx = mb.PushVertex(tlf);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(tlb);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(trf);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(trb);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	// Bottom face
	mb.SetNormal(Vector3(0.f, -1.f, 0.f));
	mb.SetTangent(Vector4(1.f, 0.f, 0.f, 1.f));

	mb.SetUV(Vector2(0.f, 0.f));
	idx = mb.PushVertex(brf);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(brb);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(blf);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(blb);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	mb.End(); 

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_type = type;
	return mesh; 
}


Mesh* Mesh::CreateUVSphere( eVertexType type, uint slices, uint wedges )
{
	// (0,0) to (1,1)

	Vector3 position = Vector3::ZERO;
	float radius = 1.f;

	MeshBuilder mb; 
	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);
	uint idx = (uint)mb.m_vertices.size();

	for (uint sliceIdx = 0; sliceIdx <= slices; ++sliceIdx ) {
		float v = (float)(sliceIdx) / (float)(slices);
		float azimuth = 180.f * v; 

		for (uint wedgeIdx = 0; wedgeIdx <= wedges; ++wedgeIdx ) {
			float u = (float)(wedgeIdx) / (float)(wedges); 
			float rot = 360.f * u;

			mb.SetUV( Vector2(u, v) ); 
			Vector3 pos = position + PolarToCartesian( radius, rot, azimuth ); 
			Vector3 normal = (pos - position).GetNormalized();
			mb.SetNormal(normal);

			Vector3 normalXZ = Vector3(normal.x, 0.f, normal.z);
			Vector3 projectedXZ = GetProjectedVector(normal, normalXZ);
			Vector2 projected2D = Vector2(projectedXZ.x, projectedXZ.z);
			Vector2 rotated2D = projected2D.RotateDegree2D(90.f);
			Vector3 rotatedXZ = Vector3(rotated2D.x, 0.f, rotated2D.y).GetNormalized();
			Vector3 tangent = rotatedXZ.Cross(normal).GetNormalized();
			Vector4 tangentV4 = Vector4(tangent.x, tangent.y, tangent.z, 1.f);
			mb.SetTangent(tangentV4);

			mb.PushVertex( pos ); 
		}
	}

	for (uint sliceIdx = 0; sliceIdx < slices; ++sliceIdx ) {
		for (uint wedgeIdx = 0; wedgeIdx < wedges; ++wedgeIdx ) {
			uint tlIdx = wedges * sliceIdx + wedgeIdx + idx;
			uint trIdx = tlIdx + 1;
			uint blIdx = tlIdx + wedges;
			uint brIdx = blIdx + 1;

			mb.AddQuad( blIdx, brIdx, tlIdx, trIdx ); 
		}
	}

	mb.End();

	return mb.CreateMesh(type, DRAW_TRIANGLE); 
}


Mesh* Mesh::CreateModel(std::string fp, eVertexType type)
{
	std::vector<Vector3> verts;
	std::vector<Vector3> uvs;
	std::vector<Vector3> normals;

	LoadObj(fp, verts, uvs, normals);

	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	// Add face by triangles
	for (uint vertIdx = 0; vertIdx < verts.size(); vertIdx += 3)
	{
		Vector2 uv_0 = Vector2(uvs[vertIdx].x, uvs[vertIdx].y);
		Vector2 uv_1 = Vector2(uvs[vertIdx + 1].x, uvs[vertIdx + 1].y);
		Vector2 uv_2 = Vector2(uvs[vertIdx + 2].x, uvs[vertIdx + 2].y);

		Vector3 normal_0 = normals[vertIdx];
		Vector3 normal_1 = normals[vertIdx + 1];
		Vector3 normal_2 = normals[vertIdx + 2];

		Vector3 pos_0 = verts[vertIdx];
		Vector3 pos_1 = verts[vertIdx + 1];
		Vector3 pos_2 = verts[vertIdx + 2];

		Vector3 e1 = pos_0 - pos_2;
		Vector3 e2 = pos_1 - pos_2;
		Vector2 uv1 = uv_0 - uv_2;
		Vector2 uv2 = uv_1 - uv_2;
		Vector4 tan = mb.CalcTangent(e1, e2, uv1, uv2);
		mb.SetTangent(tan);

		mb.SetUV(uv_0);
		mb.SetNormal(normal_0);
		mb.PushVertex(pos_0);

		mb.SetUV(uv_1);
		mb.SetNormal(normal_1);
		mb.PushVertex(pos_1);

		mb.SetUV(uv_2);
		mb.SetNormal(normal_2);
		mb.PushVertex(pos_2);

		mb.AddTriangle(vertIdx, vertIdx + 1, vertIdx + 2);
	}

	mb.End();

	return mb.CreateMesh(type, DRAW_TRIANGLE);
}


Mesh* Mesh::CreateTerrainImmediateFromSurfacePatch(SurfacePatch* patch, eVertexType type)
{
	std::vector<Vector3> verts = patch->m_verts;
	std::vector<Vector2> uvs = patch->m_uvs;
	std::vector<Vector3> normals = patch->m_normals;

	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	// Add face by triangles
	for (uint vertIdx = 0; vertIdx < verts.size(); vertIdx += 3)
	{
		Vector2 uv_0 = uvs[vertIdx];
		Vector2 uv_1 = uvs[vertIdx + 1];
		Vector2 uv_2 = uvs[vertIdx + 2];

		Vector3 normal_0 = normals[vertIdx];
		Vector3 normal_1 = normals[vertIdx + 1];
		Vector3 normal_2 = normals[vertIdx + 2];

		Vector3 pos_0 = verts[vertIdx];
		Vector3 pos_1 = verts[vertIdx + 1];
		Vector3 pos_2 = verts[vertIdx + 2];

		Vector3 e1 = pos_0 - pos_2;
		Vector3 e2 = pos_1 - pos_2;
		Vector2 uv1 = uv_0 - uv_2;
		Vector2 uv2 = uv_1 - uv_2;
		Vector4 tan = mb.CalcTangent(e1, e2, uv1, uv2);
		mb.SetTangent(tan);

		mb.SetUV(uv_0);
		mb.SetNormal(normal_0);
		mb.PushVertex(pos_0);

		mb.SetUV(uv_1);
		mb.SetNormal(normal_1);
		mb.PushVertex(pos_1);

		mb.SetUV(uv_2);
		mb.SetNormal(normal_2);
		mb.PushVertex(pos_2);

		mb.AddTriangle(vertIdx, vertIdx + 1, vertIdx + 2);
	}

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}


Mesh* Mesh::CreateTerrainImmediateFromChunk(MapChunk* chunk, eVertexType type)
{
	std::vector<Vector3> verts = chunk->m_triangleVerts;
	std::vector<Vector2> uvs = chunk->m_triangleUVs;
	std::vector<Vector3> normals = chunk->m_triangleNormals;

	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	// Add face by triangles
	for (uint vertIdx = 0; vertIdx < verts.size(); vertIdx += 3)
	{
		Vector2 uv_0 = uvs[vertIdx];
		Vector2 uv_1 = uvs[vertIdx + 1];
		Vector2 uv_2 = uvs[vertIdx + 2];

		Vector3 normal_0 = normals[vertIdx];
		Vector3 normal_1 = normals[vertIdx + 1];
		Vector3 normal_2 = normals[vertIdx + 2];

		Vector3 pos_0 = verts[vertIdx];
		Vector3 pos_1 = verts[vertIdx + 1];
		Vector3 pos_2 = verts[vertIdx + 2];

		Vector3 e1 = pos_0 - pos_2;
		Vector3 e2 = pos_1 - pos_2;
		Vector2 uv1 = uv_0 - uv_2;
		Vector2 uv2 = uv_1 - uv_2;
		Vector4 tan = mb.CalcTangent(e1, e2, uv1, uv2);
		mb.SetTangent(tan);

		mb.SetUV(uv_0);
		mb.SetNormal(normal_0);
		mb.PushVertex(pos_0);

		mb.SetUV(uv_1);
		mb.SetNormal(normal_1);
		mb.PushVertex(pos_1);

		mb.SetUV(uv_2);
		mb.SetNormal(normal_2);
		mb.PushVertex(pos_2);

		mb.AddTriangle(vertIdx, vertIdx + 1, vertIdx + 2);
	}

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}


Mesh* Mesh::CreateTriangleImmediate(eVertexType type, const Rgba& color,
	const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(color);

	mb.SetUV(Vector2::ZERO);
	uint idx = mb.PushVertex(v1);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(v2);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(v3);

	mb.AddTriangle(idx, idx + 1, idx + 2);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateTetrahedronImmediate(eVertexType type, const Rgba& color, const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4)
{
	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(color);

	// f1
	mb.SetUV(Vector2::ZERO);
	uint idx = mb.PushVertex(v1);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(v2);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(v3);

	mb.AddTriangle(idx, idx + 1, idx + 2);

	// f2
	mb.SetUV(Vector2::ZERO);
	idx = mb.PushVertex(v2);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(v3);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(v4);

	mb.AddTriangle(idx, idx + 1, idx + 2);

	// f3
	mb.SetUV(Vector2::ZERO);
	idx = mb.PushVertex(v3);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(v4);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(v1);

	mb.AddTriangle(idx, idx + 1, idx + 2);

	// f4
	mb.SetUV(Vector2::ZERO);
	idx = mb.PushVertex(v2);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(v4);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(v1);

	mb.AddTriangle(idx, idx + 1, idx + 2);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateCone(eVertexType type, int base_side)
{
	// apex
	Vector3 apex = Vector3(0.f, 1.f, 0.f);
	Vector3 base = Vector3::ZERO;

	MeshBuilder mb;

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	// uv, tangent and normal are different per faces
	float del_deg = 360.f / (float)(base_side);
	for (int i = 0; i < base_side; ++i)
	{
		float first_angle = del_deg * i;
		float next_angle = del_deg * (i + 1);
		float first_x = 1.f * CosDegrees(first_angle);
		float first_z = 1.f * SinDegrees(first_angle);
		float next_x = 1.f * CosDegrees(next_angle);
		float next_z = 1.f * SinDegrees(next_angle);
		Vector3 bottom_vert_0 = Vector3(first_x, 0.f, first_z);
		Vector3 bottom_vert_1 = Vector3(next_x, 0.f, next_z);

		Vector3 tangentV3 = (bottom_vert_1 - bottom_vert_0).GetNormalized();
		Vector4 tangent = tangentV3.ToVector4(1.f);
		mb.SetTangent(tangent);

		Vector3 toApex = (apex - bottom_vert_0).GetNormalized();
		Vector3 normal = tangentV3.Cross(toApex);
		mb.SetNormal(normal);

		mb.SetUV(Vector2(0.f, 0.f));			// uv
		uint idx = mb.PushVertex(bottom_vert_0);

		mb.SetUV(Vector2(1.f, 0.f));
		mb.PushVertex(bottom_vert_1);

		mb.SetUV(Vector2(0.5f, 1.f));
		mb.PushVertex(apex);

		mb.AddTriangle(idx, idx + 1, idx + 2);

		mb.SetTangent(tangent);

		mb.SetNormal(Vector3(0.f, -1.f, 0.f));

		mb.SetUV(Vector2(1.f, 0.f));
		idx = mb.PushVertex(bottom_vert_1);

		mb.SetUV(Vector2(0.f, 0.f));
		mb.PushVertex(bottom_vert_0);

		mb.SetUV(Vector2(0.5f, 1.f));
		mb.PushVertex(base);

		mb.AddTriangle(idx, idx + 1, idx + 2);
	}

	mb.End(); 

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_type = type;
	return mesh; 
}

Mesh* Mesh::CreateQuad2D(eVertexType type, Rgba color /*= Rgba::WHITE*/)
{
	MeshBuilder mb;

	Vector3 center = Vector3::ZERO;
	Vector3 size = Vector3::ONE;

	Vector3 bl = center + Vector3(-size.x / 2.f, -size.y / 2.f, 0.f);
	Vector3 br = center + Vector3(size.x / 2.f, -size.y / 2.f, 0.f);
	Vector3 tl = center + Vector3(-size.x / 2.f, size.y / 2.f, 0.f);
	Vector3 tr = center + Vector3(size.x / 2.f, size.y / 2.f, 0.f);

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(color);

	// not setting normal and tangent for 2D

	mb.SetUV(Vector2(0.f, 0.f));
	uint idx = mb.PushVertex(bl);

	mb.SetUV(Vector2(1.f, 0.f));
	mb.PushVertex(br);

	mb.SetUV(Vector2(0.f, 1.f));
	mb.PushVertex(tl);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(tr);

	mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);

	mb.End();

	return mb.CreateMesh(type, DRAW_TRIANGLE);
}

Mesh* Mesh::CreateDisc2D(eVertexType type, Rgba color /*= Rgba::WHITE*/)
{
	MeshBuilder mb;
	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(color);

	int lineSegNum = 100;
	float deltaAngle = 360.f / (float)lineSegNum;
	float radius = 1.f;
	Vector2 center = Vector2::ZERO;
	Vector3 center3D = center.ToVector3(0.f);

	for (int idx = 0; idx < lineSegNum; ++idx)
	{
		Vector2 startDeviation = Vector2(CosDegrees(idx * deltaAngle), SinDegrees(idx * deltaAngle)) * radius;
		Vector2 endDeviation = Vector2(CosDegrees((idx + 1) * deltaAngle), SinDegrees((idx + 1) * deltaAngle)) * radius;

		Vector2 start = center + startDeviation;
		Vector2 end = center + endDeviation;

		Vector3 start3D = start.ToVector3(0.f);
		Vector3 end3D = end.ToVector3(0.f);

		mb.SetUV(Vector2(0.f, 0.f));
		uint vertIdx = mb.PushVertex(center3D);

		mb.SetUV(Vector2(1.f, 0.f));
		mb.PushVertex(start3D);

		mb.SetUV(Vector2(0.f, 1.f));
		mb.PushVertex(end3D);

		mb.AddTriangle(vertIdx + 0, vertIdx + 1, vertIdx + 2);
	}

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	return mesh;
}

Mesh* Mesh::CreateDiscImmediate2D(Vector2 center, Rgba tint, float radius, int lineSegNum)
{
	MeshBuilder mb;
	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(tint);

	float deltaAngle = 360.f / (float)lineSegNum;
	Vector3 center3D = center.ToVector3(0.f);

	for (int idx = 0; idx < lineSegNum; ++idx)
	{
		Vector2 startDeviation = Vector2(CosDegrees(idx * deltaAngle), SinDegrees(idx * deltaAngle)) * radius;
		Vector2 endDeviation = Vector2(CosDegrees((idx + 1) * deltaAngle), SinDegrees((idx + 1) * deltaAngle)) * radius;

		Vector2 start = center + startDeviation;
		Vector2 end = center + endDeviation;

		Vector3 start3D = start.ToVector3(0.f);
		Vector3 end3D = end.ToVector3(0.f);

		mb.SetUV(Vector2(0.f, 0.f));
		uint vertIdx = mb.PushVertex(center3D);

		mb.SetUV(Vector2(1.f, 0.f));
		mb.PushVertex(start3D);

		mb.SetUV(Vector2(0.f, 1.f));
		mb.PushVertex(end3D);

		mb.AddTriangle(vertIdx + 0, vertIdx + 1, vertIdx + 2);
	}

	mb.End();

	Mesh* mesh = mb.CreateMesh(VERT_PCU, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateLineImmediate2D(const Vector2& start, const Vector2& end, const Rgba& tint, eVertexType type)
{
	MeshBuilder mb;

	Vector3 start3 = Vector3(start.x, start.y, 0.f);
	Vector3 end3 = Vector3(end.x, end.y, 0.f);

	mb.Begin( DRAW_LINE, true );
	mb.SetColor(tint);

	mb.SetUV(Vector2(0.f, 0.f));
	uint idx = mb.PushVertex(start3);

	mb.SetUV(Vector2(1.f, 1.f));
	mb.PushVertex(end3);

	mb.AddLine(idx + 0, idx + 1);

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_LINE);
	mesh->m_immediate = true;
	return mesh;
}

Mesh* Mesh::CreateTextImmediate(Rgba color, const Vector2& drawmin, const BitmapFont* font,
	float cellHeight, float asepctScale, std::string text, eVertexType type)
{
	MeshBuilder mb;
	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(color);

	//Create and attach new mesh.
	//We have updated text stored.
	//Process each character, and create an integrated mesh for it.
	float cellWidth = cellHeight * (font->GetGlyphAspect() * asepctScale);
	for (size_t charIdx = 0; charIdx < text.length(); ++charIdx)
	{
		const char& character = text.at(charIdx);
		Vector2 bl = Vector2(drawmin.x + (charIdx * cellWidth), drawmin.y);			// pos is drawmin
		Vector2 br = Vector2(bl.x + cellWidth, bl.y);
		Vector2 tl = Vector2(bl.x, bl.y + cellHeight);
		Vector2 tr = Vector2(bl.x + cellWidth, bl.y + cellHeight);

		AABB2 uvBound = font->GetUVsForGlyph(character);
		std::vector<Vector2> corners = uvBound.GetCornersFromBLInCounterClockwise();
		Vector2 uvBL = corners[0];
		Vector2 uvBR = corners[1];
		Vector2 uvTL = corners[3];
		Vector2 uvTR = corners[2];

		// Glyph bl
		mb.SetUV(uvBL);
		uint idx = mb.PushVertex(bl.ToVector3(0.f));

		mb.SetUV(uvBR);
		mb.PushVertex(br.ToVector3(0.f));

		mb.SetUV(uvTL);
		mb.PushVertex(tl.ToVector3(0.f));

		mb.SetUV(uvTR);
		mb.PushVertex(tr.ToVector3(0.f));

		mb.AddQuad(idx + 0, idx + 1, idx + 2, idx + 3);
	}

	mb.End();

	Mesh* mesh = mb.CreateMesh(type, DRAW_TRIANGLE);
	mesh->m_immediate = true;
	mesh->m_textMesh = true;
	mesh->m_textMeshColor = color;
	mesh->m_textMeshDrawmin2 = drawmin;
	return mesh;
}