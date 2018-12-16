#pragma once

#include "Engine/Core/GameObject.hpp"

class Quad : public GameObject
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	Quad();
	Quad(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint, std::string meshName,
		std::string materialName, Vector2 uvBL, Vector2 uvBR, Vector2 uvTL, Vector2 uvTR,
		eMoveStatus moveStat,
		eBodyIdentity bid,
		bool multipass = false, 
		eDepthCompare compare = COMPARE_LESS, 
		eCullMode cull = CULLMODE_BACK,
		eWindOrder order = WIND_COUNTER_CLOCKWISE,
		eDynamicScheme scheme = DISCRETE);
	Quad(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint, 
		std::string meshName, std::string materialName, 
		eMoveStatus moveStat,
		eBodyIdentity bid,
		bool multipass = false,
		eDepthCompare compare = COMPARE_LESS,
		eCullMode cull = CULLMODE_BACK,
		eWindOrder order = WIND_COUNTER_CLOCKWISE,
		eDynamicScheme scheme = DISCRETE);
	~Quad();
};