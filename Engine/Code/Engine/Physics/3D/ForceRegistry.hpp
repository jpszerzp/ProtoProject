#pragma once

#include "Engine/Physics/3D/ForceGenerator.hpp"

struct ForceRegistration
{
	Entity3* m_entity3;
	ForceGenerator* m_generator;

	ForceRegistration(Entity3* e, ForceGenerator* fg)
		: m_entity3(e), m_generator(fg){}
	~ForceRegistration(){}
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