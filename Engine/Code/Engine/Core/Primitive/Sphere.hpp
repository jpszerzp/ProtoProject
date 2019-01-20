#pragma once

#include "Engine/Core/GameObject.hpp"

class Sphere : public GameObject
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;
	void UpdateWithSetPos(const Vector3& pos) override;

	Sphere();
	Sphere(const float& mass, const Vector3& pos, const Vector3& rot, const Vector3& scale, const Rgba& tint,
		std::string meshName, std::string materialName, 
		eMoveStatus moveStat,
		eBodyIdentity bid,
		bool multipass = false,
		eDepthCompare compare = COMPARE_LESS,
		eCullMode cull = CULLMODE_BACK,
		eWindOrder = WIND_COUNTER_CLOCKWISE,
		eDynamicScheme = DISCRETE);
	~Sphere();

	// RF
	Sphere(const float& mass, const Vector3& pos, const Vector3& rot, const Vector3& scale, const Rgba& tint);

	float GetRadius() const;
};