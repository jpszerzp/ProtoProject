#pragma once

#include "Engine/Physics/3D/Rigidbody3.hpp"

class SphereRB3 : public Rigidbody3
{
public:
	Sphere3 m_primitive;

public:
	SphereRB3();
	SphereRB3(float mass, Sphere3 primitive, eMoveStatus moveStat);
	~SphereRB3();

	void SetEntityForPrimitive() override;

	void UpdateEntitiesTransforms() override;
	void UpdateInput(float deltaTime) override;
	void Integrate(float deltaTime) override;

	const Sphere3& GetSpherePrimitive() const { return m_primitive; }
};