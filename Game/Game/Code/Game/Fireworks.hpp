#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Primitive/Point.hpp"

class Fireworks : public Point
{
public:
	bool m_lastRound = false;
	float m_age;
	Vector3 m_minVel;
	Vector3 m_maxVel;

public:
	Fireworks(Vector3 pos);
	~Fireworks();

	void Update(float deltaTime) override;
	void Render(Renderer* renderer) override;
	void Configure(float age, Vector3 inheritVel, Vector3 maxVel, Vector3 minVel, bool lastRound);
};