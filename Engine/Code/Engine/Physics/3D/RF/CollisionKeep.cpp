#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

bool CollisionKeep::AllowMoreCollision()
{
	return m_collision_left > 0;
}

void CollisionKeep::Reset(uint contacts)
{
	m_collision_left = contacts;
	m_collision_count = 0;
	m_collision = m_collision_head;		// back to head of array
}

void CollisionKeep::NotifyAddedCollisions(uint count)
{
	m_collision_left -= count;
	m_collision_count += count;

	m_collision += count;
}

