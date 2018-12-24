#include "Game/ControlGroup.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Box.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/HullObject.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Physics/3D/QuadRb3.hpp"
#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderable.hpp"


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

static QuickHull* fake_hull = nullptr;		// holder for mksi hull
void ControlGroup::ProcessInput()
{
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		for (GameObject* go : m_gos)
			go->m_debugOn = !go->m_debugOn;
	}

	GameObject* g0 = m_gos[0];
	GameObject* g1 = m_gos[1];

	if (g0->GetEntity() != nullptr)
	{
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
	}
	else
	{
		// for line control
		Line* line = dynamic_cast<Line*>(g0);
		if (line != nullptr)
		{
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
			{
				line->m_start += Vector3(0.f, 0.f, .01f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
			{
				line->m_start -= Vector3(0.f, 0.f, .01f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
			{
				line->m_start -= Vector3(.01f, 0.f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
			{
				line->m_start += Vector3(.01f, 0.f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
			{
				line->m_start += Vector3(0.f, .01f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
			else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
			{
				line->m_start -= Vector3(0.f, .01f, 0.f);
				if (line->m_renderable->m_mesh != nullptr)
				{
					delete line->m_renderable->m_mesh;
					line->m_renderable->m_mesh = nullptr;
				}
				line->m_renderable->m_mesh = Mesh::CreateLineImmediate(VERT_PCU, line->m_start, line->m_end, Rgba::WHITE);
			}
		}

		// the object may be a hull, handle it here
		HullObject* hull = dynamic_cast<HullObject*>(g0);
		if (hull != nullptr)
		{
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos += Vector3(0.f, 0.f, 1.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos -= Vector3(0.f, 0.f, 1.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos -= Vector3(1.f, 0.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos += Vector3(1.f, 0.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos += Vector3(0.f, 1.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
			{
				Vector3 pos = hull->m_renderable->m_transform.GetLocalPosition();
				pos -= Vector3(0.f, 1.f, 0.f);
				hull->m_renderable->m_transform.SetLocalPosition(pos);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_1))
			{
				Vector3 rot = hull->m_renderable->m_transform.GetLocalRotation();
				rot += Vector3(1.f, 0.f, 0.f);
				hull->m_renderable->m_transform.SetLocalRotation(rot);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
			{
				Vector3 rot = hull->m_renderable->m_transform.GetLocalRotation();
				rot += Vector3(0.f, 1.f, 0.f);
				hull->m_renderable->m_transform.SetLocalRotation(rot);
			}
			if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_7))
			{
				Vector3 rot = hull->m_renderable->m_transform.GetLocalRotation();
				rot += Vector3(0.f, 0.f, 1.f);
				hull->m_renderable->m_transform.SetLocalRotation(rot);
			}
		}
	}

	if (g1->GetEntity() != nullptr)
	{
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0))
	{
		// first generate the minkowski hull if the group is all about hull object
		if (m_id == CONTROL_HULL_HULL)
		{
			if (fake_hull == nullptr)
			{
				HullObject* hull_0 = dynamic_cast<HullObject*>(g0);
				HullObject* hull_1 = dynamic_cast<HullObject*>(g1);

				QuickHull* qh_0 = hull_0->GetHullPrimitive();
				QuickHull* qh_1 = hull_1->GetHullPrimitive();

				fake_hull = QuickHull::GenerateMinkowskiHull(qh_0, qh_1);
			}
		}
	}
}

static Mesh* obb3_obb3_pt_pos = nullptr;
static Mesh* obb3_obb3_face_center = nullptr;
void ControlGroup::RenderCore(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gos.size(); ++idx)
	{
		if (!m_gos[idx]->m_isInForwardPath)
			m_gos[idx]->Render(renderer);
	}

	if (m_id == CONTROL_HULL_HULL)
	{
		if (fake_hull != nullptr)
			fake_hull->RenderHull(renderer);
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
	case CONTROL_BOX_PLANE:
	{
		Contact3 contact;
		
		Box* b = static_cast<Box*>(m_gos[0]);
		Quad* q = static_cast<Quad*>(m_gos[1]);

		BoxRB3* rbb = static_cast<BoxRB3*>(b->GetEntity());
		QuadRB3* rbq = static_cast<QuadRB3*>(q->GetEntity());

		const OBB3& obb = rbb->m_primitive;
		const Plane& pl = rbq->m_primitive;

		bool intersected = CollisionDetector::OBB3VsPlane3Core(obb, pl, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_BOX_SPHERE:
	{
		Contact3 contact;
		
		Box* b = static_cast<Box*>(m_gos[0]);
		Sphere* s = static_cast<Sphere*>(m_gos[1]);

		BoxRB3* rbb = static_cast<BoxRB3*>(b->GetEntity());
		SphereRB3* rbs = static_cast<SphereRB3*>(s->GetEntity());

		const OBB3& obb = rbb->m_primitive;
		const Sphere3& sph = rbs->m_primitive;

		bool intersected = CollisionDetector::OBB3VsSphere3Core(obb, sph, contact);
		if (intersected)
			m_contacts.push_back(contact);
	}
		break;
	case CONTROL_BOX_BOX:
	{
		Contact3 contact;

		Box* b_0 = static_cast<Box*>(m_gos[0]);
		Box* b_1 = static_cast<Box*>(m_gos[1]);

		BoxRB3* rbb_0 = static_cast<BoxRB3*>(b_0->GetEntity());
		BoxRB3* rbb_1 = static_cast<BoxRB3*>(b_1->GetEntity());

		const OBB3& obb_0 = rbb_0->m_primitive;
		const OBB3& obb_1 = rbb_1->m_primitive;

		//TODO("contact point is not correct - it is set to center of an entity, see Core for detail");
		//bool intersected = CollisionDetector::OBB3VsOBB3Core(obb_0, obb_1, contact);
		//if (intersected)
		//	m_contacts.push_back(contact);

		//// debug
		//Vector3 vert_pos;
		//Vector3 face_center;
		//CollisionDetector::OBB3VsOBB3CoreBreakdownPtVsFace(obb_0, obb_1, vert_pos, face_center);
		//if (obb3_obb3_pt_pos != nullptr)
		//{
		//	delete obb3_obb3_pt_pos;
		//	obb3_obb3_pt_pos = nullptr;
		//}
		//obb3_obb3_pt_pos = Mesh::CreatePointImmediate(VERT_PCU, vert_pos, Rgba::MEGENTA);
		//if (obb3_obb3_face_center != nullptr)
		//{
		//	delete obb3_obb3_face_center;
		//	obb3_obb3_face_center = nullptr;
		//}
		//obb3_obb3_face_center = Mesh::CreatePointImmediate(VERT_PCU, face_center, Rgba::BLUE);

		CollisionDetector::OBB3VsOBB3StepOne(obb_0, obb_1);
		CollisionDetector::OBB3VsOBB3StepTwo(obb_0, obb_1);
		CollisionDetector::OBB3VsOBB3StepThree(obb_0, obb_1);
	}
		break;
	case CONTROL_LINE_LINE:
	{

	}
		break;
	case CONTROL_HULL_HULL:
	{

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

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
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

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
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
	case CONTROL_BOX_PLANE:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Box v.s plane";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
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
	case CONTROL_BOX_SPHERE:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Box v.s sphere";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
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
	case CONTROL_BOX_BOX:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Box v.s box";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string contact_num = Stringf("Contact number: %i", m_contacts.size());
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, contact_num, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

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
		break;
	case CONTROL_LINE_LINE:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Line v.s line";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		// for convenience, update closest point here directly so that we do not need class variables for 
		Line* l_1 = static_cast<Line*>(m_gos[0]);
		const Vector3& start_1 = l_1->m_start;
		const Vector3& end_1 = l_1->m_end;
		LineSegment3 seg_1 = LineSegment3(start_1, end_1);

		Line* l_2 = static_cast<Line*>(m_gos[1]);
		const Vector3& start_2 = l_2->m_start;
		const Vector3& end_2 = l_2->m_end;
		LineSegment3 seg_2 = LineSegment3(start_2, end_2);

		Vector3 close_1, close_2;
		float t1, t2;
		float close_dist_sqr = LineSegment3::ClosestPointsSegments(seg_1, seg_2, t1, t2, close_1, close_2);
		DebugRenderLine(0.1f, close_1, close_2, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);

		Vector3 normal = close_1 - close_2;
		float dot1 = DotProduct(seg_1.extent, normal);
		float dot2 = DotProduct(seg_2.extent, normal);

		std::string close_name1 = Stringf("Closest point on line 1: %f, %f, %f", close_1.x, close_1.y, close_1.z);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, close_name1, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string close_name2 = Stringf("Closest point on line 2: %f, %f, %f", close_2.x, close_2.y, close_2.z);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, close_name2, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string dot_str1 = Stringf("Close vector dot with line 1: %f", dot1);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, dot_str1, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);

		std::string dot_str2 = Stringf("Close vector dot with line 2: %f", dot2);
		mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, dot_str2, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);
	}
		break;
	case CONTROL_HULL_HULL:
	{
		Vector2 min = m_startMin;

		std::string cp_title = "Hull v.s Hull";
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, cp_title, VERT_PCU);
		m_view.push_back(mesh);
		min -= Vector2(0.f, m_textHeight);
	}
		break;
	default:
		break;
	}
}
