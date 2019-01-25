#pragma once

#include "Engine/Physics/3D/RF/CollisionEntity.hpp"
#include "Engine/Core/EngineCommon.hpp"

struct PhysMaterialData
{
	float m_friction = 0.f;
	float m_restitution = 0.f;
};

class Collision
{
public:
	CollisionRigidBody* m_bodies[2];

	PhysMaterialData m_mat;

	Vector3 m_pos;
	Vector3 m_normal;
	float m_penetration;

	Matrix33 m_to_world;
	Vector3 m_relative_pos[2];

	Vector3 m_closing_vel;
	float m_desired_vel;

public:
	void SetCollisionNormalWorld(const Vector3& normal) { m_normal = normal; }
	void SetCollisionPtWorld(const Vector3& pt) { m_pos = pt; }
	void SetPenetration(const float& penetration) { m_penetration = penetration; }
	void SetBodies(CollisionRigidBody* first, CollisionRigidBody* second);
	void SetFriction(const float& friction) { m_mat.m_friction = friction; }
	void SetRestitution(const float& restitution) { m_mat.m_restitution = restitution; }

	void CacheData(float deltaTime);

	void SwapRigidBodies();

	void ComputeContactCoord();
	void ComputeRelativePosWorldCoord();
	void ComputeClosingVelocityContactCoord(float deltaTime);
	Vector3 ComputeLocalVelocity(uint idx, float deltaTime);
	void ComputeDesiredDeltaVelocity(float deltaTime);

	void CheckAwake();

	float GetPenetration() const { return m_penetration; }

	void ApplyPositionChange(Vector3 linear[2], Vector3 angular[2], float penetration);
	void ApplyVelocityChange(Vector3 linear[2], Vector3 angular[2]);

	Vector3 ComputeFrictionlessImpulse(Matrix33* iit);
	Vector3 ComputeFrictionalImpulse(Matrix33* iit);
};