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

	void UpdateEntityPrimitive() override;
	void UpdateInput(float deltaTime) override;
	void UpdateEntitiesTransforms() override;
	void Integrate(float deltaTime) override;

	const OBB3& GetOBBPrimitive() const { return m_primitive; }
};