#pragma once

#include "Engine/Renderer/Renderer.hpp"

#include "PxPhysicsAPI.h"
using namespace physx;

class PhysXObject
{
	PxRigidActor* m_ra;

	bool m_delete = false;

public:
	PhysXObject(){}
	PhysXObject(PxRigidActor* ra);
	~PhysXObject();

	PxRigidActor* GetRigidActor() const { return m_ra; }
	bool ShouldDelete() const { return m_delete; }

	void SetShouldDelete(bool value) { m_delete = value; }

	void RenderActor(Renderer* rdr);

	static std::string ChooseMesh(const PxGeometryHolder& h);
	static void ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn);
	static void ChooseTexture(const bool& is_sleep, const bool& is_trigger, std::string& tn);
};