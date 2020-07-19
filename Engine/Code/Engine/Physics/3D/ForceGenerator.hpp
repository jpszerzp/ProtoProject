#pragma once

#include "Engine/Physics/3D/RF/CollisionEntity.hpp"
#include "Engine/Math/Vector3.hpp"

class ForceGenerator
{
public:
	virtual void UpdateForce(CollisionRigidBody* entity3D, float deltaTime) = 0;
};

class GravityGenerator : public ForceGenerator
{
public:
	// the gravity acceleration, not gravity force itself
	Vector3 m_gravity;

public:
	GravityGenerator(const Vector3& gravity)
		: m_gravity(gravity){}

	virtual void UpdateForce(CollisionRigidBody* entity3D, float deltaTime);
};

class DragGenerator : public ForceGenerator
{
public:
	float m_dragCoef1;
	float m_dragCoef2;

public:
	DragGenerator(float d1, float d2)
		: m_dragCoef1(d1), m_dragCoef2(d2){}

	virtual void UpdateForce(CollisionRigidBody* entity3D, float deltaTime);
};

class SpringGenerator : public ForceGenerator
{
public:
	CollisionRigidBody* m_other;
	float m_const;
	float m_restLength;

public:
	SpringGenerator(CollisionRigidBody* other, float const_coef, float restLength)
		: m_other(other), m_const(const_coef), m_restLength(restLength){}

	virtual void UpdateForce(CollisionRigidBody* entity3D, float deltaTime);
};

class AnchorSpringGenerator : public ForceGenerator
{
public:
	Vector3 m_anchor;
	float m_const;
	float m_restLength;

public:
	AnchorSpringGenerator(Vector3 anchor, float coef, float rl)
		: m_anchor(anchor), m_const(coef), m_restLength(rl) {}

	virtual void UpdateForce(CollisionRigidBody* entity3D, float deltaTime);
};