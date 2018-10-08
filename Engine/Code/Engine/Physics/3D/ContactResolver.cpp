#include "Engine/Physics/3D/ContactResolver.hpp"

ContactResolver::ContactResolver(uint iterations)
	: m_iterations(iterations)
{
	m_scheme = RESOLVE_ITERATIVE;
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
	else
		ResolveContactsCoherent(deltaTime);
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

void ContactResolver::ResolveContactsCoherent(float)
{
	/*
	if (!m_collision->m_contacts.empty())
		return;

	// prepare internal data for each contact
	PrepareContactsCoherent(deltaTime);

	// resolve contact positions
	ResolvePositionsCoherent(deltaTime);

	// resolve velocity
	ResolveVelocityCoherent(deltaTime);
	*/
}

void ContactResolver::PrepareContactsCoherent(float deltaTime)
{
	for (Contact3& c : m_collision->m_contacts)
		// prepare for data: contact matrix, relative pos, relative velocity
		c.PrepareInternal(deltaTime);
}

void ContactResolver::ResolvePositionsCoherent(float)
{
	uint i, idx;
	Vector3 linearChange[2];
	Vector3 angularChange[2];
	//float angularAmount[2];
	float max;
	Vector3 deltaPos;
	uint numContacts = (uint)m_collision->m_contacts.size();

	uint currentIter = 0;
	while (currentIter < COHERENT_POS_ITER)
	{
		max = COHERENT_POS_EPSILON;
		idx = numContacts;
		for (i = 0; i < numContacts; ++i)
		{
			if (m_collision->m_contacts[i].m_penetration > max)
			{
				max = m_collision->m_contacts[i].m_penetration;
				idx = i;
			}
		}
		if (idx == numContacts)
			break;

		TODO("Awake state match");

		// resolve max-penetration contact
		Contact3& maxContact = m_collision->m_contacts[idx];
		maxContact.ResolvePositionCoherent(linearChange, angularChange);

		for (i = 0; i < numContacts; ++i)
		{
			Contact3& thisContact = m_collision->m_contacts[i];

			// other contacts may be reordered, update all
			Entity3* ent1 = thisContact.m_e1;
			Entity3* ent2 = thisContact.m_e2;

			if (ent1 != nullptr)
			{
				if (ent1 == maxContact.m_e1)
				{
					deltaPos = angularChange[0].Cross(thisContact.m_relativePosWorld[0]);
					deltaPos += linearChange[0];

					TODO("Maybe multiply rotation amount later? how is that different? Need another array at that time");
					//thisContact.m_penetration -= rotationAmount[0] * DotProduct(deltaPos, thisContact.m_normal);
					thisContact.m_penetration -= DotProduct(deltaPos, thisContact.m_normal);
				}
				else if (ent1 == maxContact.m_e2)
				{
					deltaPos = angularChange[1].Cross(thisContact.m_relativePosWorld[0]);
					deltaPos += linearChange[1];

					thisContact.m_penetration -= DotProduct(deltaPos, thisContact.m_normal);
				}
			}

			if (ent2 != nullptr)
			{
				if (ent2 == maxContact.m_e1)
				{
					deltaPos = angularChange[0].Cross(thisContact.m_relativePosWorld[1]);
					deltaPos += linearChange[0];

					thisContact.m_penetration += DotProduct(deltaPos, thisContact.m_normal);
				}
				else if (ent2 == maxContact.m_e2)
				{
					deltaPos = angularChange[1].Cross(thisContact.m_relativePosWorld[1]);
					deltaPos += linearChange[1];

					thisContact.m_penetration += DotProduct(deltaPos, thisContact.m_normal);
				}
			}
		}

		currentIter++;
	}
}

void ContactResolver::ResolveVelocityCoherent(float deltaTime)
{
	Vector3 linearChange[2];
	Vector3 angularChange[2];
	Vector3 delVel;
	uint numContacts = (uint)m_collision->m_contacts.size();

	uint iter = 0;
	while(iter < COHERENT_VEL_ITER)
	{
		float max = COHERENT_VEL_EPSILON;
		uint idx = numContacts;
		for (uint i = 0; i < numContacts; ++i)
		{
			if (m_collision->m_contacts[i].m_desiredVelDelta > max)
			{
				max = m_collision->m_contacts[i].m_desiredVelDelta;
				idx = i;
			}
		}
		if (idx == numContacts)
			break;

		TODO("Awake state match");

		Contact3& maxContact = m_collision->m_contacts[idx];
		maxContact.ResolveVelocityCoherent(linearChange, angularChange);

		for (uint i = 0; i < numContacts; ++i)
		{
			Contact3& thisContact = m_collision->m_contacts[i];

			// other contacts may be reordered, update all
			Entity3* ent1 = thisContact.m_e1;
			Entity3* ent2 = thisContact.m_e2;
			const Matrix33& toContact = thisContact.m_toWorld.Transpose();

			if (ent1 != nullptr)
			{
				if (ent1 == maxContact.m_e1)
				{
					delVel = angularChange[0].Cross(thisContact.m_relativePosWorld[0]);
					delVel += linearChange[0];

					thisContact.m_closingVel += toContact * delVel;

					thisContact.ComputeDesiredVelDeltaCoherent(deltaTime);
				}
				else if (ent1 == maxContact.m_e2)
				{
					delVel = angularChange[1].Cross(thisContact.m_relativePosWorld[0]);
					delVel += linearChange[1];

					thisContact.m_closingVel += toContact * delVel;
					
					thisContact.ComputeDesiredVelDeltaCoherent(deltaTime);
				}
			}

			if (ent2 != nullptr)
			{
				if (ent2 == maxContact.m_e1)
				{
					delVel = angularChange[0].Cross(thisContact.m_relativePosWorld[1]);
					delVel += linearChange[0];

					thisContact.m_closingVel -= toContact * delVel;

					thisContact.ComputeDesiredVelDeltaCoherent(deltaTime);
				}
				else if (ent2 == maxContact.m_e2)
				{
					delVel = angularChange[1].Cross(thisContact.m_relativePosWorld[1]);
					delVel += linearChange[1];

					thisContact.m_closingVel -= toContact * delVel;

					thisContact.ComputeDesiredVelDeltaCoherent(deltaTime);
				}
			}
		}

		iter++;
	}
}

void ContactResolver::ClearRecords()
{
	if (m_scheme == RESOLVE_ALL || m_scheme == RESOLVE_ITERATIVE)
		m_collision->ClearContacts();
	else
		m_collision->ClearCoherent();	// clear contacts exceeding threshold
}