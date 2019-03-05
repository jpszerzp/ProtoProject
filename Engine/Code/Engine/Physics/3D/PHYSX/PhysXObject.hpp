#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/PHYSX/PhysXShapeRender.hpp"

class PhysXObject
{
private:
	PxRigidActor* m_ra;
	//PxActor* m_ra;
	
	bool m_should_delete = false;

public:
	PhysXObject(PxRigidActor* ra);
	//PhysXObject(PxActor* ra);
	~PhysXObject();

	PxRigidActor* GetRigidActor() const { return m_ra; }
	//PxActor* GetRigidActor() const { return m_ra; }
	bool ShouldDelete() const { return m_should_delete; }

	void SetShouldDelete(const bool& value) { m_should_delete = value; }

	void RenderActor(Renderer* rdr);
	
	static std::string ChooseMesh(const PxGeometryHolder& h);
	static void ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn);
	static void ChooseTexture(const bool& is_sleep, const bool& is_trigger, std::string& tn);
};
