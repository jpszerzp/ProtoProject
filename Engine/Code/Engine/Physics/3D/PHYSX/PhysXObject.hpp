#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Quaternion.hpp"

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
	Vector3 GetPos() const;

	void SetShouldDelete(bool value) { m_delete = value; }
	void SetPos(const Vector3& pos);
	void SetOrient(const Quaternion& orient);
	void SetPosAndOrient(const Vector3& pos, const Quaternion& orient);
	void SetLinearVel(const Vector3& lin_vel);
	void SetAngularVel(const Vector3& ang_vel);

	void RenderActor(Renderer* rdr);

	static std::string ChooseMesh(const PxGeometryHolder& h);
	static void ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn);
	static void ChooseTexture(const bool& is_sleep, const bool& is_trigger, std::string& tn);

	void DisableGravity();
};