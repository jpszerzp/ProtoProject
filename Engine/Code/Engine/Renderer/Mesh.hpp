#pragma once

//#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/SurfacePatch.hpp"
#include "Engine/Renderer/Renderer.hpp"

class MeshBuilder;
class MapChunk;

// Just Geometry
class Mesh 
{
public:
	Mesh();
	~Mesh();

	void SetIndices( uint count, size_t stride );  
	void SetVertices( uint count, size_t stride );
	void SetDrawInstruction(  eDrawPrimitiveType type, bool use_indices, uint start_index, uint elem_count ); 
	void SetLayout(VertexLayout layout) { m_layout = layout; }

	// 3D
	static Mesh* CreatePoint(eVertexType type);
	static Mesh* CreateLineImmediate(eVertexType type, const Vector3& startPos, const Vector3& endPos, const Rgba& color);
	static Mesh* CreatePointImmediate(eVertexType type, const Vector3& pos, const Rgba& color);
	static Mesh* CreateQuad(eVertexType type);
	static Mesh* CreateQuad20(eVertexType type);
	static Mesh* CreateQuadImmediate(eVertexType type, const Vector3& bl, const Vector3& br, 
		const Vector3& tl, const Vector3& tr, const Rgba& tint);
	static Mesh* CreateQuadTextured(eVertexType, Vector2 uvBL, Vector2 uvBR, Vector2 uvTL, Vector2 uvTR);
	static Mesh* CreateQuadTexturedImmediate(eVertexType type, Vector3 drawmin, Vector3 up, Vector3 right, AABB2 glphybound, float cellHeight, float cellWidth);
	static Mesh* CreatePolygonImmedidate(eVertexType type, const Vector2& bl, const Vector2& br, const Vector2& tl, const Vector2& tr, Rgba color);
	static Mesh* CreateCube(eVertexType type);
	static Mesh* CreateUVSphere( eVertexType type, uint slices, uint wedges );
	static Mesh* CreateModel(std::string fp, eVertexType type);
	static Mesh* CreateTextImmediate(Rgba color, const Vector2& drawmin, const BitmapFont* font,
		float cellHeight, float asepctScale, std::string text, eVertexType type);
	static Mesh* CreateTerrainImmediateFromSurfacePatch(SurfacePatch* patch, eVertexType type);
	static Mesh* CreateTerrainImmediateFromChunk(MapChunk* chunk, eVertexType type);
	static Mesh* CreateTriangleImmediate(eVertexType type, const Rgba& color,
		const Vector3& v1, const Vector3& v2, const Vector3& v3);
	static Mesh* CreateTetrahedronImmediate(eVertexType type, const Rgba& color,
		const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4);
	static Mesh* CreateCone(eVertexType type, int base_side);

	// 2D
	static Mesh* CreateQuad2D(eVertexType type, Rgba color = Rgba::WHITE);
	static Mesh* CreateDisc2D(eVertexType type, Rgba color = Rgba::WHITE);
	//static Mesh* CreateCapsule2D(eVertexType type, Rgba color = Rgba::WHITE);
	static Mesh* CreateDiscImmediate2D(Vector2 center, Rgba tint, float radius, int lineSegNum);
	static Mesh* CreateLineImmediate2D(const Vector2& start, const Vector2& end, const Rgba& tint, eVertexType type);
	static Mesh* CreatePointImmediate2D(eVertexType type, Vector2 pos, Rgba color);

	VertexLayout*		GetLayout() { return &m_layout; }
	sDrawInstruction&	GetDrawInstruction() { return m_instruction; }
	int					GetIndexCount() const { return m_ibo.m_indexCount; }
	int					GetVertexCount() const { return m_vbo.m_vertexCount; }
	bool				UseIndices() const { return m_instruction.using_indices; }

	template<typename T>
	void FromBuilder(const MeshBuilder& builder);

public:
	// Attempt in bringing mesh builder into the mesh so that
	// push to vertex does not happen every frame.
	// This is applied only to ship for now.

	// vertices
	VertexBuffer m_vbo; 

	// indices
	IndexBuffer m_ibo; 

	// draw call 
	sDrawInstruction m_instruction; 

	VertexLayout m_layout;

	eVertexType m_type;

	bool m_immediate = false;

	std::vector<Texture*> m_texFromAssimp;		// does not fall in the "create or get" pattern

public:
	// text mesh properties
	bool m_textMesh = false;
	Rgba m_textMeshColor;
	Vector2 m_textMeshDrawmin2;		// 2D
};


template<typename T>
void Mesh::FromBuilder(const MeshBuilder& builder)
{
	size_t vsize = m_layout.m_stride * builder.GetVertexCount(); 
	size_t isize = sizeof(uint) * builder.GetIndexCount();

	std::vector<T> vertices;
	for (std::vector<sVertexBuilder>::size_type vbCount = 0; 
		vbCount < builder.m_vertices.size(); ++vbCount)
	{
		T vt(builder.m_vertices[vbCount]);
		vertices.push_back(vt);
	}

	m_vbo.CopyToGPU( vsize, &vertices[0] );
	m_ibo.CopyToGPU( isize, &builder.m_indices[0] );
}
