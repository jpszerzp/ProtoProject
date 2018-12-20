#pragma once

#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/Transform.hpp"

class Renderable
{
public:
	Material*		m_material;
	const Material* m_sharedMat;

	Shader*			m_non_mat_shader;
	Mesh*			m_mesh;
	Transform		m_transform;
	Vector4			m_tint;

	bool			m_delete = false;

public:
	Renderable(const Material* mat, Mesh* mesh, Transform& transform, Vector4 tint);
	Renderable(Shader* shader, Mesh* mesh, const Transform& transform, const Vector4& tint);
	~Renderable();

	void SetMesh(Mesh* mesh) { m_mesh = mesh; }
	void SetMaterial(Material* material) { m_material = material; }

	Material*		GetMaterial();
	const Material*	GetSharedMaterial() { return m_sharedMat; };
	Mesh*			GetMesh() { return m_mesh; }
	Vector4			GetTint() const {return m_tint;}
	Shader*			GetShader() { return m_non_mat_shader; }
};