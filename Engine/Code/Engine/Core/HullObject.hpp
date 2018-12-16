#pragma once

#include "Engine/Core/GameObject.hpp"
#include "Engine/Math/QuickHull.hpp"

class HullObject : public GameObject
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	void SetHullObject() { m_hull->m_go_ref = this; }

	HullObject(){}
	HullObject(const Vector3& pos, const Vector3& ext, const Vector3& rot, const Vector3& scale, const Rgba& tint,
		std::string matName, eMoveStatus moveStat, eBodyIdentity bid, int sampleCount, std::string fileName = "", bool multipass = false,
		eDepthCompare compare = COMPARE_LESS, eCullMode cull = CULLMODE_BACK, eWindOrder wind = WIND_COUNTER_CLOCKWISE);
	~HullObject();

private:
	QuickHull* m_hull;
};