#pragma once
#include "Engine/Physics/3D/Rigidbody3.hpp"

class RigidForceGenerator
{
public:
	virtual void UpdateForce(Rigidbody3* body, float deltaTime) = 0;
};

class GravityRigidForceGenerator : public RigidForceGenerator
{
public:
	Vector3 m_gravity;

public:
	GravityRigidForceGenerator(const Vector3& gravity)
		: m_gravity(gravity){}

	virtual void UpdateForce(Rigidbody3* body, float deltaTime) override;
};

class AnchorSpringRigidForceGenerator : public RigidForceGenerator
{
public:
	Vector3 m_anchorWorld;
	Vector3 m_attachLocal;
	Rigidbody3* m_other;
	float m_const;
	float m_restLength;

public:
	AnchorSpringRigidForceGenerator(const Vector3& anchorWorld, Rigidbody3* other,
		const Vector3& attachLocal, float const_coef, float restLength)
		: m_anchorWorld(anchorWorld), m_attachLocal(attachLocal), m_other(other),
		m_const(const_coef), m_restLength(restLength){}

	virtual void UpdateForce(Rigidbody3* body, float deltaTime) override;
};