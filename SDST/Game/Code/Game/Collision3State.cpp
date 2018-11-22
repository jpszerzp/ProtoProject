#include "Game/Collision3State.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
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

	// cp 1
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
	scale = Vector3(2.f, 2.f, 2.f);
	Sphere* sph_0 = new Sphere(pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	rigid = static_cast<Rigidbody3*>(sph_0->GetEntity());
	sph_0->m_physDriven = true;
	rigid->SetGameobject(sph_0);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph_0);

	ControlGroup* grp = new ControlGroup(sph, sph_0, CONTROL_SPHERE_SPHERE, m_cameraInitialPos);
	m_controlGrps.push_back(grp);

	// cp 2
	pos = Vector3(0.f, -5.f, 0.f);
	scale = Vector3::ONE;
	Sphere* sph_1 = new Sphere(pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	rigid = static_cast<Rigidbody3*>(sph_1->GetEntity());
	sph_1->m_physDriven = true;
	rigid->SetGameobject(sph_1);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph_1);

	pos = Vector3(0.f, -7.f, 0.f);
	rot = Vector3(90.f, 0.f, 0.f);
	scale = Vector3(2.f, 2.f, 2.f);
	stat = MOVE_KINEMATIC;
	Quad* pl = new Quad(pos, rot, scale, tint, "quad_pcu", matName, stat, bid, multipass, compare, CULLMODE_FRONT, wind, scheme);
	rigid = static_cast<Rigidbody3*>(pl->GetEntity());
	pl->m_physDriven = true;
	rigid->SetGameobject(pl);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(pl);

	ControlGroup* grp_0 = new ControlGroup(sph_1, pl, CONTROL_SPHERE_PLANE, Vector3(0.f, -6.f, -7.f));
	m_controlGrps.push_back(grp_0);

	// set the focused grp and the initial index
	m_focusedIndex = 0;
	m_focusedGrp = m_controlGrps[m_focusedIndex];

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

Collision3State::~Collision3State()
{
	DeleteVector(m_controlGrps);
}

void Collision3State::Update(float deltaTime)
{
	UpdateDebugDraw(deltaTime);
	UpdateKeyboard(deltaTime);
	UpdateMouse(deltaTime);
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		// switch focused control group
		++m_focusedIndex;
		int groupCount = m_controlGrps.size();
		m_focusedIndex = m_focusedIndex % groupCount;
		m_focusedGrp = m_controlGrps[m_focusedIndex];

		const Vector3& observation = m_focusedGrp->GetObservationPos();

		m_camera->GetTransform().SetLocalPosition(observation);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
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
	m_focusedGrp->RenderUI();

	// draw group contents
	renderer->SetCamera(m_camera);
	m_focusedGrp->RenderCore(renderer);

	m_forwardPath->RenderScene(m_sceneGraph);
}


