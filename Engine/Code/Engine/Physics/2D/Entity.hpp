#pragma once

#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Physics/MassData.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Math/Disc2.hpp"

class PhysicsScene;

class Entity
{
public:
	PhysicsScene*	m_scene = nullptr;
	Mesh*			m_colliderMesh = nullptr;				// this mesh is based on entity shape 
	Mesh*			m_discBoundMesh = nullptr;				// quad mesh
	Mesh*			m_boxBoundMesh = nullptr;
	Transform		m_discBoundTransform;
	Transform		m_boxBoundTransform;

	// constness
	bool m_const = false;

	// p
	float    m_angleDestination;
	float    m_orientation;
	Vector2  m_center;

	// p'
	float	 m_signedRotationVel;
	Vector2  m_velocity;

	// p''
	float	 m_signedRotationAcc = 0.f;
	Vector2  m_acceleration;
	Vector2	 m_netForce = Vector2::ZERO;
	Vector3	 m_torque;

	// mass and physics material
	MassData		m_massData;
	PhysicsMaterial m_physicsMat;

	// color
	Rgba m_color;
	//Rgba m_cosmeticDiscColor = Rgba(255, 0, 255, 255);
	//Rgba m_physicsDiscColor = Rgba(0, 255, 255, 255);
	Rgba m_frictionColor = Rgba(0, 0, 255, 255);
	Rgba m_elasticityColor = Rgba(255, 0, 0, 255);
	Rgba m_constColor = Rgba::GREY;
	Rgba m_nonConstColor = Rgba::WHITE;

	bool	 m_drawBoundDisc = false;
	bool     m_drawBoundAABB = false;
	bool	 m_dead = false;
	//bool     m_drag = false;
	//bool     m_burst = false;
	//float    m_damping = 0.f;

	Disc2 m_boundCircle;
	AABB2 m_boundAABB;

	// debug visuals
	bool m_passedBroadphase = false;

public:
	Entity();
	~Entity();

	virtual void Update(float deltaSeconds);
	virtual void UpdateInput();
	virtual void UpdateBoundAABB();
	virtual void UpdateBoundDisc();

	void UpdateLinearPosition(float deltaSeconds);
	void UpdateLinearAcceleration();
	void UpdateLinearVelocity(float deltaSeconds);
	void UpdateAngularPosition(float deltaSeconds);
	void UpdateAngularAcceleration();
	void UpdateAngularVelocity(float deltaSeconds);
	void LimitSpeed(float limit);
	virtual void Integrate(float deltaSeconds);
	void ClearForce();

	virtual void Render(Renderer* renderer);

	/*
	// getters
	Rgba	GetColor() const {return m_color;}
	Vector2 GetCenter() const { return m_center; }

	// setters
	void SetCenter (Vector2 value) { m_center = value; }
	void SetVelocity(Vector2 value) { m_velocity = value; }
	*/

	void TranslateEntity(Vector2 translation);
};