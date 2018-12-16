#pragma once

#include "Engine/Core/GameObject.hpp"

class Box : public GameObject
{
public:
	void Update(float deltaTime) override;

	Box();
	Box(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint,
		std::string meshName, std::string matName, 
		eMoveStatus moveStat, eBodyIdentity bid,
		bool multipass = false, 
		eDepthCompare compare = COMPARE_LESS,
		eCullMode cull = CULLMODE_BACK,
		eWindOrder order = WIND_COUNTER_CLOCKWISE);
	~Box();
};