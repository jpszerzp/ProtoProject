/*
#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/PHYSX/PhysXShapeRender.hpp"

class PhysXObject
{
private:
	//std::vector<PhysXShapeRender> m_shape_renders;

	PxRigidActor* m_ra;

public:
	PhysXObject(PxRigidActor* ra);
	~PhysXObject();

	// we do not need update for this object type,
	// becuz the physics scene has handled that
	// we just need to keep the rigid actor so that the updated data can be retrieved
	//void Update(float dt);

	//void Render(Renderer* rdr);

	PxRigidActor* GetRigidActor() const { return m_ra; }

	// sync data from actor
	//void CacheData();

	void RenderActor(Renderer* rdr);
};
*/