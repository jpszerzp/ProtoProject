#include "Engine/Renderer/Submesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

Submesh::Submesh()
{

}

Submesh::~Submesh()
{

}

void Submesh::SetIndices( uint count, size_t stride )
{
	m_ibo.m_indexStride = (uint)stride;
	m_ibo.m_indexCount = count;
}


void Submesh::SetVertices( uint count, size_t stride )
{
	m_vbo.m_vertexStride = (uint)stride;
	m_vbo.m_vertexCount = count;
}

void Submesh::SetDrawInstruction( eDrawPrimitiveType type, bool use_indices, 
	uint start_index, uint elem_count )
{
	m_instruction.primitive_type = type;
	m_instruction.using_indices = use_indices;
	m_instruction.start_index = start_index;
	m_instruction.elem_count = elem_count;
}