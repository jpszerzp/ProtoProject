#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/Particle.hpp"

class PointEntity3 : public Entity3
{
public:
	Particle m_primitive;

public:
	PointEntity3(const Particle& primitive, eMoveStatus moveStat);
	~PointEntity3();

	void UpdatePrimitives() override;
	void Render(Renderer* renderer) override;

	void Translate(Vector3 translation) override;

	Particle GetParticlePrimitive() { return m_primitive; }

	void SetEntityForPrimitive() override;
};