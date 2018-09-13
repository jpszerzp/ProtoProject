#pragma once

#include "Engine/Core/Primitive/Quad.hpp"

class WaterSurface : public Quad
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	WaterSurface();
	~WaterSurface();

	Texture* m_waterTex;
	Mesh*	 m_surfaceMesh;
};