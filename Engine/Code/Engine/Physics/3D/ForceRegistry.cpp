#include "Engine/Physics/3D/ForceRegistry.hpp"

void ParticleForceRegistry::Register(Entity3* e, ForceGenerator* generator)
{
	ParticleForceRegistration* registration = new ParticleForceRegistration(e, generator);
	m_registrations.push_back(registration);
}

void ParticleForceRegistry::ClearAllRegistries()
{
	m_registrations.clear();
}

void ParticleForceRegistry::UpdateForces(float deltaTime)
{
	for each (ParticleForceRegistration* fr in m_registrations)
	{
		Entity3* theEntity = fr->m_entity3;
		ForceGenerator* theGenerator = fr->m_generator;

		theGenerator->UpdateForce(theEntity, deltaTime);
	}
}

void RigidForceRegistry::Register(Rigidbody3* rb, RigidForceGenerator* rbGenerator)
{
	RigidbodyForceRegistration* rbRegistration = new RigidbodyForceRegistration(rb, rbGenerator);
	m_registrations.push_back(rbRegistration);
}

void RigidForceRegistry::ClearAllRegistries()
{
	m_registrations.clear();
}

void RigidForceRegistry::UpdateForces(float deltaTime)
{
	for each (RigidbodyForceRegistration* rfr in m_registrations)
	{
		RigidForceGenerator* theGenerator = rfr->m_rigidGenerator;

		theGenerator->UpdateForce(rfr->m_rb3, deltaTime);
	}
}
