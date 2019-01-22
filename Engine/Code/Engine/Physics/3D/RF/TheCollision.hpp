#pragma once

#include "Engine/Physics/3D/RF/CollisionEntity.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Collision
{
public:
	CollisionRigidBody* m_bodies[2];

	float m_friction = 0.f;

	float m_restitution = 0.f;

	Vector3 m_pos;

	Vector3 m_normal;

	float m_penetration;

	Matrix33 m_to_world;

	Vector3 m_closing_vel;

	float m_desired_vel;

	Vector3 m_relative_pos[2];

public:
	void SetCollisionNormalWorld(const Vector3& normal) { m_normal = normal; }
	void SetCollisionPtWorld(const Vector3& pt) { m_pos = pt; }
	void SetPenetration(const float& penetration) { m_penetration = penetration; }
	void SetBodies(CollisionRigidBody* first, CollisionRigidBody* second);
	void SetFriction(const float& friction) { m_friction = friction; }
	void SetRestitution(const float& restitution) { m_restitution = restitution; }

	void PrepareInternal(float duration);

	void SwapBodies();

	void CalculateContactBasis();
	Vector3 CalculateLocalVelocity(uint idx, float duration);
	void CalculateDesiredDeltaVelocity(float duration);

	void CheckAwakeState();

	float GetPenetration() const { return m_penetration; }

	void ApplyPositionChange(Vector3 linear[2], Vector3 angular[2], float penetration);
	void ApplyVelocityChange(Vector3 linear[2], Vector3 angular[2]);

	Vector3 CalculateFrictionlessImpulse(Matrix33* iit);
	Vector3 CalculateFrictionImpulse(Matrix33* iit);
};