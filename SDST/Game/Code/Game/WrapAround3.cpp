#include "Game/WrapAround3.hpp"
#include "Engine/Renderer/Renderable.hpp"

WrapAround::WrapAround(Vector3 min, Vector3 max)
{
	m_bounds = AABB3(min, max);
}

WrapAround::~WrapAround()
{

}

void WrapAround::Update()
{
	// goes thru every gameobject in this wraparound group and update ONLY when they teleport
	for (GameObject* go : m_gos)
	{
		// surpassing xmin
		if (go->GetEntity()->GetEntityCenter().x < m_bounds.m_min.x)
		{
			float pos_x = m_bounds.m_max.x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().x > m_bounds.m_max.x)
		{
			float pos_x = m_bounds.m_min.x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().y < m_bounds.m_min.y)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = m_bounds.m_max.y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().y > m_bounds.m_max.y)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = m_bounds.m_min.y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().z < m_bounds.m_min.z)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = m_bounds.m_max.z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().z > m_bounds.m_max.z)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = m_bounds.m_min.z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		// see line 74 in Sphere.cpp for physics driven update logic
		go->GetEntity()->m_entityTransform.SetLocalPosition(go->GetEntity()->GetEntityCenter());
		go->GetEntity()->m_boundSphere.m_transform.SetLocalPosition(go->GetEntity()->GetEntityCenter());
		go->GetEntity()->m_boxBoundTransform.SetLocalPosition(go->GetEntity()->GetEntityCenter());

		go->GetEntity()->UpdatePrimitives();

		go->m_renderable->m_transform = go->GetEntity()->GetEntityTransform();

		go->UpdateBasis();
	}
}

