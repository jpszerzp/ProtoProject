#pragma once

#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Physics/MassData.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Physics/3D/BoundingVolume3.hpp"
#include "Engine/Math/Sphere3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Quaternion.hpp"

enum eVerletScheme
{
	BASIC_VERLET,
	VELOCITY_VERLET
};

class GameObject;

class Entity3
{
protected:
	Mesh* m_colliderMesh = nullptr;
	Mesh* m_sphereBoundMesh = nullptr;
	Mesh* m_boxBoundMesh = nullptr;
	
	// need updated
	Transform m_entityTransform;
	Transform m_sphereBoundTransform;
	Transform m_boxBoundTransform;

	bool m_const = false;

	// p 
	Vector3 m_center;

	// p'
	Vector3 m_linearVelocity;

	// p'': force and acc
	Vector3 m_linearAcceleration;
	Vector3 m_netforce = Vector3::ZERO;
	bool m_forcePersistent = false;			// by default force is NOT persistent

	// mass and phys mat
	MassData m_massData;
	PhysicsMaterial m_physMat;

	// color
	Rgba m_color;
	Rgba m_constColor = Rgba::GREY;
	Rgba m_nonConstColor = Rgba::WHITE;

	bool m_drawBoundSphere = false;
	bool m_drawBoundBox = false;
	
	// need updated
	//Sphere3 m_boundSphere;
	BoundingSphere m_boundSphere;
	AABB3 m_boundBox;

	GameObject* m_goRef = nullptr;

	bool m_passedBroadPhase = false;

	bool m_considerDamp = true;		// by default, consider damping
	float m_linearDamp = 0.f;

	// verlet - basic
	bool m_verlet = false;
	eVerletScheme m_verlet_scheme = BASIC_VERLET;
	Vector3 m_lastCenter;
	// verlet - velocity
	Vector3 m_halfStepVelocity;

public:
	void SetDrawBoundingSphere(bool value) { m_drawBoundSphere = value; }
	void SetDrawBoundingBox(bool value) { m_drawBoundBox = value; }
	void SetBoundSpherePos(Vector3 pos);
	void SetBoundBoxPos(Vector3 pos);
	void SetGameobject(GameObject* go) { m_goRef = go; }
	void SetMass(float mass);
	void SetLinearVelocity(float vel_x, float vel_y, float vel_z);
	void SetLinearVelocity(Vector3 vel) { m_linearVelocity = vel; }
	void SetLinearAcceleration(float acc_x, float acc_y, float acc_z);
	void SetLinearAcceleration(Vector3 acc) { m_linearAcceleration = acc; }
	void SetDamping(float damp) { m_linearDamp = damp; }
	void SetEntityCenter(Vector3 center) { m_center = center; }
	void SetEntityLastCenter(Vector3 last_center) { m_lastCenter = last_center; }
	void SetNetForce(float f_x, float f_y, float f_z);
	void SetNetForce(Vector3 force) { m_netforce = force; }
	void SetConsiderDamp(bool value) { m_considerDamp = value; }
	void SetNetForcePersistent(bool value) { m_forcePersistent = value; }
	void SetVerlet(bool value) { m_verlet = value; }
	void SetVerletScheme(eVerletScheme scheme) { m_verlet_scheme = scheme; }
	virtual void SetEntityForPrimitive();

	bool HasInfiniteMass() const;
	bool IsConst() const { return m_const; }
	bool GetDrawBoundingSphere() const { return m_drawBoundSphere; }
	bool GetDrawBoundingBox() const { return m_drawBoundBox; }
	Vector3 GetEntityCenter() const { return m_center; }
	Transform GetEntityTransform() const { return m_entityTransform; }
	GameObject* GetGameobject() const { return m_goRef; }
	MassData GetMassData() const { return m_massData; }
	float GetMass() const { return GetMassData().m_mass; }
	Vector3 GetLinearVelocity() const { return m_linearVelocity; }
	Vector3 GetLinearAcceleration() const { return m_linearAcceleration; }

	void UpdateBoundPrimitives();
	void UpdateEntitiesTransforms();
	virtual void UpdateEntityPrimitive();
	virtual void Update(float deltaTime);
	virtual void Render(Renderer* renderer);
	virtual void Translate(Vector3 translation);

	void VerletIntegrate(float deltaTime);
	void EulerIntegrate(float deltaTime);
	virtual void Integrate(float deltaTime);
	void PositionIntegrate(float deltaTime);
	void VelocityIntegrate(float deltaTime);
	void ClearForce() { SetNetForce(Vector3::ZERO); }
	virtual void AddForce(Vector3 force) { m_netforce += force; }	// all forces need to be added/updated before integration step
};