#include "Game/Collision3State.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

Collision3State::Collision3State()
{
	Renderer* renderer = Renderer::GetInstance();
	m_sceneGraph = new RenderSceneGraph();
	m_forwardPath = new ForwardRenderPath();
	m_forwardPath->m_renderer = renderer;

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	m_cameraInitialPos = Vector3(0.f, 0.f, -7.f);
	m_cameraRotationSpd = 50.f;

	if (!m_camera)
	{
		m_camera = new Camera(m_cameraInitialPos);
		m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_camera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_camera->SetAspect(aspect);
		m_camera->SetFOV(45.f);

		m_camera->SetPerspectiveProjection(m_camera->GetFOV(), aspect, 1.f, 1000.f);

		m_sceneGraph->AddCamera(m_camera);
	}
	
	if (!m_UICamera)
	{
		m_UICamera = new Camera();
		m_UICamera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_UICamera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_UICamera->SetAspect(aspect);

		m_UICamera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

	m_textHeight = height / 50.f;
	m_titleMin = Vector2(-width / 2.f, height / 2.f - m_textHeight);
	Rgba titleColor = Rgba::WHITE;
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	std::string title = "Collision playground!";
	m_title = Mesh::CreateTextImmediate(titleColor, m_titleMin, font, m_textHeight, .5f, title, VERT_PCU);

	Vector3 pos = Vector3(-1.5f, 0.f, 0.f);
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Rgba tint = Rgba::WHITE;
	std::string meshName = "sphere_pcu";
	std::string matName = "default";
	eMoveStatus stat = MOVE_KINEMATIC;
	eBodyIdentity bid = BODY_RIGID;
	bool multipass = false;
	eCompare compare = COMPARE_LESS;
	eCullMode cull = CULLMODE_BACK;
	eWindOrder wind = WIND_COUNTER_CLOCKWISE;
	eDynamicScheme scheme = DISCRETE;
	Sphere* sph = new Sphere(pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	Rigidbody3* rigid = static_cast<Rigidbody3*>(sph->GetEntity());
	sph->m_physDriven = true;
	rigid->SetGameobject(sph);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph);

	pos = Vector3(1.5f, 0.f, 0.f);
	Sphere* sph_0 = new Sphere(pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	rigid = static_cast<Rigidbody3*>(sph_0->GetEntity());
	sph_0->m_physDriven = true;
	rigid->SetGameobject(sph_0);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph_0);

	// control groups
	ControlGroup* grp_0 = new ControlGroup(sph, sph_0, CONTROL_SPHERE_SPHERE);

	m_controlGrps.push_back(grp_0);

	// set the focused grp and the initial index
	m_focusedIndex = 0;
	m_focusedGrp = m_controlGrps[m_focusedIndex];

	// the number of cp view text is 5: id, intersected, contact point, normal and penetration
	m_cpView.resize(5, nullptr);

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

Collision3State::~Collision3State()
{
	if (m_title != nullptr)
	{
		delete m_title;
		m_title = nullptr;
	}

	DeleteVector(m_cpView);

	DeleteVector(m_controlGrps);
}

void Collision3State::Update(float deltaTime)
{
	UpdateDebugDraw(deltaTime);
	UpdateKeyboard(deltaTime);
	UpdateMouse(deltaTime);
	UpdateUI(deltaTime);
	UpdateFocusedGroup(deltaTime);
}


void Collision3State::UpdateMouse(float deltaTime)
{
	// Apply Rotation
	Vector2 mouseDelta = g_input->GetMouseDelta();

	// local euler offset, starts at zero
	Vector3 localEuler = Vector3::ZERO; 
	localEuler.y = mouseDelta.x * m_cameraRotationSpd * deltaTime; 
	localEuler.x = mouseDelta.y * m_cameraRotationSpd * deltaTime; 

	float camEulerPitch = ClampFloat(m_camera->GetTransform().GetLocalRotation().x + localEuler.x, -90.f, 90.f);
	float camEulerYaw = fmod(m_camera->GetTransform().GetLocalRotation().y + localEuler.y, 360.f);
	float camEulerRoll = m_camera->GetTransform().GetLocalRotation().z;

	m_camera->GetTransform().SetLocalRotation(Vector3(camEulerPitch, camEulerYaw, camEulerRoll));
	Matrix44 model = m_camera->GetTransform().GetLocalMatrixEulerTranspose();

	Matrix44 inverseModel = model.Invert();
	m_camera->SetView(inverseModel);
	//m_camera->GetView().Print();
}

void Collision3State::UpdateKeyboard(float deltaTime)
{
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Z))
	{
		// Reset camera position, euler and scale 
		m_camera->GetTransform().SetLocalPosition(m_cameraInitialPos);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}

	// Apply camera movement
	float leftRight = 0.f;
	float forwardBack = 0.f;
	float upDown = 0.f; 
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_A))
	{
		leftRight = -10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_D))
	{
		leftRight = 10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_W))
	{
		forwardBack = 10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_S))
	{
		forwardBack = -10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_Q))
	{
		upDown = 10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_E))
	{
		upDown = -10.f;
	}
	// camera update from input
	Vector3 camForward = m_camera->GetLocalForward(); 
	Vector3 camUp = m_camera->GetLocalUp(); 
	Vector3 camRight = m_camera->GetLocalRight(); 

	Vector3 worldOffset = camRight * leftRight * deltaTime
		+ camUp * upDown * deltaTime
		+ camForward * forwardBack * deltaTime;

	// Apply world offset
	m_camera->GetTransform().TranslateLocal(worldOffset); 
}

void Collision3State::UpdateUI(float deltaTime)
{
	const std::string id_str = m_focusedGrp->GetControlIDString();

	bool intersected = m_focusedGrp->IsIntersect();

	std::string intersect_str;
	std::string point_str;
	std::string normal_str;
	std::string penetration_str;
	
	if (intersected)
	{
		intersect_str = "YES";
		point_str = m_focusedGrp->GetPointString();
		normal_str = m_focusedGrp->GetNormalString();
		penetration_str = m_focusedGrp->GetPenetrationString();
	}
	else
	{
		intersect_str = "NO";
		point_str = "N/A";
		normal_str = "N/A";
		penetration_str = "N/A";
	}

	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	Vector2 min = m_titleMin - Vector2(0.f, m_textHeight);
	if (m_cpView[0] != nullptr)
	{
		delete m_cpView[0];
		m_cpView[0] = nullptr;
	}
	m_cpView[0] = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, id_str, VERT_PCU);
	
	min = min - Vector2(0.f, m_textHeight);
	if (m_cpView[1] != nullptr)
	{
		delete m_cpView[1];
		m_cpView[1] = nullptr;
	}
	Rgba color = intersected ? Rgba::GREEN : Rgba::RED;
	m_cpView[1] = Mesh::CreateTextImmediate(color, min, font, m_textHeight, .5f, intersect_str, VERT_PCU);

	min = min - Vector2(0.f, m_textHeight);
	if (m_cpView[2] != nullptr)
	{
		delete m_cpView[2];
		m_cpView[2] = nullptr;
	}
	m_cpView[2] = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, point_str, VERT_PCU);

	min = min - Vector2(0.f, m_textHeight);
	if (m_cpView[3] != nullptr)
	{
		delete m_cpView[3];
		m_cpView[3] = nullptr;
	}
	m_cpView[3] = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, normal_str, VERT_PCU);

	min = min - Vector2(0.f, m_textHeight);
	if (m_cpView[4] != nullptr)
	{
		delete m_cpView[4];
		m_cpView[4] = nullptr;
	}
	m_cpView[4] = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_textHeight, .5f, penetration_str, VERT_PCU);
}

void Collision3State::UpdateFocusedGroup(float deltaTime)
{
	m_focusedGrp->Update(deltaTime);
}

void Collision3State::UpdateDebugDraw(float deltaTime)
{
	DebugRenderUpdate(deltaTime);
}

void Collision3State::Render(Renderer* renderer)
{
	// draw UI
	renderer->SetCamera(m_UICamera);
	renderer->ClearScreen(Rgba::BLACK);
	DrawTextCut(m_title);
	DrawTexts(m_cpView);

	// draw group contents
	renderer->SetCamera(m_camera);
	m_focusedGrp->Render(renderer);
	//for (ControlGroup* cgrp : m_controlGrps)
	//	cgrp->Render(renderer);

	m_forwardPath->RenderScene(m_sceneGraph);
}


