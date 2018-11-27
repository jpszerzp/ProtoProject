#pragma once

#include "Engine/Physics/3D/Rigidbody3.hpp"

struct BVHContact
{
	Entity3* m_rb1;
	Entity3* m_rb2;
};

template<class T>
class BVHNode 
{
public:
	BVHNode* m_children[2];
	BVHNode* m_parent;
	T m_volume;
	Entity3* m_body;

public:
	BVHNode(BVHNode* parent, const T& volume, Entity3* body = nullptr);
	~BVHNode();

	bool IsLeaf() const { return (m_body != nullptr); }

	uint GetContacts(std::vector<BVHContact>& contacts, uint limit) const;
	bool Overlaps(const BVHNode<T>* other) const;
	uint GetContactsAgainst(const BVHNode<T>* other, 
		std::vector<BVHContact>& contacts, uint limit) const;

	void Insert(Entity3* body, const T& volume);
	BVHNode<T>* GetRightLeaf();
	void RecalculateBV();

	void DrawNode(Renderer* renderer);
	void UpdateNode();
	void UpdateBV();
};

template<class T>
BVHNode<T>* BVHNode<T>::GetRightLeaf()
{
	BVHNode<T>* res = nullptr;

	if (!IsLeaf())
		res = m_children[1]->GetRightLeaf();
	else
		res = this;

	return res;
}

template<class T>
void BVHNode<T>::UpdateBV()
{
	TODO("If later scale of BV or other information for rigid changes, need to update here too");

	if (IsLeaf())
	{
		m_volume.m_center = m_body->GetBoundingSphere().GetCenter();
		m_volume.m_transform.SetLocalPosition(m_volume.m_center);
	}
	else
	{
		m_volume = T(m_children[0]->m_volume, m_children[1]->m_volume);

		// bv recalculated, need to update mesh and transform
		Renderer* renderer = Renderer::GetInstance();
		m_volume.m_boundMesh = renderer->CreateOrGetMesh("sphere_pcu");
		m_volume.m_transform = Transform(
			m_volume.m_center, 
			Vector3::ZERO, 
			Vector3(m_volume.m_radius)); 
	}

	if (m_parent != nullptr)
		m_parent->UpdateBV();
}

template<class T>
void BVHNode<T>::UpdateNode()
{
	if (!IsLeaf())
	{
		m_children[0]->UpdateNode();
		m_children[1]->UpdateNode();
	}
	else
		UpdateBV();
}

template<class T>
void BVHNode<T>::DrawNode(Renderer* renderer)
{
	m_volume.DrawBound(renderer);

	if (!IsLeaf())
	{
		m_children[0]->DrawNode(renderer);
		m_children[1]->DrawNode(renderer);
	}
}

template<class T>
BVHNode<T>::~BVHNode()
{
	if (m_parent != nullptr)
	{
		// in our scheme, sibling will NEVER be nullptr
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
BVHNode<T>::BVHNode(BVHNode* parent, const T& volume, Entity3* body /*= nullptr*/)
{
	m_parent = parent;
	m_volume = volume;
	m_body = body;
	m_children[0] = nullptr;
	m_children[1] = nullptr;
}

template<class T>
void BVHNode<T>::RecalculateBV()
{
	if (IsLeaf())
		return;
	m_volume = T(m_children[0]->m_volume, m_children[1]->m_volume);

	// bv recalculated, need to update mesh and transform
	Renderer* renderer = Renderer::GetInstance();
	m_volume.m_boundMesh = renderer->CreateOrGetMesh("sphere_pcu");
	m_volume.m_transform = Transform(
		m_volume.m_center, 
		Vector3::ZERO, 
		Vector3(m_volume.m_radius));

	if (m_parent != nullptr)
		m_parent->RecalculateBV();
}

template<class T>
void BVHNode<T>::Insert(Entity3* body, const T& volume)
{
	if (IsLeaf())
	{
		m_children[0] = new BVHNode<T>(this, m_volume, m_body);
		m_children[1] = new BVHNode<T>(this, volume, body);
		this->m_body = nullptr;
		m_children[0]->m_parent = this;
		m_children[1]->m_parent = this;
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
	return m_volume.Overlaps(&other->m_volume);
}

template<class T>
uint BVHNode<T>::GetContacts(std::vector<BVHContact>& contacts, uint limit) const
{
	if (IsLeaf() || limit == 0)
		return 0;

	uint potential_contact = m_children[0]->GetContactsAgainst(m_children[1], contacts, limit);
	return potential_contact;
}

template<class T>
uint BVHNode<T>::GetContactsAgainst(const BVHNode<T>* other, std::vector<BVHContact>& contacts, uint limit) const
{
	if (!Overlaps(other) || limit == 0)
		return 0;

	// when both node at leaf (ent to ent),
	// generate potential contact
	BVHContact pContact = BVHContact();
	if (IsLeaf() && other->IsLeaf())
	{
		pContact.m_rb1 = m_body;
		pContact.m_rb2 = other->m_body;
		contacts.push_back(pContact);
		return 1;
	}

	if (other->IsLeaf() || (!IsLeaf() && (m_volume.GetVolume() >= other->m_volume.GetVolume())))
	{
		uint count = m_children[0]->GetContactsAgainst(other, contacts, limit);
		
		if (limit > count)
		{
			// process the other child branch since we still have space open
			uint other_count = m_children[1]->GetContactsAgainst(other, contacts, limit - count);
			return (count + other_count);
		}
		else
			return count;
	}
	else
	{
		uint count = GetContactsAgainst(other->m_children[0], contacts, limit);

		if (limit > count)
		{
			uint other_count = GetContactsAgainst(other->m_children[1], contacts, limit - count);
			return (count + other_count);
		}
		else
			return count;
	}
}