#include "Engine/Renderer/Renderable.hpp"

Renderable::Renderable(const Material* mat, Mesh* mesh, Transform& transform, Vector4 tint)
{
	m_material = nullptr;
	m_sharedMat = mat;
	m_mesh = mesh;
	m_transform = transform;
	m_tint = tint;
}

Renderable::~Renderable()
{
	delete m_material;
	m_material = nullptr;

	if (m_mesh->m_immediate)
	{
		delete m_mesh;
	}
	m_mesh = nullptr;
}

Material* Renderable::GetMaterial()
{
	if (m_material == nullptr)
	{
		m_material = m_sharedMat->Clone();
	}

	return m_material;
}

