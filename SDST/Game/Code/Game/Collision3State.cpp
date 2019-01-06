#include "Game/Collision3State.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Box.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/HullObject.hpp"
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

	m_cameraInitialPos = Vector3(-110.f, 0.f, -7.f);
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
	Vector3 pos = Vector3(-111.5f, 0.f, 0.f);
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Rgba tint = Rgba::WHITE;
	std::string meshName = "sphere_pcu";
	std::string matName = "default";
	eMoveStatus stat = MOVE_KINEMATIC;
	eBodyIdentity bid = BODY_RIGID;
	bool multipass = false;
	eDepthCompare compare = COMPARE_LESS;
	eCullMode cull = CULLMODE_BACK;
	eWindOrder wind = WIND_COUNTER_CLOCKWISE;
	eDynamicScheme scheme = DISCRETE;
	Sphere* sph = new Sphere(1.f, pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	Rigidbody3* rigid = static_cast<Rigidbody3*>(sph->GetEntity());
	sph->m_physDriven = true;
	rigid->SetGameobject(sph);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph);

	pos = Vector3(-108.5f, 0.f, 0.f);
	scale = Vector3(2.f, 2.f, 2.f);
	Sphere* sph_0 = new Sphere(1.f, pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	rigid = static_cast<Rigidbody3*>(sph_0->GetEntity());
	sph_0->m_physDriven = true;
	rigid->SetGameobject(sph_0);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph_0);

	ControlGroup* grp = new ControlGroup(sph, sph_0, CONTROL_SPHERE_SPHERE, m_cameraInitialPos);
	m_controlGrps.push_back(grp);

	// cp 2
	pos = Vector3(100.f, -105.f, 0.f);
	scale = Vector3::ONE;
	Sphere* sph_1 = new Sphere(1.f, pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind, scheme);
	rigid = static_cast<Rigidbody3*>(sph_1->GetEntity());
	sph_1->m_physDriven = true;
	rigid->SetGameobject(sph_1);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph_1);

	pos = Vector3(100.f, -107.f, 0.f);
	rot = Vector3(90.f, 0.f, 0.f);
	scale = Vector3(4.f, 4.f, 1.f);
	stat = MOVE_KINEMATIC;
	Quad* pl = new Quad(pos, rot, scale, tint, "quad_pcu", matName, stat, bid, multipass, compare, CULLMODE_FRONT, wind, scheme);
	rigid = static_cast<Rigidbody3*>(pl->GetEntity());
	pl->m_physDriven = true;
	rigid->SetGameobject(pl);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(pl);

	ControlGroup* grp_0 = new ControlGroup(sph_1, pl, CONTROL_SPHERE_PLANE, Vector3(100.f, -106.f, -7.f));
	m_controlGrps.push_back(grp_0);

	// cp 3
	pos = Vector3(110.f, -105.f, 0.f);
	float rand_rot_x = GetRandomFloatInRange(0.f, 360.f);
	float rand_rot_y = GetRandomFloatInRange(0.f, 360.f);
	float rand_rot_z = GetRandomFloatInRange(0.f, 360.f);
	rot = Vector3(rand_rot_x, rand_rot_y, rand_rot_z);
	scale = Vector3::ONE;
	Box* b = new Box(pos, rot, scale, tint, "cube_pcu", "default", stat, bid, multipass, compare, cull, wind);
	rigid = static_cast<Rigidbody3*>(b->GetEntity());
	b->m_physDriven = true;
	rigid->SetGameobject(b);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(b);

	pos = Vector3(110.f, -107.f, 0.f);
	rot = Vector3(90.f, 0.f, 0.f);
	scale = Vector3(4.f, 4.f, 1.f);
	Quad* pl_0 = new Quad(pos, rot, scale, tint, "quad_pcu", matName, stat, bid, multipass, compare, CULLMODE_FRONT, wind, scheme);
	rigid = static_cast<Rigidbody3*>(pl_0->GetEntity());
	pl_0->m_physDriven = true;
	rigid->SetGameobject(pl_0);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(pl_0);

	ControlGroup* grp_1 = new ControlGroup(b, pl_0, CONTROL_BOX_PLANE, Vector3(110.f, -106.f, -7.f));
	m_controlGrps.push_back(grp_1);

	// cp 4
	pos = Vector3(110.f, 0.f, 0.f);
	rand_rot_x = GetRandomFloatInRange(0.f, 360.f);
	rand_rot_y = GetRandomFloatInRange(0.f, 360.f);
	rand_rot_z = GetRandomFloatInRange(0.f, 360.f);
	rot = Vector3(rand_rot_x, rand_rot_y, rand_rot_z);
	scale = Vector3::ONE;
	Box* b_0 = new Box(pos, rot, scale, tint, "cube_pcu", "default", stat, bid, multipass, compare, cull, wind);
	rigid = static_cast<Rigidbody3*>(b_0->GetEntity());
	b_0->m_physDriven = true;
	rigid->SetGameobject(b_0);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(b_0);

	pos = Vector3(112.f, 0.f, 0.f);
	rot = Vector3::ZERO;
	Sphere* sph_2 = new Sphere(1.f, pos, rot, scale, tint, meshName, matName, stat, bid, multipass, compare, cull, wind);
	rigid = static_cast<Rigidbody3*>(sph_2->GetEntity());
	sph_2->m_physDriven = true;
	rigid->SetGameobject(sph_2);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(sph_2);

	ControlGroup* grp_2 = new ControlGroup(b_0, sph_2, CONTROL_BOX_SPHERE, Vector3(111.f, 0.f, -7.f));
	m_controlGrps.push_back(grp_2);

	// cp 5
	pos = Vector3(120.f, 0.f, 0.f);
	rand_rot_x = GetRandomFloatInRange(0.f, 360.f);
	rand_rot_y = GetRandomFloatInRange(0.f, 360.f);
	rand_rot_z = GetRandomFloatInRange(0.f, 360.f);
	rot = Vector3(rand_rot_x, rand_rot_y, rand_rot_z);
	Box* b_1 = new Box(pos, rot, scale, tint, "cube_pcu", "default", stat, bid, multipass, compare, cull, wind);
	rigid = static_cast<Rigidbody3*>(b_1->GetEntity());
	b_1->m_physDriven = true;
	rigid->SetGameobject(b_1);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(b_1);

	pos = Vector3(122.f, 0.f, 0.f);
	rand_rot_x = GetRandomFloatInRange(0.f, 360.f);
	rand_rot_y = GetRandomFloatInRange(0.f, 360.f);
	rand_rot_z = GetRandomFloatInRange(0.f, 360.f);
	rot = Vector3(rand_rot_x, rand_rot_y, rand_rot_z);
	Box* b_2 = new Box(pos, rot, scale, tint, "cube_pcu", "default", stat, bid, multipass, compare, cull, wind);
	rigid = static_cast<Rigidbody3*>(b_2->GetEntity());
	b_2->m_physDriven = true;
	rigid->SetGameobject(b_2);
	rigid->SetAwake(true);
	rigid->SetCanSleep(false);
	m_gameObjects.push_back(b_2);

	ControlGroup* grp_3 = new ControlGroup(b_1, b_2, CONTROL_BOX_BOX, Vector3(121.f, 0.f, -7.f));
	m_controlGrps.push_back(grp_3);

	Vector3 start_line = Vector3(-150.f, 10.f, 0.f);
	Vector3 end_line = Vector3(-140.f, 10.f, 0.f);
	Line* l_1 = new Line(start_line, end_line, 5.f, Rgba::WHITE, "white");
	start_line = Vector3(-150.f, 7.f, 0.f);
	end_line = Vector3(-140.f, 7.f, 0.f);
	Line* l_2 = new Line(start_line, end_line, 5.f, Rgba::WHITE, "white");
	m_gameObjects.push_back(l_1);
	m_gameObjects.push_back(l_2);

	ControlGroup* grp_4 = new ControlGroup(l_1, l_2, CONTROL_LINE_LINE, Vector3(-145.f, 8.f, -5.f));
	m_controlGrps.push_back(grp_4);

	pos = Vector3(-55.f, 0.f, 0.f);
	rot = Vector3::ZERO;
	scale = Vector3::ONE;
	Vector3 hull_ext = Vector3(15.f, 15.f, 15.f);
	TODO("material needs to be adjusted so that it respects data imported from assimp");
	HullObject* h_1 = new HullObject(pos, hull_ext, rot, scale, tint, "white", stat, bid, 5, "", multipass, compare, cull, wind);
	pos = Vector3(55.f, 0.f, 0.f);
	rot = Vector3(45.f, 0.f, 0.f);
	HullObject* h_2 = new HullObject(pos, hull_ext, rot, scale, tint, "white", stat, bid, 5, "", multipass, compare, cull, wind);
	h_1->SetHullObject();
	h_2->SetHullObject();
	m_gameObjects.push_back(h_1);
	m_gameObjects.push_back(h_2);

	ControlGroup* grp_5 = new ControlGroup(h_1, h_2, CONTROL_HULL_HULL, Vector3(0.f, 0.f, -80.f));
	m_controlGrps.push_back(grp_5);

	// set the focused grp and the initial index
	m_focusedIndex = 0;
	m_focusedGrp = m_controlGrps[m_focusedIndex];

	// origin
	m_origin_mesh = Mesh::CreatePointImmediate(VERT_PCU, Vector3::ZERO, Rgba::MEGENTA);

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
		m_camera->GetTransform().SetLocalPosition(m_focusedGrp->GetObservationPos());
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		// switch focused control group
		++m_focusedIndex;
		int groupCount = (int)m_controlGrps.size();
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

	DrawPoint(m_origin_mesh);

	m_forwardPath->RenderScene(m_sceneGraph);
}


