#include "Engine/Core/Vertex.hpp"
#include "Engine/Renderer/GLFunctions.hpp"

#include <iterator>

VertexAttribute::VertexAttribute(char const *name, eRenderType type, uint count, bool normalized, size_t offset)
{
	std::string handleName(name);
	m_handle = handleName;
	m_type = type;
	m_elementCount = (int)count;
	m_memberOffset = (int)offset;
	m_isNormalized = normalized;
}

VertexLayout::VertexLayout(size_t stride, VertexAttribute const *layout, int attributeNum)
{
	m_stride = (int)stride;
	m_attributeNum = attributeNum;
	
	for (int attrCount = 0; attrCount < attributeNum; ++attrCount)
	{
		m_attributes.push_back(layout[attrCount]);
	}
}

VertexAttribute const Vertex_3DPCU::s_attributes[] = {
	VertexAttribute( "POSITION",    RT_FLOAT,           3, false, offsetof(Vertex_3DPCU, m_pos) ),
	VertexAttribute( "COLOR",       RT_UNSIGNED_BYTE,   4, true,  offsetof(Vertex_3DPCU, m_color) ),
	VertexAttribute( "UV",          RT_FLOAT,           2, false, offsetof(Vertex_3DPCU, m_uv) )
	//VertexAttribute()
};

VertexLayout const Vertex_3DPCU::s_layout = VertexLayout( sizeof(Vertex_3DPCU), 
	Vertex_3DPCU::s_attributes, 3 ); 

VertexAttribute const VertexLit::s_attributes[] = {
	VertexAttribute("POSITION",		RT_FLOAT,			3, false,	offsetof(VertexLit, m_pos)),
	VertexAttribute("COLOR",		RT_UNSIGNED_BYTE,	4, true,	offsetof(VertexLit, m_color)),
	VertexAttribute( "UV",          RT_FLOAT,           2, false,	offsetof(VertexLit, m_uv)),
	VertexAttribute( "NORMAL",      RT_FLOAT,           3, false,	offsetof(VertexLit, m_normal)),
	VertexAttribute( "TANGENT",     RT_FLOAT,           4, false,	offsetof(VertexLit, m_tangent))
	//VertexAttribute()
};

VertexLayout const VertexLit::s_layout = VertexLayout( sizeof(VertexLit), 
	VertexLit::s_attributes, 5 ); 