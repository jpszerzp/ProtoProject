#pragma once

#include "Engine/Physics/3D/Rigidbody3.hpp"

struct PotentialContact
{
	Rigidbody3* m_rb1;
	Rigidbody3* m_rb2;
};

template<class T>
class BVHNode 
{
public:
	BVHNode* m_children[2];
	BVHNode* m_parent;
	T m_volume;
	Rigidbody3* m_body;

public:
	BVHNode(BVHNode* parent, const T& volume, Rigidbody3* body = nullptr);
	~BVHNode();

	bool IsLeaf() const { return (m_body != nullptr); }
	uint GetPotentialContacts(std::vector<PotentialContact>& contacts, uint limit) const;

	bool Overlaps(const BVHNode<T>* other) const;
	uint GetPotentialContactsAgainst(const BVHNode<T>* other, std::vector<PotentialContact>& contacts, uint limit) const;

	void Insert(Rigidbody3* body, const T& volume);
	void RecalculateBV(bool recursive = true);
};

template<class T>
BVHNode<T>::~BVHNode()
{
	if (m_parent != nullptr)
	{
		BVHNode<T>* sibling;
		if (m_parent->m_children[0] == this)
			sibling = m_parent->m_children[1];
		else
			sibling = m_parent->m_children[0];

		m_parent->m_volume = sibling->m_volume;
		m_parent->m_body = sibling->m_body;
		m_parent->m_children[0] = sibling->m_children[0];
		m_parent->m_children[1] = sibling->m_children[1];

		sibling->m_parent = nullptr;
		sibling->m_body = nullptr;
		sibling->m_children[0] = nullptr;
		sibling->m_children[1] = nullptr;
		delete sibling;

		m_parent->RecalculateBV();
	}

	if (m_children[0] != nullptr)
	{
		m_children[0]->m_parent = nullptr;
		delete m_children[0];
	}
	if (m_children[1] != nullptr)
	{
		m_children[1]->m_parent = nullptr;
		delete m_children[1];
	}
}

template<class T>
BVHNode<T>::BVHNode(BVHNode* parent, const T& volume, Rigidbody3* body /*= nullptr*/)
{
	m_parent = parent;
	m_volume = volume;
	m_body = body;
	m_children[0] = nullptr;
	m_children[1] = nullptr;
}

template<class T>
void BVHNode<T>::RecalculateBV(bool recursive /*= true*/)
{
	if (IsLeaf())
		return;
	m_volume = T(m_children[0]->m_volume, m_children[1]->m_volume);

	if (m_parent != nullptr)
		m_parent->RecalculateBV();
}

template<class T>
void BVHNode<T>::Insert(Rigidbody3* body, const T& volume)
{
	if (IsLeaf())
	{
		m_children[0] = new BVHNode<T>(this, m_volume, m_body);
		m_children[1] = new BVHNode<T>(this, volume, body);
		this->m_body = nullptr;
		RecalculateBV();
	}
	else
	{
		if (m_children[0]->m_volume.GetGrowth(volume) < m_children[1]->m_volume.GetGrowth(volume))
			m_children[0]->Insert(body, volume);
		else
			m_children[1]->Insert(body, volume);
	}
}

template<class T>
bool BVHNode<T>::Overlaps(const BVHNode<T>* other) const
{
	TODO("Implement Overlaps in bounding volume class");
	return m_volume->Overlaps(other->m_volume);
}

template<class T>
uint BVHNode<T>::GetPotentialContacts(std::vector<PotentialContact>& contacts, uint limit) const
{
	if (IsLeaf() || limit == 0)
		return 0;

	uint potential_contact = m_children[0]->GetPotentialContactsAgainst(m_children[1], contacts, limit);
	return potential_contact;
}

template<class T>
uint BVHNode<T>::GetPotentialContactsAgainst(const BVHNode<T>* other, std::vector<PotentialContact>& contacts, uint limit) const
{
	if (!Overlaps(other) || limit == 0)
		return 0;

	// when both node at leaf, generate potential contact
	PotentialContact pContact = PotentialContact();
	if (IsLeaf() && other->IsLeaf())
	{
		pContact.m_rb1 = m_body;
		pContact.m_rb2 = other->m_body;
		contacts.push_back(pContact);
		return 1;
	}

	TODO("Implement GetVolume in bounding volume class");
	if (other->IsLeaf() || (!IsLeaf() && (m_volume->GetVolume() >= other->m_volume->GetVolume())))
	{
		uint count = m_children[0]->GetPotentialContactsAgainst(other, contacts, limit);
		
		if (limit > count)
		{
			// process the other child branch since we still have space open
			uint other_contact = m_children[1]->GetPotentialContactsAgainst(other, contacts, limit - count);
			return (count + other_contact);
		}
		else
			return count;
	}
	else
	{
		uint count = GetPotentialContactsAgainst(other->m_children[0], contacts, limit);

		if (limit > count)
		{
			uint other_count = GetPotentialContactsAgainst(other->m_children[1], contacts, limit - count);
			return (count + other_count);
		}
		else
			return count;
	}
}