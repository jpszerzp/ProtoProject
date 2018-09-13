#include "Engine/Physics/3D/ContactResolver.hpp"

ContactResolver::ContactResolver(uint iterations)
	: m_iterations(iterations)
{

}

void ContactResolver::ResolveContacts(std::vector<Contact3>& contacts,
	uint numContacts, float deltaTime)
{
	m_iterationsUsed = 0;
	while(m_iterationsUsed < m_iterations)
	{
		// Find the contact with the largest closing velocity;
		float max = 0;
		uint maxIndex = 0;
		for (uint i = 0; i < numContacts; ++i)
		{
			const Contact3& contact = contacts[i];
			float separate = contact.ComputeSeparatingVelocity();
			if (separate < max)
			{
				max = separate;
				maxIndex = i;
			}
		}

		// Resolve this contact.
		contacts[maxIndex].ResolveContact(deltaTime);
		++m_iterationsUsed;
	}
}

