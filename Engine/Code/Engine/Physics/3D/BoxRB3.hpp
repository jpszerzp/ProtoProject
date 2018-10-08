#pragma once

#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Math/OBB3.hpp"

class BoxRB3 : public Rigidbody3
{
public:
	OBB3 m_primitive;

public:
	BoxRB3(){}
	BoxRB3(float mass, const OBB3& primitive, const Vector3& euler, eMoveStatus status);
	~BoxRB3(){}

	void SetEntityForPrimitive() override;

	void UpdatePrimitives() override;
	void UpdateTransforms() override;
	void UpdateInput(float deltaTime) override;
	void Integrate(float deltaTime) override;

	OBB3 GetBoxPrimitive() { return m_primitive; }
};