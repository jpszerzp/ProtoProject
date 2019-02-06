#pragma once

#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Submesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"

class MeshBuilder 
{
public:
	void Begin(eDrawPrimitiveType prim, bool use_indices);
	void End();
	void SetColor( Rgba const &c );
	void SetUV( Vector2 const &uv );
	void SetNormal(Vector3 const &normal);
	void SetTangent(Vector4 const &tangent);
	Vector4 CalcTangent(Vector3 edge1, Vector3 edge2, Vector2 uv1, Vector2 uv2);
	uint PushVertex( Vector3 position );
	Mesh* CreateMesh(eVertexType vertType, eDrawPrimitiveType drawType);
	void AddTriangle(uint idx1, uint idx2, uint idx3);
	void AddQuad(uint idx1, uint idx2, uint idx3, uint idx4);
	void AddLine(uint idx1, uint idx2);
	void AddPoint(uint idx);
	int GetVertexCount() const;
	int GetIndexCount() const;

public:
	sVertexBuilder m_stamp;
	std::vector<sVertexBuilder> m_vertices;		
	std::vector<uint> m_indices;				

	sDrawInstruction m_draw; 
};