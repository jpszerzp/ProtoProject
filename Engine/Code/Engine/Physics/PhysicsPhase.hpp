#pragma once

#include "Engine/Math/Disc2.hpp"
#include "Engine/Physics/2D/Entity.hpp"


enum eSAPAxis
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
	AXIS_NUM
};

// broad phase
bool BroadPhaseCirclePrune(const Disc2& disc1, const Disc2& disc2);
bool BroadPhaseCirclePrune(const Entity& entity1, const Entity& entity2);

// SAP
bool AscendBoundAABBMinX(const Entity& e1, const Entity& e2);
bool DescendBoundAABBMinX(const Entity& e1, const Entity& e2);
bool AscendBoundAABBMinY(const Entity& e1, const Entity& e2);
bool DescendBoundAABBMinY(const Entity& e1, const Entity& e2);
