#pragma once

#include "Engine/Core/GameObject.hpp"

class Cone : public GameObject
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	Cone(){}
	Cone(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint, std::string meshName,
		std::string materialName, eMoveStatus moveStat, eBodyIdentity bid, bool multipass = false,
		eDepthCompare compare = COMPARE_LESS, eCullMode cull = CULLMODE_BACK, eWindOrder wind = WIND_COUNTER_CLOCKWISE);
	~Cone();
};