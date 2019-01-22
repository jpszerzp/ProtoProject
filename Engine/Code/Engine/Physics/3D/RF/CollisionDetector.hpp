#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

class CollisionSensor
{
public:
	static uint SphereVsSphere(const CollisionSphere& s1, const CollisionSphere& s2, CollisionKeep* c_data);
};