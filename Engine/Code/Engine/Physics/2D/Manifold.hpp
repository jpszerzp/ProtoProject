#pragma once

#include "Engine/Physics/2D/Entity.hpp"

class Manifold
{
public:
	Entity* m_e1;
	Entity* m_e2;
	float   m_penetration;
	Vector2 m_normal;
	Vector2 m_start;
	Vector2 m_end;

public:
	Manifold(Entity* e1, Entity* e2);
	~Manifold();
};

bool AABB2VsAABB2Manifold(Manifold* manifold);
bool AABB2VsDisc2Manifold(Manifold* manifold);

bool DiscVsDiscManifold(Manifold* manifold);
bool OBB2VsDiscManifold(Manifold* manifold);
bool OBB2VsOBB2Manifold(Manifold* manifold);