#pragma once

#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Physics/3D/ContactResolver.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Cube.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Game/GameState.hpp"
#include "Game/Ballistics.hpp"
#include "Game/Fireworks.hpp"
#include "Game/Spring.hpp"
#include "Game/Links.hpp"

class Physics3State : public GameState
{
public:
	Physics3State();
	~Physics3State();

	Sphere* InitializePhysSphere(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);
	Cube* InitializePhysCube(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);
	Point* InitializePhysPoint(Vector3 pos, Vector3 rot, float size, 
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);
	Quad* InitializePhysQuad(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);

	//Sphere* InitializeRigidSphere(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint);
	
	Fireworks* SetupFireworks(float age, Vector3 pos, Vector3 inheritVel, Vector3 maxVel, Vector3 minVel, bool lastRound = false);
	Ballistics* SetupBallistics(eBallisticsType type, Vector3 pos, bool frozen, Rgba color);
	Spring* SetupSpring(Point* end1, Point* end2, float coef, float rl);
	AnchorSpring* SetupAnchorSpring(Point* end1, Point* end2, float coef, float rl);
	Rod* SetupRod(float length, Point* p1, Point* p2);

	// DEPRCATED
	//void ParticlePhysicsUpdate(float deltaTime);
	//void RigidbodyPhysicsUpdate(float deltaTime);
	//void SingleContactUpdate();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override;

	// pipeline of update
	void UpdateInput(float deltaTime);
	void UpdateGameobjects(float deltaTime);
	void UpdateDebugDraw(float deltaTime);

	// update of GO
	void UpdateForceRegistry(float deltaTime);
	void UpdateGameobjectsCore(float deltaTime);
	void UpdateContactGeneration();
	void UpdateContactResolution(float deltaTime);

	void Render(Renderer* renderer) override;
	void RenderGameobjects(Renderer* renderer);

public:
	// entity
	GameObject* m_g0;
	GameObject* m_g1;
	GameObject* m_g2;
	GameObject* m_g3;

	// rigid body
	GameObject* m_r0;

	Ballistics* m_ballistics;
	Spring* m_spring;
	AnchorSpring* m_anchorSpring;
	Rod* m_rod;

	// for collision process
	std::vector<Sphere*> m_spheres;
	std::vector<Cube*>	 m_cubes;
	std::vector<Quad*>	 m_quads;
	std::vector<Point*>  m_points;

	ParticleForceRegistry* m_particleRegistry = nullptr;
	RigidForceRegistry* m_rigidRegistry = nullptr;

	//CollisionData3* m_collisionData;		// a storage of all contacts
	ContactResolver* m_allResolver;		
	ContactResolver* m_iterResolver;
};