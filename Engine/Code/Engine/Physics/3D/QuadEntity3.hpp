#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/Plane.hpp"


class QuadEntity3 : public Entity3
{
public:
	Plane m_primitive; 

public:
	QuadEntity3(const Plane& primitive, eMoveStatus moveStat,
		Vector3 center, Vector3 rot, Vector3 scale);
	~QuadEntity3();

	void UpdatePrimitives() override;
	void Render(Renderer* renderer) override;
	void Translate(Vector3 translation) override;

	Plane GetPlanePrimitive() { return m_primitive; }

	void SetEntityForPrimitive() override;
};