#pragma once

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
//#include "Engine/Renderer/Submesh.hpp"

class Drawcall
{
public:
	Drawcall();
	~Drawcall();

	//Submesh* m_submesh;
	Mesh*		m_mesh;
	Material*	m_material;
	Matrix44	m_model;
	Vector4		m_tint;
	// int m_passIdx;

	uint m_lightCount;
	int m_lightIndices[MAX_LIGHTS];

	int GetLayer();
	eRenderQueue GetQueue();
};