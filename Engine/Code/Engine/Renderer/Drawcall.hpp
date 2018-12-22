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
	Material*	m_material;

	Mesh*		m_mesh;
	Shader*		m_shader;
	Matrix44	m_model;
	Vector4		m_tint;
	// int m_passIdx;

	// single light
	Vector3 m_light_mat_ambient;
	Vector3 m_light_mat_diff;
	Vector3 m_light_mat_spec;

	uint m_lightCount;
	int m_lightIndices[MAX_LIGHTS];

	int GetLayer();
	eRenderQueue GetQueue();
};