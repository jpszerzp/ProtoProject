#include "Engine/Physics/3D/ForceRegistry.hpp"

void ForceRegistry::Register(Entity3* e, ForceGenerator* generator)
{
	ForceRegistration* registration = new ForceRegistration(e, generator);
	m_registrations.push_back(registration);
}

void ForceRegistry::ClearAllRegistries()
{
	m_registrations.clear();
}

void ForceRegistry::UpdateForces(float deltaTime)
{
	for each (ForceRegistration* fr in m_registrations)
	{
		Entity3* theEntity = fr->m_entity3;
		ForceGenerator* theGenerator = fr->m_generator;

		theGenerator->UpdateForce(theEntity, deltaTime);
	}
}
