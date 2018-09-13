#pragma once

#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

class MeshBuilder;

class Submesh
{
public:
	Submesh();
	~Submesh();

	void SetIndices( uint count, size_t stride );  
	void SetVertices( uint count, size_t stride );
	void SetDrawInstruction(  eDrawPrimitiveType type, bool use_indices, uint start_index, uint elem_count ); 
	void SetLayout(VertexLayout layout) { m_layout = layout; }

	template<typename T>
	void FromBuilder(const MeshBuilder& builder);

	VertexLayout*		GetLayout() { return &m_layout; }
	uint				GetMatIndex() const { return m_materialIndex; }
	sDrawInstruction&	GetDrawInstruction() { return m_instruction; }
	int					GetIndexCount() const { return m_ibo.m_indexCount; }
	int					GetVertexCount() const { return m_vbo.m_vertexCount; }
	bool				UseIndices() const { return m_instruction.using_indices; }

public:
	// vertices
	VertexBuffer m_vbo; 

	// indices
	IndexBuffer m_ibo; 

	sDrawInstruction m_instruction;

	VertexLayout m_layout;

	eVertexType m_type;

	bool m_immediate = false;

	//unsigned int m_numIndices;
	uint m_materialIndex;
};


template<typename T>
void Submesh::FromBuilder(const MeshBuilder& builder)
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
