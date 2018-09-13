#pragma once

#include "Engine/Core/Light/Light.hpp"

class PointLight : public Light
{
public:
	PointLight(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba color, Vector3 attenuation, Vector3 specAttenuation, Vector3 direction,
		float innerAngle, float outerAngle, float directionFac);
	~PointLight();

public:
	void Update(float deltaTime) override;
	void Render(Renderer* renderer) override;
};