#pragma once

#include "Engine/Physics/3D/RF/TheCollision.hpp"
#include "Engine/Core/EngineCommon.hpp"

class CollisionKeep
{
public:
	Collision* m_collision_head;

	Collision* m_collision;

	uint m_collision_left;

	uint m_collision_count;

	float m_global_friction = .9f;
	float m_global_restitution = .1f;
	float m_tolerance;

	bool AllowMoreCollision();

	void Reset(uint contacts);

	void NotifyAddedCollisions(uint count);
};