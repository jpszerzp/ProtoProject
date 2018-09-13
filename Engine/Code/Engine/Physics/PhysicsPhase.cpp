#include "Engine/Physics/PhysicsPhase.hpp"
#include "Engine/Math/MathUtils.hpp"

bool BroadPhaseCirclePrune(const Disc2& disc1, const Disc2& disc2)
{
	bool overlapped = DoDiscsOverlap(disc1, disc2);
	return overlapped;
}

bool BroadPhaseCirclePrune(const Entity& entity1, const Entity& entity2)
{
	const Disc2& d1 = entity1.m_boundCircle;
	const Disc2& d2 = entity2.m_boundCircle;

	return BroadPhaseCirclePrune(d1, d2);
}

bool AscendBoundAABBMinX(const Entity& e1, const Entity& e2)
{
	AABB2 boundAABB1 = e1.m_boundAABB;
	AABB2 boundAABB2 = e2.m_boundAABB;

	if (boundAABB1.mins.x < boundAABB2.mins.x)
	{
		return true;
	}

	return false;
}

bool DescendBoundAABBMinX(const Entity& e1, const Entity& e2)
{
	AABB2 boundAABB1 = e1.m_boundAABB;
	AABB2 boundAABB2 = e2.m_boundAABB;

	if (boundAABB1.mins.x > boundAABB2.mins.x)
	{
		return true;
	}

	return false;
}

bool AscendBoundAABBMinY(const Entity& e1, const Entity& e2)
{
	AABB2 boundAABB1 = e1.m_boundAABB;
	AABB2 boundAABB2 = e2.m_boundAABB;

	if (boundAABB1.mins.y < boundAABB2.mins.y)
	{
		return true;
	}

	return false;
}

bool DescendBoundAABBMinY(const Entity& e1, const Entity& e2)
{
	AABB2 boundAABB1 = e1.m_boundAABB;
	AABB2 boundAABB2 = e2.m_boundAABB;

	if (boundAABB1.mins.y > boundAABB2.mins.y)
	{
		return true;
	}

	return false;
}

