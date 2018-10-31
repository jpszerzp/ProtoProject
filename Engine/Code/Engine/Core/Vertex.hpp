#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>

///////////////////////////////////// Builder //////////////////////////////////////////
struct sVertexBuilder
{
	Vector3 m_position; 
	Vector2 m_uv; 
	Rgba m_color; 
	Vector3 m_normal;
	Vector4 m_tangent;
};

//////////////////////////////////// Render Type ////////////////////////////////////
enum eRenderType 
{
	RT_FLOAT,
	RT_UNSIGNED_BYTE,
	NUM_RENDERTYPE
};

//////////////////////////////////// Vertex Type ////////////////////////////////////
enum eVertexType
{
	VERT_PCU,
	VERT_LIT,
	NUM_VERT_TYPE
};

///////////////////////////////////// Attribute /////////////////////////////////////
struct VertexAttribute
{
	VertexAttribute() {}
	VertexAttribute( char const *name, eRenderType type, uint count, bool normalized, size_t offset );

	std::string m_handle; // POSITION

	eRenderType m_type;
	int m_elementCount; 
	bool m_isNormalized; 

	int m_memberOffset; // how far from start of element to this piece of data
};

///////////////////////////////////// Layout /////////////////////////////////////
class VertexLayout 
{
public:
	VertexLayout() {}
	VertexLayout( size_t stride, VertexAttribute const *layout, int attributeNum);

public:
	std::vector<VertexAttribute> m_attributes; 
	int m_stride; // how far between element
	int m_attributeNum;

	int GetAttributeCount() const { return (int)m_attributes.size(); }
	VertexAttribute* GetAttribute(int idx) { return &(m_attributes.at(idx)); }
};

///////////////////////////////////// Vertex Structure //////////////////////////////////////////
struct VertexLit
{
	Vector3 m_pos;
	Rgba m_color;
	Vector2 m_uv;
	Vector3 m_normal;
	Vector4 m_tangent;

	static VertexAttribute const s_attributes[];
	static VertexLayout const s_layout;

	VertexLit(sVertexBuilder builder)
	{
		m_pos = builder.m_position;
		m_color = builder.m_color;
		m_normal = builder.m_normal;
		m_uv = builder.m_uv;
		m_tangent = builder.m_tangent;
	}
	VertexLit(Vector3 pos, Rgba color, Vector2 uv, Vector3 normal, /*Vector3 tangent*/Vector4 tangent)
		: m_pos(pos), m_color(color), m_uv(uv), m_normal(normal), m_tangent(tangent) {}
	~VertexLit() {}
};

struct Vertex_3DPCU
{
	Vector3 m_pos;
	Rgba m_color;
	Vector2 m_uv;

	static VertexAttribute const s_attributes[]; 
	static VertexLayout const s_layout; 

	Vertex_3DPCU(sVertexBuilder builder)
	{
		m_pos = builder.m_position;
		m_color = builder.m_color;
		m_uv = builder.m_uv;
	}
	Vertex_3DPCU(){}
	Vertex_3DPCU(Vector3 pos, Rgba color, Vector2 uv) : m_pos(pos), m_color(color), m_uv(uv) {}
	~Vertex_3DPCU() {}
};
