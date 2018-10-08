#pragma once

#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Physics/MassData.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Physics/3D/BoundingVolume3.hpp"
#include "Engine/Math/Sphere3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Primitive3.hpp"
#include "Engine/Core/Quaternion.hpp"

enum eVerletScheme
{
	BASIC_VERLET,
	VELOCITY_VERLET
};

enum eMoveStatus
{
	MOVE_STATIC, 
	MOVE_KINEMATIC,
	MOVE_DYNAMIC
};

enum eBodyIdentity
{
	BODY_PARTICLE,
	BODY_RIGID
};

class GameObject;

class Entity3
{
protected:
	Mesh* m_colliderMesh = nullptr;
	//Mesh* m_sphereBoundMesh = nullptr;
	Mesh* m_boxBoundMesh = nullptr;
	
	// need updated
	Transform m_entityTransform;
	//Transform m_sphereBoundTransform;
	Transform m_boxBoundTransform;

	// p 
	Vector3 m_center = Vector3::ZERO;

	// p'
	Vector3 m_linearVelocity = Vector3::ZERO;

	// p'': force and acc
	Vector3 m_linearAcceleration = Vector3::ZERO;
	Vector3 m_netforce = Vector3::ZERO;
	bool m_forcePersistent = false;			// by default force is NOT persistent

	// mass and phys mat
	MassData3 m_massData;
	PhysicsMaterial m_physMat;

	// color
	Rgba m_color;
	Rgba m_constColor = Rgba::GREY;
	Rgba m_nonConstColor = Rgba::WHITE;

	bool m_drawBoundSphere = false;
	bool m_drawBoundBox = false;
	
	// need updated
	BoundingSphere m_boundSphere;
	AABB3 m_boundBox;

	GameObject* m_goRef = nullptr;

	bool m_passedBroadPhase = false;

	bool m_considerDamp = true;		// by default, consider damping
	float m_linearDamp = 0.f;

	// verlet - basic
	bool m_verlet = false;
	eVerletScheme m_verlet_scheme = BASIC_VERLET;
	Vector3 m_lastCenter = Vector3::ZERO;
	// verlet - velocity
	Vector3 m_halfStepVelocity = Vector3::ZERO;

	eMoveStatus m_moveStatus = MOVE_DYNAMIC;	// constness
	eBodyIdentity m_bodyID = BODY_PARTICLE;		// this entity can be statically casted to specified type

	bool m_frozen = false;

	//Primitive3 m_primitive;

public:
	virtual ~Entity3(){}

	void SetDrawBoundingSphere(bool value) { m_drawBoundSphere = value; }
	void SetDrawBoundingBox(bool value) { m_drawBoundBox = value; }
	void SetBoundSpherePos(Vector3 pos);
	void SetBoundBoxPos(Vector3 pos);
	void SetGameobject(GameObject* go) { m_goRef = go; }
	void SetMass(float mass);
	void SetLinearVelocity(float vel_x, float vel_y, float vel_z);
	void SetLinearVelocity(Vector3 vel) { m_linearVelocity = vel; }
	void IncrementVelocity(Vector3 added) { m_linearVelocity += added; }
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
	void SetFrozen(bool value) { m_frozen = value; }
	void SetTransform(const Transform& t) { m_entityTransform = t; }
	virtual void SetEntityForPrimitive(){}

	bool HasInfiniteMass() const;
	bool IsConst() const { return (m_moveStatus == MOVE_STATIC); }
	bool GetDrawBoundingSphere() const { return m_drawBoundSphere; }
	bool GetDrawBoundingBox() const { return m_drawBoundBox; }
	Vector3 GetEntityCenter() const { return m_center; }
	Transform GetEntityTransform() const { return m_entityTransform; }
	GameObject* GetGameobject() const { return m_goRef; }
	MassData3 GetMassData3() const { return m_massData; }
	float GetMass3() const { return GetMassData3().m_mass; }
	Vector3 GetLinearVelocity() const { return m_linearVelocity; }
	Vector3 GetLinearAcceleration() const { return m_linearAcceleration; }
	eMoveStatus GetEntityMoveStatus() const { return m_moveStatus; }
	bool IsEntityStatic() const { return (GetEntityMoveStatus() == MOVE_STATIC); }
	bool IsEntityKinematic() const { return (GetEntityMoveStatus() == MOVE_KINEMATIC); }
	bool IsEntityDynamic() const { return (GetEntityMoveStatus() == MOVE_DYNAMIC); }
	bool IsFrozen() const { return m_frozen; }
	eBodyIdentity GetEntityBodyID() const { return m_bodyID; }
	BoundingSphere GetBoundingSphere() const { return m_boundSphere; }

	virtual void UpdatePrimitives(){}	// do not know which primitive this entity has, hence virtual
	virtual void UpdateTransforms();		// all transforms
	virtual void Update(float){}
	virtual void UpdateInput(float){}
	virtual void Render(Renderer*){}
	virtual void Translate(Vector3){}

	void VerletIntegrate(float deltaTime);
	void EulerIntegrate(float deltaTime);
	virtual void Integrate(float deltaTime);
	void PositionIntegrate(float deltaTime);
	void VelocityIntegrate(float deltaTime);
	void ClearForce() { SetNetForce(Vector3::ZERO); }
	void AddForce(Vector3 force) { m_netforce += force; }	// all forces need to be added/updated before integration step
};