#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/PHYSX/PhysXShapeRender.hpp"

class PhysXObject
{
private:
	PxRigidActor* m_ra;

public:
	PhysXObject(PxRigidActor* ra);
	~PhysXObject();

	PxRigidActor* GetRigidActor() const { return m_ra; }

	void RenderActor(Renderer* rdr);
	
	static std::string ChooseMesh(const PxGeometryHolder& h);
	static void ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn);
};
