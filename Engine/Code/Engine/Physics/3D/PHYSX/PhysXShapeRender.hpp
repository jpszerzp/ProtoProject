/*
#pragma once

#include "Engine/Renderer/Renderer.hpp"

class PhysXShapeRender
{
private:
	Mesh* m_mesh;
	Shader* m_shader;
	Texture* m_tex;
	Vector4 m_tint;
	PxMat44 m_model_matrix;

	PxGeometryHolder m_geo;

public:
	PhysXShapeRender(const PxGeometryHolder& h, const PxMat44& mm);
	~PhysXShapeRender();

	void Render(Renderer* rdr);
};
*/