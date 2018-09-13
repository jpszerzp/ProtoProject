#include "Game/PachinkoState.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

PachinkoState::PachinkoState()
{
	m_physicsScene = new PhysicsScene();
	m_physicsScene->m_scheme = PHYSICS_CORRECTIVE;

	Renderer* renderer = Renderer::GetInstance();

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	if (!m_camera)
	{
		m_camera = new Camera();
		m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_camera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_camera->SetAspect(aspect);

		m_camera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

	if (!m_UICamera)
	{
		m_UICamera = new Camera();
		m_UICamera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_UICamera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_UICamera->SetAspect(aspect);

		m_UICamera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

	// const disc 1
	Vector2 discCenter = Vector2(-width / 8.f, height / 16.f);
	float discRadius = 16.f;
	Disc2 discPrimitive = Disc2(discCenter, discRadius);
	m_d0 = new DiscEntity(discPrimitive, true);
	m_physicsScene->AddDiscPhysicsEntity(m_d0);
	m_d0->m_scene = m_physicsScene;

	// const disc 2
	discCenter = Vector2(width / 8.f, height / 4.f);
	discRadius = 48.f;
	discPrimitive = Disc2(discCenter, discRadius);
	m_d1 = new DiscEntity(discPrimitive, true);
	m_physicsScene->AddDiscPhysicsEntity(m_d1);
	m_d1->m_scene = m_physicsScene;

	/*
	// const obb
	Vector2 obbCenter = Vector2(1.5f * width / 4.f, -height / 4.f);
	float obbRotation = 45.f;
	Vector2 obbScale = Vector2(256.f, 32.f);
	OBB2 obb = OBB2(obbCenter, obbRotation, obbScale);
	RectangleEntity* r0 = new RectangleEntity(obb, true);
	m_physicsScene->AddRectanglePhysicsEntity(r0);
	r0->m_scene = m_physicsScene;
	*/

	Vector2 titleTextMin = Vector2(-width / 2.f, height / 2.f - 32.f);
	float titleHeight = height / 50.f;
	Rgba titleTextTint = Rgba::WHITE;
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	std::string title = "Welcome to Pachinko!";
	m_titleMesh = Mesh::CreateTextImmediate(titleTextTint, titleTextMin, font, titleHeight, 1.f, title, VERT_PCU);
}

PachinkoState::~PachinkoState()
{
	delete m_d0;
	m_d0 = nullptr;

	delete m_d1;
	m_d1 = nullptr;

	delete m_titleMesh;
	m_titleMesh = nullptr;

	delete m_gravTextMesh;
	m_gravTextMesh = nullptr;
}

void PachinkoState::Update(float deltaTime)
{
	UpdateKeyboard(deltaTime);
	UpdateMouse(deltaTime);

	m_physicsScene->UpdateScene(deltaTime);

	UpdateUI(deltaTime);

	UpdateDelete();
}

void PachinkoState::UpdateMouse(float)
{

}

void PachinkoState::UpdateKeyboard(float)
{
	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_T))
	{
		// toggle gravity
		m_physicsScene->m_gravityOn = !m_physicsScene->m_gravityOn;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_R))
	{
		// reset velocity to 0 for all dyanamic discs
		for each (DiscEntity* se in m_dynamicDiscs)
		{
			se->m_velocity = Vector2::ZERO;
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_B))
	{
		for each (DiscEntity* se in m_dynamicDiscs)
		{
			se->m_velocity *= 2.f;
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_1))
	{
		Vector2 discCenter = Vector2(-width / 4.f, height / 4.f);
		float discRadius = 32.f;
		Disc2 disc = Disc2(discCenter, discRadius);
		DiscEntity* se = new DiscEntity(disc);
		se->m_velocity = Vector2(0.f, 256.f);
		m_physicsScene->AddDiscPhysicsEntity(se);
		m_dynamicDiscs.push_back(se);
		se->m_scene = m_physicsScene;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_2))
	{
		Vector2 discCenter = Vector2(width / 4.f, height / 4.f);
		float discRadius = 32.f;
		Disc2 disc = Disc2(discCenter, discRadius);
		DiscEntity* se = new DiscEntity(disc);
		se->m_velocity = Vector2(0.f, -256.f);
		m_physicsScene->AddDiscPhysicsEntity(se);
		m_dynamicDiscs.push_back(se);
		se->m_scene = m_physicsScene;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_3))
	{
		Vector2 discCenter = Vector2(width / 4.f, -height / 4.f);
		float discRadius = GetRandomFloatInRange(32.f, 48.f);
		Disc2 disc = Disc2(discCenter, discRadius);
		DiscEntity* se = new DiscEntity(disc);
		se->m_velocity = Vector2(-256.f, 0.f);
		m_physicsScene->AddDiscPhysicsEntity(se);
		m_dynamicDiscs.push_back(se);
		se->m_scene = m_physicsScene;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_4))
	{
		Vector2 discCenter = Vector2(-1.5f * width / 4.f, -height / 4.f);
		float discRadius = GetRandomFloatInRange(32.f, 48.f);
		Disc2 disc = Disc2(discCenter, discRadius);
		DiscEntity* se = new DiscEntity(disc);
		se->m_velocity = Vector2(256.f, 0.f);
		m_physicsScene->AddDiscPhysicsEntity(se);
		m_dynamicDiscs.push_back(se);
		se->m_scene = m_physicsScene;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_5))
	{
		// shoot from straight top
		Vector2 discCenter = Vector2(0.f, 1.5f * height / 4.f);
		float discRadius = GetRandomFloatInRange(32.f, 48.f);
		Disc2 disc = Disc2(discCenter, discRadius);
		DiscEntity* se = new DiscEntity(disc);
		se->m_velocity = Vector2(0.f, -512.f);
		m_physicsScene->AddDiscPhysicsEntity(se);
		m_dynamicDiscs.push_back(se);
		se->m_scene = m_physicsScene;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		// clear all dynamic discs
		for each (DiscEntity* se in m_dynamicDiscs)
		{
			se->m_dead = true;
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_P))
	{
		// switch mode
		if (m_physicsScene->m_scheme == PHYSICS_CORRECTIVE)
		{
			m_physicsScene->m_scheme = PHYSICS_PREVENTATIVE;
		}
		else if (m_physicsScene->m_scheme == PHYSICS_PREVENTATIVE)
		{
			m_physicsScene->m_scheme = PHYSICS_CORRECTIVE;
		}
	}
}

void PachinkoState::UpdateUI(float)
{
	Renderer* renderer = Renderer::GetInstance();
	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();

	if (m_gravTextMesh != nullptr)
	{
		delete m_gravTextMesh;
		m_gravTextMesh = nullptr;
	}

	std::string toggleText = (m_physicsScene->m_gravityOn) ? "On" : "Off";
	Vector2 gravityTextMin = Vector2(-width / 2.f, height / 2.f - 96.f);
	float gravityTextHeight = height / 50.f;
	Rgba gravityTextTint = Rgba::WHITE;
	std::string gravityTitle = "Gravity: " + toggleText;
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	m_gravTextMesh = Mesh::CreateTextImmediate(gravityTextTint, gravityTextMin, font, gravityTextHeight, 1.f, gravityTitle, VERT_PCU);
}

void PachinkoState::UpdateDelete()
{
	for (int discIdx = (int)(m_dynamicDiscs.size()- 1U); discIdx >= 0; --discIdx)
	{
		DiscEntity* disc = m_dynamicDiscs[discIdx];

		if (disc->m_dead)
		{
			delete disc;

			m_dynamicDiscs[discIdx] = m_dynamicDiscs[m_dynamicDiscs.size() - 1U];
			m_dynamicDiscs.pop_back();
		}
	}

	for (int recIdx = (int)(m_dynamicRecs.size()- 1U); recIdx >= 0; --recIdx)
	{
		RectangleEntity* rec = m_dynamicRecs[recIdx];

		if (rec->m_dead)
		{
			delete rec;

			m_dynamicRecs[recIdx] = m_dynamicRecs[m_dynamicRecs.size() - 1U];
			m_dynamicRecs.pop_back();
		}
	}

	for (int quadIdx = (int)(m_dynamicQuads.size()- 1U); quadIdx >= 0; --quadIdx)
	{
		QuadEntity* quad = m_dynamicQuads[quadIdx];

		if (quad->m_dead)
		{
			delete quad;

			m_dynamicQuads[quadIdx] = m_dynamicQuads[m_dynamicQuads.size() - 1U];
			m_dynamicQuads.pop_back();
		}
	}

	m_physicsScene->UpdateDelete();
}

void PachinkoState::Render(Renderer* renderer)
{
	renderer->SetCamera(m_camera);
	renderer->ClearScreen(Rgba::BLACK);

	DrawImmediateTitles(m_titleMesh);
	DrawImmediateTitles(m_gravTextMesh);

	m_physicsScene->RenderScene(renderer);
}

void PachinkoState::DrawImmediateTitles(Mesh* mesh)
{
	if ( mesh != nullptr )
	{
		Renderer* renderer = Renderer::GetInstance();
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->DrawMesh(mesh);
	}
}

