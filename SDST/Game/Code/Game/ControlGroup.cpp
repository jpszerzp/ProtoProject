#include "Game/ControlGroup.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

ControlGroup::ControlGroup(GameObject* go1, GameObject* go2, eControlID id)
{
	m_gos.push_back(go1);
	m_gos.push_back(go2);
	m_id = id;
}

void ControlGroup::ProcessInput()
{
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		for (GameObject* go : m_gos)
			go->m_debugOn = !go->m_debugOn;
	}

	GameObject* g0 = m_gos[0];
	GameObject* g1 = m_gos[1];

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
		g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
		g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
		g0->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
		g0->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
		g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
		g0->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		g0->GetEntity()->SetLinearVelocity(Vector3::ZERO);

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_I))
		g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
		g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
		g1->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
		g1->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
		g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
		g1->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		g1->GetEntity()->SetLinearVelocity(Vector3::ZERO);
}

void ControlGroup::Render(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gos.size(); ++idx)
	{
		if (!m_gos[idx]->m_isInForwardPath)
			m_gos[idx]->Render(renderer);
	}
}

void ControlGroup::Update(float deltaTime)
{
	ProcessInput();

	for (GameObject* go : m_gos)
		go->Update(deltaTime);

	// update collision
	switch (m_id)
	{
	case CONTROL_SPHERE_SPHERE:
	{
		Sphere* s1 = static_cast<Sphere*>(m_gos[0]);
		Sphere* s2 = static_cast<Sphere*>(m_gos[1]);

		SphereRB3* rb1 = static_cast<SphereRB3*>(s1->GetEntity());
		SphereRB3* rb2 = static_cast<SphereRB3*>(s2->GetEntity());

		const Sphere3& sph1 = rb1->m_primitive;
		const Sphere3& sph2 = rb2->m_primitive;

		m_intersect = CollisionDetector::Sphere3VsSphere3Core(sph1, sph2, m_contact);
	}
		break;
	default:
		break;
	}

	if (m_intersect)
	{
		Vector3 point = m_contact.m_point;
		Vector3 end = point + m_contact.m_normal * m_contact.m_penetration;
		DebugRenderLine(0.1f, point, end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
	}
}

std::string ControlGroup::GetControlIDString() const
{
	switch (m_id)
	{
	case CONTROL_SPHERE_SPHERE:
		return "Sphere3 v.s Sphere3";
		break;
	default:
		break;
	}
}

std::string ControlGroup::GetPointString() const
{
	return Stringf("Point: (%f, %f, %f)", m_contact.m_point.x, m_contact.m_point.y, m_contact.m_point.z);
}

std::string ControlGroup::GetNormalString() const
{
	return Stringf("Normal: (%f, %f, %f)", m_contact.m_normal.x, m_contact.m_normal.y, m_contact.m_normal.z);
}

std::string ControlGroup::GetPenetrationString() const
{
	return Stringf("Penetration: %f", m_contact.m_penetration);
}

