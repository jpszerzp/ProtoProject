#pragma once

#include "Engine/Physics/3D/ForceGenerator.hpp"
#include "Engine/Physics/3D/RigidForceGenerator.hpp"

struct ParticleForceRegistration
{
	Entity3* m_entity3;
	ForceGenerator* m_generator;

	ParticleForceRegistration(Entity3* e, ForceGenerator* fg)
		: m_entity3(e), m_generator(fg){}
	~ParticleForceRegistration(){}
};

struct RigidbodyForceRegistration
{
	Rigidbody3* m_rb3;
	RigidForceGenerator* m_rigidGenerator;

	RigidbodyForceRegistration(Rigidbody3* rb, RigidForceGenerator* rfg)
		: m_rb3(rb), m_rigidGenerator(rfg){}
	~RigidbodyForceRegistration(){}
};

class ParticleForceRegistry
{
public:
	std::vector<ParticleForceRegistration*> m_registrations;

public:
	void Register(Entity3* e, ForceGenerator* generator);
	void Unregister(Entity3* e, ForceGenerator* generator);
	void ClearAllRegistries();
	bool NavigateRegistry(Entity3* e, ForceGenerator* generator);

	void UpdateForces(float deltaTime);
};

class RigidForceRegistry
{
public:
	std::vector<RigidbodyForceRegistration*> m_registrations;

public:
	void Register(Rigidbody3* rb, RigidForceGenerator* rbGenerator);

	void UpdateForces(float deltaTime);
};