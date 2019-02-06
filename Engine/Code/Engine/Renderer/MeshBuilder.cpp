#include "Engine/Renderer/MeshBuilder.hpp"


void MeshBuilder::Begin(eDrawPrimitiveType prim, bool use_indices)
{
	m_draw.primitive_type = prim; 
	m_draw.using_indices = use_indices; 

	if (use_indices) {
		m_draw.start_index = (uint)m_indices.size(); 
	} else {
		m_draw.start_index = (uint)m_vertices.size(); 
	}
}

void MeshBuilder::End()
{
	uint end_idx;
	if (m_draw.using_indices) {
		end_idx = (uint)m_indices.size(); 
	} else {
		end_idx = (uint)m_vertices.size(); 
	}

	m_draw.elem_count = end_idx - m_draw.start_index; 
}

void MeshBuilder::SetColor(Rgba const &c)
{
	m_stamp.m_color = c; 
}

void MeshBuilder::SetUV(Vector2 const &uv)
{
	m_stamp.m_uv = uv; 
}

void MeshBuilder::SetNormal(Vector3 const &normal)
{
	m_stamp.m_normal = normal;
}

void MeshBuilder::SetTangent(Vector4 const &tangent)
{
	m_stamp.m_tangent = tangent;
}

Vector4 MeshBuilder::CalcTangent(Vector3 edge1, Vector3 edge2, Vector2 uv1, Vector2 uv2)
{
	Vector4 tangent;

	float f = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

	tangent.x = f * (uv2.y * edge1.x - uv1.y * edge2.x);
	tangent.y = f * (uv2.y * edge1.y - uv1.y * edge2.y);
	tangent.z = f * (uv2.y * edge1.z - uv1.y * edge2.z);
	tangent.w = 1.f;

	return tangent;
}

uint MeshBuilder::PushVertex(Vector3 position)
{
	m_stamp.m_position = position; 
	m_vertices.push_back( m_stamp ); 

	return (uint)(m_vertices.size() - 1); 
}

Mesh* MeshBuilder::CreateMesh(eVertexType vertType, eDrawPrimitiveType drawType)
{
	Mesh* mesh = new Mesh(); 

	// set draw call
	m_draw.primitive_type = drawType;
	m_draw.start_index = 0;
	m_draw.using_indices = (m_indices.size() != 0);
	m_draw.elem_count = m_draw.using_indices ? (uint)m_indices.size() : (uint)m_vertices.size();
	mesh->SetDrawInstruction(m_draw.primitive_type, m_draw.using_indices,
		m_draw.start_index, m_draw.elem_count);
	if (vertType == VERT_PCU)
	{
		mesh->SetLayout(Vertex_3DPCU::s_layout);
		mesh->FromBuilder<Vertex_3DPCU>(*this);
		mesh->SetVertices( (uint)(m_vertices.size()), sizeof(Vertex_3DPCU) );
	}
	else if (vertType == VERT_LIT)
	{
		mesh->SetLayout(VertexLit::s_layout);
		mesh->FromBuilder<VertexLit>(*this);
		mesh->SetVertices( (uint)(m_vertices.size()), sizeof(VertexLit) );
	}
	mesh->SetIndices( (uint)(m_indices.size()), sizeof(uint) ); 

	return mesh; 
}

void MeshBuilder::AddTriangle(uint idx1, uint idx2, uint idx3)
{
	m_indices.push_back(idx1);
	m_indices.push_back(idx2);
	m_indices.push_back(idx3);
}

void MeshBuilder::AddQuad(uint idx1, uint idx2, uint idx3, uint idx4)
{
	AddTriangle(idx1, idx2, idx3);
	AddTriangle(idx3, idx2, idx4);
}

void MeshBuilder::AddLine(uint idx1, uint idx2)
{
	m_indices.push_back(idx1);
	m_indices.push_back(idx2);
}

void MeshBuilder::AddPoint(uint idx)
{
	m_indices.push_back(idx);
}

int MeshBuilder::GetVertexCount() const
{
	return (int)m_vertices.size();
}

int MeshBuilder::GetIndexCount() const
{
	return (int)m_indices.size();
}
