#include "Game/Links.hpp"

float Link::GetCurrentLength() const
{
	Vector3 rel_pos = m_p1->GetWorldPosition() - m_p2->GetWorldPosition();
	return rel_pos.GetLength();
}

uint Cable::FillContact(std::vector<Contact3>& contacts, uint) const
{
	float length = GetCurrentLength();

	if (length < m_maxLength)
		return 0;		// have not overextended

	// have overextended, create new contact
	Contact3 new_contact = Contact3();
	new_contact.m_e1 = m_p1->m_physEntity;
	new_contact.m_e2 = m_p2->m_physEntity;

	Vector3 normal = m_p2->GetWorldPosition() - m_p1->GetWorldPosition();
	normal.NormalizeAndGetLength();
	new_contact.m_normal = normal;

	new_contact.m_penetration = length - m_maxLength;
	new_contact.m_restitution = m_cableRestitution;

	contacts.push_back(new_contact);

	return 1;
}

Rod::Rod(float length, Point* p1, Point* p2)
	: m_length(length)
{
	m_p1 = p1;
	m_p2 = p2;

	// no damp
	m_p1->m_physEntity->SetConsiderDamp(false);
	m_p2->m_physEntity->SetConsiderDamp(false);

	// set a force of gravity
	m_p1->m_physEntity->SetNetForce(Vector3(0.f, -.5f, 0.f));
	m_p2->m_physEntity->SetNetForce(Vector3(0.f, -.1f, 0.f));

	// forces are persistent
	m_p1->m_physEntity->SetNetForcePersistent(true);
	m_p2->m_physEntity->SetNetForcePersistent(true);
}

uint Rod::FillContact(std::vector<Contact3>& contacts, uint) const
{
	contacts.clear();

	float length = GetCurrentLength();

	if (length == m_length)
		return 0;

	Contact3 contact = Contact3();
	contact.m_e1 = m_p1->m_physEntity;
	contact.m_e2 = m_p2->m_physEntity;

	Vector3 normal = m_p2->GetWorldPosition() - m_p1->GetWorldPosition();
	normal.NormalizeAndGetLength();

	if (length > m_length)
	{
		contact.m_normal = normal;
		contact.m_penetration = length - m_length;
	}
	else
	{
		contact.m_normal = normal * -1.f;
		contact.m_penetration = m_length - length;
	}

	contact.m_restitution = 0.f;

	contacts.push_back(contact);

	return 1;
}
