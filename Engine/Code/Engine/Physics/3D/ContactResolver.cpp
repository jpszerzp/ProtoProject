#include "Engine/Physics/3D/ContactResolver.hpp"

ContactResolver::ContactResolver(uint iterations, eResolveScheme scheme)
	: m_iterations(iterations), m_scheme(scheme)
{
	m_collision = new CollisionData3(MAX_CONTACTS);
}

ContactResolver::ContactResolver()
{
	m_iterations = 0;
	m_iterationsUsed = 0;
	m_scheme = RESOLVE_ALL;
	m_collision = new CollisionData3(MAX_CONTACTS);
}


ContactResolver::ContactResolver(eResolveScheme scheme)
	: m_scheme(scheme)
{
	m_iterations = 0;
	m_iterationsUsed = 0;
	m_collision = new CollisionData3(MAX_CONTACTS);
}

void ContactResolver::ResolveContacts(float deltaTime)
{
	if (m_scheme == RESOLVE_ALL)
		ResolveContactsAll(deltaTime);
	else if (m_scheme == RESOLVE_ITERATIVE)
		ResolveContactsIterative(deltaTime);
}

void ContactResolver::ResolveContactsIterative(float deltaTime)
{
	if (!m_collision->m_contacts.empty())
	{
		m_iterationsUsed = 0;
		while(m_iterationsUsed < m_iterations)
		{
			// Find the contact with the largest closing velocity;
			float max = 0;
			uint maxIndex = 0;
			for (uint i = 0; i < m_collision->m_contacts.size(); ++i)
			{
				const Contact3& contact = m_collision->m_contacts[i];
				float separate = contact.ComputeSeparatingVelocity();
				if (separate < max)
				{
					max = separate;
					maxIndex = i;
				}
			}

			// Resolve this contact.
			m_collision->m_contacts[maxIndex].ResolveContact(deltaTime);
			++m_iterationsUsed;
		}
	}
}

void ContactResolver::ResolveContactsAll(float deltaTime)
{
	for (std::vector<Contact3>::size_type idx = 0; idx < m_collision->m_contacts.size(); ++idx)
	{
		Contact3 contact = m_collision->m_contacts[idx];
		contact.ResolveContact(deltaTime);
	}
}

void ContactResolver::ClearRecords()
{
	m_collision->ClearContacts();
}

