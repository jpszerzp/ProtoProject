/*
#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/PHYSX/PhysXShapeRender.hpp"
#include "Engine/Core/Quaternion.hpp"
#include "Engine/Math/Vector3.hpp"

class PhysXObject
{
private:
	PxRigidActor* m_ra;
	
	bool m_should_delete = false;

public:
	PhysXObject(PxRigidActor* ra);
	~PhysXObject();

	PxRigidActor* GetRigidActor() const { return m_ra; }
	bool ShouldDelete() const { return m_should_delete; }
	Vector3 GetPos() const;

	void SetShouldDelete(const bool& value) { m_should_delete = value; }
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
*/
