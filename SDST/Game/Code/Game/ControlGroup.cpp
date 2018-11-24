#include "Game/ControlGroup.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Physics/3D/QuadRb3.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Window.hpp"

ControlGroup::ControlGroup(GameObject* go1, GameObject* go2, const eControlID& id, const Vector3& observation)
{
	m_gos.push_back(go1);
	m_gos.push_back(go2);
	m_id = id;

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	m_textHeight = height / 50.f;
	m_startMin = Vector2(-width / 2.f, height / 2.f - m_textHeight);

	m_observation_pos = observation;
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

	Rigidbody3* rb0 = static_cast<Rigidbody3*>(g0->GetEntity());
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_1))
		rb0->SetAngularVelocity(Vector3(30.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
		rb0->SetAngularVelocity(Vector3(0.f, 30.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_7))
		rb0->SetAngularVelocity(Vector3(0.f, 0.f, 30.f));
	else
		rb0->SetAngularVelocity(Vector3::ZERO);

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

	Rigidbody3* rb1 = static_cast<Rigidbody3*>(g1->GetEntity());
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_2))
		rb1->SetAngularVelocity(Vector3(30.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5))
		rb1->SetAngularVelocity(Vector3(0.f, 30.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_8))
		rb1->SetAngularVelocity(Vector3(0.f, 0.f, 30.f));
	else 
		rb1->SetAngularVelocity(Vector3::ZERO);
}

void ControlGroup::RenderCore(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gos.size(); ++idx)
	{
		if (!m_gos[idx]->m_isInForwardPath)
			m_gos[idx]->Render(renderer);
	}
}

void ControlGroup::RenderUI()
{
	DrawTexts(m_view);
}

void ControlGroup::Update(float deltaTime)
{
	// clear all contacts at beginning of frame
	m_contacts.clear();

	ProcessInput();

	for (GameObject* go : m_gos)
		go->Update(deltaTime);

	// update collision
	switch (m_id)
	{
	case CONTROL_SPHERE_SPHERE:
	{
		Contact3 contact;

		Sphere* s1 = static_cast<Sphere*>(m_gos[0]);
		Sphere* s2 = static_cast<Sphere*>(m_gos[1]);

		SphereRB3* rb1 = static_cast<SphereRB3*>(s1->GetEntity());
		SphereRB3* rb2 = static_cast<SphereRB3*>(s2->GetEntity());

		const Sphere3& sph1 = rb1->m_primitive;
		const Sphere3& sph2 = rb2->m_primitive;

		bool intersected = CollisionDetector::Sphere3VsSphere3Core(sph1, sph2, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_SPHERE_PLANE:
	{
		Contact3 contact;

		Sphere* s = static_cast<Sphere*>(m_gos[0]);
		Quad* q = static_cast<Quad*>(m_gos[1]);

		SphereRB3* rbs = static_cast<SphereRB3*>(s->GetEntity());
		QuadRB3* rbq = static_cast<QuadRB3*>(q->GetEntity());

		const Sphere3& sph = rbs->m_primitive;
		const Plane& pl = rbq->m_primitive;

		bool intersected = CollisionDetector::Sphere3VsPlane3Core(sph, pl, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	default:
		break;
	}

	UpdateUI();

	UpdateDebugDraw();
}

void ControlGroup::UpdateDebugDraw()
{
	for (const Contact3& contact : m_contacts)
	{
		Vector3 point = contact.m_point;
		Vector3 end = point + contact.m_normal * contact.m_penetration;
		DebugRenderLine(0.1f, point, end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
	}
}

void ControlGroup::UpdateUI()
{
	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	DeleteVector(m_view);

	switch (m_id)
	{
	case CONTROL_SPHERE_SPHERE:
	{
		Vector2 min = m_startMin;

		// basic info
		std::string cp_title = "Sphere v.s sphere";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contac_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contac_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		// detailed contact info
		if (m_contacts.size() > 0U)
		{
			for (int i = 0; i < m_contacts.size(); ++i)
			{
				const Contact3& theContact = m_contacts[i];
				std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
					theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
					theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
				mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
				m_view.push_back(mesh);
				min -= Vector2(0.f, m_textHeight);
			}
		}
	}
		break;
	case CONTROL_SPHERE_PLANE:
	{
		Vector2 min = m_startMin;

		// basic info
		std::string cp_title = "Sphere v.s plane";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contac_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contac_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		if (m_contacts.size() > 0U)
		{
			for (int i = 0; i < m_contacts.size(); ++i)
			{
				const Contact3& theContact = m_contacts[i];
				std::string contact_info = Stringf("Contact at (%f, %f, %f), has normal (%f, %f, %f), with penetration %f",
					theContact.m_point.x, theContact.m_point.y, theContact.m_point.z,
					theContact.m_normal.x, theContact.m_normal.y, theContact.m_normal.z, theContact.m_penetration);
				mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_info, VERT_PCU);
				m_view.push_back(mesh);
				min -= Vector2(0.f, m_textHeight);
			}
		}
	}
		break;
	default:
		break;
	}
}
