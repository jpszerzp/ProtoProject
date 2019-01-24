#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

class CollisionSensor
{
public:
	static uint SphereVsSphere(const CollisionSphere& s1, const CollisionSphere& s2, CollisionKeep* c_data);

	static uint BoxVsBox(const CollisionBox& b1, const CollisionBox& b2, CollisionKeep* c_data);

	static uint SphereVsPlane(const CollisionSphere& sphere, const CollisionPlane& plane, CollisionKeep* c_data);

	static uint BoxVsHalfPlane(const CollisionBox& box, const CollisionPlane& plane, CollisionKeep* c_data);

	static uint BoxVsSphere(const CollisionBox& box, const CollisionSphere& sphere, CollisionKeep* c_data);
};