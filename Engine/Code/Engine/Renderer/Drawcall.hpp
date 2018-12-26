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

	Texture*	m_diff;
	Texture*	m_spec;
	// int m_passIdx;

	// single light
	Vector3 m_light_mat_ambient;
	Vector3 m_light_mat_diff;
	Vector3 m_light_mat_spec;
	Vector3 m_light_pos;
	Vector3 m_light_dir;
	float m_light_atten_const;
	float m_light_atten_linear;
	float m_light_atten_quad;
	float m_cutoff;
	float m_outerCutoff;

	std::vector<Light*> m_lights;

	uint m_lightCount;
	int m_lightIndices[MAX_LIGHTS];

	int GetLayer();
	eRenderQueue GetQueue();
};