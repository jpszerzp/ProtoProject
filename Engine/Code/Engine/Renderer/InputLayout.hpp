#pragma once

#include "Engine/Renderer/Mesh.hpp"

class InputLayout
{
public:
	Mesh* m_mesh;
	ShaderProgram* m_sp;

	GLint m_vaoid;
};