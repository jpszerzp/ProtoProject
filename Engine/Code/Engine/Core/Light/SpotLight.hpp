#pragma once

#include "Engine/Core/Light/Light.hpp"

class SpotLight : public Light
{
public:
	SpotLight(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba color, Vector3 attenuation, Vector3 specAttenuation, Vector3 direction,
		float innerAngle, float outerAngle, float directionFac);
	SpotLight(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Rgba& color,
		const Vector3& direction);
	~SpotLight();

public:
	void Update(float deltaTime) override;
	void Render(Renderer* renderer) override;
};