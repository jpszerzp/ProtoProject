#pragma once

#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Math/Plane.hpp"

class QuadRB3 : public Rigidbody3
{
public:
	Plane m_primitive;

public:
	QuadRB3(){}
	QuadRB3(float mass, const Plane& primitive, 
		const Vector3& center, const Vector3& euler, const Vector3& scale,
		eMoveStatus moveStat);
	~QuadRB3(){}

	void SetEntityForPrimitive() override;

	void UpdatePrimitives() override;
	void UpdateTransforms() override;
	void Integrate(float deltaTime) override;

	Plane GetPlanePrimitive() const { return m_primitive; }
};