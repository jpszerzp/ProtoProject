#pragma once

#include "Engine/Core/GameObject.hpp"

class Point : public GameObject
{
public:
	virtual void Render(Renderer* renderer) override;
	virtual void Update(float deltaTime) override;
	virtual void UpdateInput(float) override;

	void ObjectDrivePosition(Vector3 pos) override;

	Point();
	Point(Vector3 pos, Vector3 rot, float size, Rgba tint,
		std::string meshName, std::string materialName,
		eMoveStatus moveStat,
		eBodyIdentity bid,
		bool multiPass = false,
		eCompare compare = COMPARE_LESS,
		eCullMode cull = CULLMODE_BACK,
		eWindOrder order = WIND_COUNTER_CLOCKWISE);
	~Point();
};