#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/Drawcall.hpp"

Renderable::Renderable(const Material* mat, Mesh* mesh, Transform& transform, Vector4 tint)
{
	m_material = nullptr;
	m_sharedMat = mat;
	m_mesh = mesh;
	m_transform = transform;
	m_tint = tint;
}

Renderable::Renderable(Shader* shader, Mesh* mesh, const Transform& transform, const Vector4& tint)
{
	m_non_mat_shader = shader;
	m_mesh = mesh;
	m_transform = transform;
	m_tint = tint;
}

Renderable::~Renderable()
{
	delete m_material;
	m_material = nullptr;

	if (m_mesh != nullptr)
	{
		if (m_mesh->m_immediate)
		{
			delete m_mesh;
		}
		m_mesh = nullptr;
	}
}

Material* Renderable::GetMaterial()
{
	if (m_material == nullptr)
	{
		m_material = m_sharedMat->Clone();
	}

	return m_material;
}

Drawcall* Renderable::ComposeDrawcall()
{
	Drawcall* dc = new Drawcall();

	dc->m_mesh = m_mesh;
	dc->m_model = m_transform.GetWorldMatrix();
	dc->m_shader = m_non_mat_shader;
	dc->m_tint = m_tint;

	dc->m_diff = m_diffuse_map;
	dc->m_spec = m_specular_map;

	return dc;
}

