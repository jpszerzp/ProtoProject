#pragma once

#include "Engine/Renderer/Renderer.hpp"

#include "PxPhysicsAPI.h"
using namespace physx;

class PhysXObject
{
	PxRigidActor* m_ra;

public:
	PhysXObject(){}
	PhysXObject(PxRigidActor* ra);
	~PhysXObject();

	void RenderActor(Renderer* rdr);

	static std::string ChooseMesh(const PxGeometryHolder& h);
	static void ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn);
	static void ChooseTexture(const bool& is_sleep, const bool& is_trigger, std::string& tn);
};