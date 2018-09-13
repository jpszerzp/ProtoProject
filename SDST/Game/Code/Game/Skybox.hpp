#pragma once

#include "Engine/Core/Primitive/Cube.hpp"

class Skybox : public Cube
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	Skybox();
	~Skybox();

	TextureCube* m_skybox;
	Mesh*		 m_boxMesh;
};