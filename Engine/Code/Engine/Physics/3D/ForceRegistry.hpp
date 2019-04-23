#pragma once

#include "Engine/Physics/3D/ForceGenerator.hpp"
#include "Engine/Physics/3D/RigidForceGenerator.hpp"
#include "Engine/Physics/3D/RF/CollisionEntity.hpp"

struct ParticleForceRegistration
{
	// ...rigidbody could still be particle (collisionpoint)
	CollisionRigidBody* m_rb3;
	ForceGenerator* m_generator;

	ParticleForceRegistration(CollisionRigidBody* e, ForceGenerator* fg)
		: m_rb3(e), m_generator(fg){}
	~ParticleForceRegistration(){}
};

struct RigidbodyForceRegistration
{
	CollisionRigidBody* m_rb3;
	RigidForceGenerator* m_rigidGenerator;

	RigidbodyForceRegistration(CollisionRigidBody* rb, RigidForceGenerator* rfg)
		: m_rb3(rb), m_rigidGenerator(rfg){}

	~RigidbodyForceRegistration(){}
};

class ParticleForceRegistry
{
public:
	std::vector<ParticleForceRegistration*> m_registrations;

public:
	void Register(CollisionRigidBody* e, ForceGenerator* generator);
	void Unregister(CollisionRigidBody* e, ForceGenerator* generator);

	void ClearAllRegistries();
	bool NavigateRegistry(CollisionRigidBody* e, ForceGenerator* generator);

	void UpdateForces(float deltaTime);
};

class RigidForceRegistry
{
public:
	std::vector<RigidbodyForceRegistration*> m_registrations;

public:
	void Register(CollisionRigidBody* rb, RigidForceGenerator* rbGenerator);

	void ClearAllRegistries();

	void UpdateForces(float deltaTime);
};