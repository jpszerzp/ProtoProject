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

class SpringRigidForceGenerator : public RigidForceGenerator
{
public:
	Vector3 m_myConnect;
	Vector3 m_otherConnect;
	Rigidbody3* m_other;
	float m_const;
	float m_restLength;

public:
	SpringRigidForceGenerator(const Vector3& connect, Rigidbody3* other,
		const Vector3& otherConnect, float const_coef, float restLength)
		: m_myConnect(connect), m_otherConnect(otherConnect), m_other(other),
		m_const(const_coef), m_restLength(restLength){}

	virtual void UpdateForce(Rigidbody3* body, float deltaTime) override;
};