#pragma once

#include "Engine/Physics/3D/ForceGenerator.hpp"
#include "Engine/Physics/3D/RigidForceGenerator.hpp"

struct ForceRegistration
{
	Entity3* m_entity3;
	ForceGenerator* m_generator;

	ForceRegistration(Entity3* e, ForceGenerator* fg)
		: m_entity3(e), m_generator(fg){}
	~ForceRegistration(){}
};

struct RigidbodyForceRegistration
{
	Entity3* m_entity3;
	RigidForceGenerator* m_rigidGenerator;

	RigidbodyForceRegistration(Entity3* e, RigidForceGenerator* rfg)
		: m_entity3(e), m_rigidGenerator(rfg){}
	~RigidbodyForceRegistration(){}
};

class ForceRegistry
{
public:
	std::vector<ForceRegistration*> m_registrations;

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
	void UpdateForces(float deltaTime);
};