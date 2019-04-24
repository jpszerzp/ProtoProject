#include "Game/Physics3State.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/Console/DevConsole.hpp"  
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"
#include "Engine/Physics/3D/BoxEntity3.hpp"
#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Physics/3D/QuadRB3.hpp"
#include "Engine/Physics/3D/RF/PhysTime.hpp"
#include "Engine/Physics/3D/RF/CollisionDetector.hpp"
#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Physics/3D/RF/ConvexPolyhedron.hpp"

#include <algorithm>

const Vector3 Physics3State::ORIGIN_CAMERA = Vector3(0.f, 0.f, -20.f);

bool IsGameobjectDead(GameObject* go) { return go->m_dead; }

Physics3State::Physics3State()
{
	Renderer* theRenderer = Renderer::GetInstance();
	m_sceneGraph = new RenderSceneGraph();
	m_forwardPath = new ForwardRenderPath();
	m_forwardPath->m_renderer = theRenderer;

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	m_cameraInitialPos = Vector3(75.f, 350.f, 45.f);
	m_cameraRotationSpd = 50.f;

	if (!m_camera)
	{
		m_camera = new Camera(m_cameraInitialPos);
		m_camera->SetColorTarget(theRenderer->GetDefaultColorTarget());
		m_camera->SetDepthStencilTarget(theRenderer->GetDefaultDepthTarget());
		m_camera->SetAspect(aspect);
		m_camera->SetFOV(45.f);

		m_camera->SetPerspectiveProjection(m_camera->GetFOV(), aspect, 1.f, 1000.f);

		m_sceneGraph->AddCamera(m_camera);
	}

	if (!m_UICamera)
	{
		m_UICamera = new Camera();
		m_UICamera->SetColorTarget(theRenderer->GetDefaultColorTarget());
		m_UICamera->SetDepthStencilTarget(theRenderer->GetDefaultDepthTarget());
		m_UICamera->SetAspect(aspect);

		// ortho projection
		m_UICamera->SetProjectionOrtho(window->GetWindowWidth(), window->GetWindowHeight(), 0.f, 100.f);
	}

	// refer to the array
	m_keep.m_collision_head = m_storage;

	// solver
	m_solver = CollisionSolver(5, .01f, .01f);

	// a plane
	m_wraparound_plane = new WrapAround(Vector3(20.f, 340.f, -10.f), Vector3(130.f, 400.f, 100.f));
	CollisionPlane* plane = new CollisionPlane(Vector2(110.f), Vector3(0.f, 1.f, 0.f), 342.f);
	CollisionRigidBody* rb = new CollisionRigidBody(1.f, Vector3(75.f, 342.f, 45.f), Vector3(90.f, 0.f, 0.f));
	rb->SetAwake(true);
	rb->SetSleepable(false);
	plane->AttachToRigidBody(rb);
	m_planes.push_back(plane);
	m_focus = plane;
	// do not include plane in wraparound

	// UI
	// local tensor is fixed
	// the format is motion - mass - tensor - velocity (to be modified)
	BitmapFont* font = theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = height / 50.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - txtHeight);
	titleMin -= Vector2(0.f, txtHeight);
	titleMin -= Vector2(0.f, txtHeight);

	std::string tensor_ui = Stringf("Local Tensor:");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
	m_tensor_ui.push_back(t_mesh);
	titleMin -= Vector2(0.f, txtHeight);

	if (m_focus)
	{
		const Matrix33& tensor_mat = m_focus->GetRigidBody()->GetTensor();
		tensor_ui = Stringf("%f, %f, %f", tensor_mat.GetI().x, tensor_mat.GetJ().x, tensor_mat.GetK().x);
		t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
		m_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);

		tensor_ui = Stringf("%f, %f, %f", tensor_mat.GetI().y, tensor_mat.GetJ().y, tensor_mat.GetK().y);
		t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
		m_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);

		tensor_ui = Stringf("%f, %f, %f", tensor_mat.GetI().z, tensor_mat.GetJ().z, tensor_mat.GetK().z);
		t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
		m_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);
	}

	// my demo
	m_wraparound_demo_0 = new WrapAround(Vector3(20.f, 300.f, -10.f), Vector3(40.f, 320.f, 10.f));

	// phsx demo
	m_wraparound_demo_1 = new WrapAround(Vector3(60.f, 300.f, -10.f), Vector3(80.f, 320.f, 10.f));
	m_corner_case_demo_pos = Vector3(50.f, 305.f, -30.f);

	// verlet
	m_wraparound_verlet = new WrapAround(Vector3(-10.f, 200.f, -10.f), Vector3(0.f, 250.f, 0.f));
	m_wraparound_verlet->m_particle = true;
	CollisionPoint* free = WrapAroundTestPoint(m_wraparound_verlet, false, false, true, Vector3(-7.f, 240.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	CollisionPoint* verlet_vel = WrapAroundTestPoint(m_wraparound_verlet, false, false, true, Vector3(-3.f, 240.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	free->GetRigidBody()->SetVerlet(false);		// no verlet scheme for it as it is not verlet particle
	free->GetRigidBody()->SetFrozen(true);
	verlet_vel->GetRigidBody()->SetVerlet(true);
	verlet_vel->GetRigidBody()->SetVerletScheme(VERLET_VEL_P);
	verlet_vel->GetRigidBody()->SetFrozen(true);

	// inspection
	m_inspection.push_back(m_cameraInitialPos);
	m_inspection.push_back(m_corner_case_demo_pos);
	m_inspection.push_back(Vector3(-5.f, 220.f, -20.f));
	m_inspection.push_back(Vector3(200.f, 200.f, -20.f));

	// force registry 
	m_particleRegistry = new ParticleForceRegistry();
	m_rigidRegistry = new RigidForceRegistry();

	// particle springs
	CollisionPoint* sp_point_0 = WrapAroundTestPoint(nullptr, false, false, false,
		Vector3(10.f, 225.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	CollisionPoint* sp_point_1 = WrapAroundTestPoint(nullptr, false, false, false,
		Vector3(10.f, 235.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	CollisionPoint* asp_point_0 = WrapAroundTestPoint(nullptr, false, false, false,
		Vector3(15.f, 225.f, -5.f), Vector3::ZERO, Vector3(10.f), true, true);
	CollisionPoint* asp_point_1 = WrapAroundTestPoint(nullptr, false, false, false,
		Vector3(15.f, 235.f, -5.f), Vector3::ZERO, Vector3(10.f), true, true);
	// setting up registrations in the registry
	// A. springs
	m_spring = SetupSpring(sp_point_1, sp_point_0, 2.f, 9.5f);		
	// two points initialized to be 5 units away, constraint by a spring system with rest length of 3
	// B. anchored spring, attched then anchor
	m_anchorSpring = SetupAnchorSpring(asp_point_0, asp_point_1, 3.f, 8.f);

	// anchor
	m_spr_anchor = WrapAroundTestPoint(nullptr, false, false, false, Vector3(20.f, 235.f, -5.f), Vector3::ZERO, Vector3(10.f), true, true);
	m_spr_anchor->SetFrozen(true);
	m_spr_anchor->GetRigidBody()->SetVerlet(false);
	// attched
	m_spr_attach = WrapAroundTestSphere(nullptr, false, false, false, Vector3(20.f, 225.f, -5.f), Vector3::ZERO, Vector3::ONE, 10.f);
	m_spr_attach->SetFrozen(true);
	// entities
	CollisionRigidBody* anchor_rb = m_spr_anchor->GetRigidBody();
	CollisionRigidBody* attached_rb = m_spr_attach->GetRigidBody();
	// set up anchor spring
	m_rigidAnchorSpring = new GeneralRigidAnchorSpring(attached_rb, anchor_rb, 50.f, 11.f);
	// force generator
	Vector3 attach_local = Vector3::ZERO;
	ProjectPlaneToSphere(Vector2(.01f, .01f), m_spr_attach->GetRadius(), attach_local);
	GravityRigidForceGenerator* grg = new GravityRigidForceGenerator(Vector3::GRAVITY / 80.f);
	AnchorSpringRigidForceGenerator* asrfg = new AnchorSpringRigidForceGenerator(m_spr_anchor->GetCenter(), attached_rb, attach_local, 50.f, 11.f);
	// force registration
	m_rigidRegistry->Register(attached_rb, grg);
	m_rigidRegistry->Register(attached_rb, asrfg);

	// continuity demo
	m_wraparound_ccd = new WrapAround(Vector3(190.f, 190.f, -10.f), Vector3(210.f, 210.f, 10.f));
	m_wraparound_ccd->m_particle = true;					// just so the primitive will wrap around
	m_discrete_ball_pos = Vector3(195.f, 195.f, 0.f);
	m_discrete_ball = WrapAroundTestSphere(m_wraparound_ccd, false, false, false, 
		m_discrete_ball_pos, Vector3::ZERO, Vector3(.5f), 1.f, 
		"wireframe", "Data/Images/white.png");
	m_discrete_ball->GetRigidBody()->SetFrozen(true);

	m_ccd_ball_pos = Vector3(195.f, 205.f, 0.f);
	m_ccd_ball = WrapAroundTestSphere(m_wraparound_ccd, false, false, false,
		m_ccd_ball_pos, Vector3::ZERO, Vector3(.5f), 1.f, 
		"wireframe", "Data/Images/white.png");
	m_ccd_ball->SetContinuity(COL_CCD);
	m_ccd_ball->GetRigidBody()->SetFrozen(true);

	CollisionPlane* plane_ccd = new CollisionPlane(Vector2(20.f), Vector3(-1.f, 0.f, 0.f), -205.f, "wireframe", "Data/Images/white.png");
	CollisionRigidBody* pl_rb_ccd = new CollisionRigidBody(1.f, Vector3(205.f, 200.f, 0.f), Vector3(0.f, 90.f, 0.f));
	pl_rb_ccd->SetAwake(true);
	pl_rb_ccd->SetSleepable(true);
	plane_ccd->AttachToRigidBody(pl_rb_ccd);
	m_planes.push_back(plane_ccd);
	// no need to include the plane in this wraparound

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

Physics3State::~Physics3State()
{
	delete m_wraparound_plane;
	m_wraparound_plane = nullptr;

	delete m_wraparound_demo_0;
	m_wraparound_demo_0 = nullptr;

	delete m_wraparound_demo_1;
	m_wraparound_demo_1 = nullptr;

	delete m_wraparound_verlet;
	m_wraparound_verlet = nullptr;

	delete m_wraparound_ccd;
	m_wraparound_ccd = nullptr;

	delete m_rigidRegistry;
	m_rigidRegistry = nullptr;

	delete m_particleRegistry;
	m_particleRegistry = nullptr;
}

void Physics3State::PostConstruct()
{
	m_wraparound_plane->m_physState = this;
	m_wraparound_demo_0->m_physState = this;
	m_wraparound_demo_1->m_physState = this;
	m_wraparound_verlet->m_physState = this;
	m_wraparound_ccd->m_physState = this;
}

void Physics3State::Update(float deltaTime)
{
	UpdateInput(deltaTime);				// update input
	UpdateForceRegistries(deltaTime);
	UpdateGameobjects(deltaTime);		// update gameobjects
	UpdateContacts(deltaTime);
	UpdateDebug(deltaTime);			
	UpdateUI();
	UpdateDelete();

	g_theApp->PhysxUpdate(deltaTime);
}

void Physics3State::UpdateMouse(float deltaTime)
{
	if (!IsProfilerOn())
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
}

void Physics3State::UpdateKeyboard(float deltaTime)
{
	// Apply movement
	float leftRight = 0.f;
	float forwardBack = 0.f;
	float upDown = 0.f; 

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_A))
	{
		leftRight = -50.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_D))
	{
		leftRight = 50.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_W))
	{
		forwardBack = 50.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_S))
	{
		forwardBack = -50.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_Q))
	{
		upDown = 50.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_E))
	{
		upDown = -50.f;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Z))
	{
		// Reset camera position, euler and scale 
		m_camera->GetTransform().SetLocalPosition(m_cameraInitialPos);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_V))
	{
		// Reset camera position, euler and scale 
		m_camera->GetTransform().SetLocalPosition(ORIGIN_CAMERA);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_R))
	{
		m_camera->GetTransform().SetLocalPosition(m_corner_case_demo_pos);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_F))
	{
		delete m_corner_case_1;
		m_corner_case_1 = nullptr;

		delete m_corner_case_2;
		m_corner_case_2 = nullptr;
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_7))
		SpawnRandomConvex(m_wraparound_plane, 5, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 360.f, 100.f));

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_8))
		SpawnRandomSphere(m_wraparound_plane, 5, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 360.f, 100.f));

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_9))
		SpawnRandomBox(m_wraparound_plane, 5, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 360.f, 100.f));

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE))
		ShootBox(m_wraparound_plane);
	
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_CONTROL))
		ShootSphere(m_wraparound_plane);

	// slow
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_0))
	{
		for (CollisionPrimitive* primitive : m_wraparound_plane->m_primitives)
			primitive->GetRigidBody()->SetSlow(.01f);

		for (CollisionPrimitive* primitive : m_wraparound_demo_0->m_primitives)
			primitive->GetRigidBody()->SetSlow(.01f);
	}
	else
	{
		for (CollisionPrimitive* primitive : m_wraparound_plane->m_primitives)
			primitive->GetRigidBody()->SetSlow(1.f);

		for (CollisionPrimitive* primitive : m_wraparound_demo_0->m_primitives)
			primitive->GetRigidBody()->SetSlow(1.f);
	}

	// physx stack
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_MINUS))
		g_theApp->SpawnPhysxStack(Vector3(100.f, 342.5f, 45.f), 5, 5);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_PLUS))
	{
		for (int i = 0; i < g_theApp->m_physx_stack.size(); ++i)
			g_theApp->m_physx_stack[i]->SetShouldDelete(true);
		g_theApp->m_physx_stack.clear();
	}

	// anchor rigid spring
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_1))
	{
		m_spr_anchor->SetFrozen(!m_spr_anchor->IsFrozen());
		m_spr_attach->SetFrozen(!m_spr_attach->IsFrozen());
	}

	// my stack
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_4))
		SpawnStack(Vector3(75.f, 342.5f, 45.f), 5, 5);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_6))
	{
		for (int i = 0; i < m_my_stack.size(); ++i)
			m_my_stack[i]->SetShouldDelete(true);
		m_my_stack.clear();
	}

	// physx corner case demo
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0))
	{
		m_phys_corner_case = FCC_FF;
		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_FF_3, CORNER_CASE_POS_FF_4, CORNER_CASE_ORIENT_FF_3, CORNER_CASE_ORIENT_FF_4);
		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);
		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_1))
	{
		m_phys_corner_case = FCC_FP;
		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_FP_3, CORNER_CASE_POS_FP_4, CORNER_CASE_ORIENT_FP_3, CORNER_CASE_ORIENT_FP_4);
		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);
		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_5))
	{
		m_phys_corner_case = FCC_FE;
		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_FE_3, CORNER_CASE_POS_FE_4, CORNER_CASE_ORIENT_FE_3, CORNER_CASE_ORIENT_FE_4);
		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);
		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_2))
	{
		m_phys_corner_case = FCC_PP;
		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_PP_3, CORNER_CASE_POS_PP_4, CORNER_CASE_ORIENT_PP_3, CORNER_CASE_ORIENT_PP_4);
		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);
		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3))
	{
		m_phys_corner_case = FCC_EE;
		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_EE_3, CORNER_CASE_POS_EE_4, CORNER_CASE_ORIENT_EE_3, CORNER_CASE_ORIENT_EE_4);
		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);
		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_4))
	{
		m_phys_corner_case = FCC_PE;
		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_PE_3, CORNER_CASE_POS_PE_4, CORNER_CASE_ORIENT_PE_3, CORNER_CASE_ORIENT_PE_4);
		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);
		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_1))
	{
		m_ff_test = true; m_fp_test = false; m_pp_test = false; m_ee_test = false; m_pe_test = false; m_fe_test = false;
		ResetCollisionCornerCase(CORNER_CASE_POS_FF_1, CORNER_CASE_POS_FF_2, CORNER_CASE_ORIENT_FF_1, CORNER_CASE_ORIENT_FF_2);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_2))
	{
		m_fp_test = true; m_ff_test = false; m_pp_test = false; m_ee_test = false; m_pe_test = false; m_fe_test = false;
		ResetCollisionCornerCase(CORNER_CASE_POS_FP_1, CORNER_CASE_POS_FP_2, CORNER_CASE_ORIENT_FP_1, CORNER_CASE_ORIENT_FP_2);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_3))
	{
		m_pp_test = true; m_ff_test = false; m_fp_test = false; m_ee_test = false; m_pe_test = false; m_fe_test = false;
		ResetCollisionCornerCase(CORNER_CASE_POS_PP_1, CORNER_CASE_POS_PP_2, CORNER_CASE_ORIENT_PP_1, CORNER_CASE_ORIENT_PP_2);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_4))
	{
		m_ee_test = true; m_pp_test = false; m_ff_test = false; m_fp_test = false; m_pe_test = false; m_fe_test = false;
		ResetCollisionCornerCase(CORNER_CASE_POS_EE_1, CORNER_CASE_POS_EE_2, CORNER_CASE_ORIENT_EE_1, CORNER_CASE_ORIENT_EE_2);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_5))
	{
		m_pe_test = true; m_ee_test = false; m_pp_test = false; m_ff_test = false; m_fp_test = false; m_fe_test = false;
		ResetCollisionCornerCase(CORNER_CASE_POS_PE_1, CORNER_CASE_POS_PE_2, CORNER_CASE_ORIENT_PE_1, CORNER_CASE_ORIENT_PE_2);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_6))
	{
		m_fe_test = true; m_pe_test = false; m_ee_test = false; m_pp_test = false; m_ff_test = false; m_fp_test = false; 
		ResetCollisionCornerCase(CORNER_CASE_POS_FE_1, CORNER_CASE_POS_FE_2, CORNER_CASE_ORIENT_FE_1, CORNER_CASE_ORIENT_FE_2);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		if (!m_inspection.empty())
		{
			Vector3 pos = m_inspection[m_insepction_count];

			// set camera pos to this inspection position
			m_camera->GetTransform().SetLocalPosition(pos);
			m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);

			size_t size = m_inspection.size();
			++m_insepction_count;
			m_insepction_count = (m_insepction_count % size);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Y))
	{
		// delete all in wpa plane (or the most general wpa)
		for (int i = 0; i < m_wraparound_plane->m_primitives.size(); ++i)
			m_wraparound_plane->m_primitives[i]->SetShouldDelete(true);

		m_wraparound_plane->m_primitives.clear();
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_I))
	{
		// to fire the corner case test, give opposite velocities
		if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_ff_test)
		{
			m_corner_case_1->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_FF_1);
			m_corner_case_2->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_FF_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_fp_test)
		{
			m_corner_case_1->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_FP_1);
			m_corner_case_2->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_FP_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_pp_test)
		{
			m_corner_case_1->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_PP_1);
			m_corner_case_2->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_PP_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_ee_test)
		{
			m_corner_case_1->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_EE_1);
			m_corner_case_2->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_EE_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_pe_test)
		{
			m_corner_case_1->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_PE_1);
			m_corner_case_2->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_PE_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_fe_test)
		{
			m_corner_case_1->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_FE_1);
			m_corner_case_2->m_cmp->SetLinearVel(CORNER_CASE_LIN_VEL_FE_2);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_U))
	{
		if (m_phys_corner_case == FCC_FF)
		{
			g_theApp->m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_FF_3);
			g_theApp->m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_FF_4);
		}
		else if (m_phys_corner_case == FCC_FP)
		{
			g_theApp->m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_FP_3);
			g_theApp->m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_FP_4);
		}
		else if (m_phys_corner_case == FCC_FE)
		{
			g_theApp->m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_FE_3);
			g_theApp->m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_FE_4);
		}
		else if (m_phys_corner_case == FCC_PP)
		{
			g_theApp->m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_PP_3);
			g_theApp->m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_PP_4);
		}
		else if (m_phys_corner_case == FCC_EE)
		{
			g_theApp->m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_EE_3);
			g_theApp->m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_EE_4);
		}
		else if (m_phys_corner_case == FCC_PE)
		{
			g_theApp->m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_PE_3);
			g_theApp->m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_PE_4);
		}
	}

	// ccd demo
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_L))
	{
		m_discrete_ball->SetRigidBodyPosition(m_discrete_ball_pos);
		m_discrete_ball->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);
		m_discrete_ball->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);
		m_discrete_ball->SetFrozen(true);

		m_ccd_ball->SetRigidBodyPosition(m_ccd_ball_pos);
		m_ccd_ball->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);
		m_ccd_ball->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);
		m_ccd_ball->SetFrozen(true);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_N))
	{
		m_discrete_ball->GetRigidBody()->SetLinearVelocity(Vector3(500.f, 0.f, 0.f));
		m_discrete_ball->SetFrozen(false);

		m_ccd_ball->GetRigidBody()->SetLinearVelocity(Vector3(500.f, 0.f, 0.f));
		m_ccd_ball->SetFrozen(false);
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

void Physics3State::UpdateInput(float deltaTime)
{
	if (!DevConsoleIsOpen())
	{
		UpdateMouse(deltaTime);
		UpdateKeyboard(deltaTime);
	}
}

void Physics3State::UpdateGameobjects(float deltaTime)
{
	UpdateGameobjectsCore(deltaTime);		// update GO core
}

void Physics3State::UpdateDebug(float deltaTime)
{
	UpdateDebugDraw(deltaTime);
}

void Physics3State::UpdateDebugDraw(float deltaTime)
{
	DebugRenderUpdate(deltaTime);
}


void Physics3State::UpdateWrapArounds()
{
	m_wraparound_plane->Update();
	m_wraparound_demo_0->Update();
	m_wraparound_demo_1->Update();
	m_wraparound_verlet->Update();
	m_wraparound_ccd->Update();
}


void Physics3State::UpdateUI()
{
	// motion
	Renderer* theRenderer = Renderer::GetInstance();
	Window* window = Window::GetInstance();
	float window_height = window->GetWindowHeight();
	float window_width = window->GetWindowWidth();
	BitmapFont* font = theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = window_height / 70.f;
	Vector2 titleMin = Vector2(-window_width / 2.f, window_height / 2.f - txtHeight);

	delete m_motion_ui;
	m_motion_ui = nullptr;

	if (m_focus)
	{
		std::string motion_ui = Stringf("Motion of focused: %f", m_focus->GetRigidBody()->GetRealTimeMotion());
		m_motion_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, motion_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		// mass
		delete m_mass_ui;
		m_mass_ui = nullptr;

		std::string mass_ui = Stringf("Mass: %f", m_focus->GetRigidBody()->GetMass());
		m_mass_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, mass_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		// considering tensor takes 4 lines
		for (int i = 0; i < 4; ++i)
			titleMin -= Vector2(0.f, txtHeight);

		// velocity
		delete m_vel_ui;
		m_vel_ui = nullptr;

		const Vector3& lin_vel = m_focus->GetRigidBody()->GetLinearVelocity();
		const Vector3& ang_vel = m_focus->GetRigidBody()->GetAngularVelocity();
		std::string vel_ui = Stringf("Linear and angular velocity of the focused: (%f, %f, %f), (%f, %f, %f)", 
			lin_vel.x, lin_vel.y, lin_vel.z, ang_vel.x, ang_vel.y, ang_vel.z);
		m_vel_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, vel_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);
	}
}

void Physics3State::UpdateDelete()
{
	// box
	for (int i = 0; i < m_boxes.size(); ++i)
	{
		CollisionBox* box = m_boxes[i];
		if (box->ShouldDelete())
		{
			// obj vector
			std::vector<CollisionBox*>::iterator it = m_boxes.begin() + i;
			m_boxes.erase(it);

			// check each wrap arounds, since we do not know which wraparound this obj is in (could be any)
			m_wraparound_plane->RemovePrimitive(box);
			m_wraparound_demo_0->RemovePrimitive(box);
			m_wraparound_demo_1->RemovePrimitive(box);

			// need to update my stack vector
			for (int j = 0; j < m_my_stack.size(); ++j)
			{
				if (m_my_stack[j] == box)
				{
					std::vector<CollisionBox*>::iterator it_t = m_my_stack.begin() + j;
					m_my_stack.erase(it_t);
					j--;
					
					break;		// deleted box found in stack, abort
				}
			}

			// is focused?
			if (m_focus == box)
				m_focus = nullptr;

			// if corner case
			if (m_corner_case_1 == box)
				m_corner_case_1 = nullptr;
			if (m_corner_case_2 == box)
				m_corner_case_2 = nullptr;

			delete box;
			i = i - 1;
		}
	}

	// spheres
	for (int i = 0; i < m_spheres.size(); ++i)
	{
		CollisionSphere* sph = m_spheres[i];
		if (sph->ShouldDelete())
		{
			// obj vector
			std::vector<CollisionSphere*>::iterator it = m_spheres.begin() + i;
			m_spheres.erase(it);

			m_wraparound_plane->RemovePrimitive(sph);
			m_wraparound_demo_0->RemovePrimitive(sph);
			m_wraparound_demo_1->RemovePrimitive(sph);

			delete sph;
			i = i - 1;
		}
	}

	// convex
	for (int i = 0; i < m_convex_objs.size(); ++i)
	{
		CollisionConvexObject* cobj = m_convex_objs[i];
		if (cobj->ShouldDelete())
		{
			std::vector<CollisionConvexObject*>::iterator it = m_convex_objs.begin() + i;
			m_convex_objs.erase(it);

			m_wraparound_plane->RemovePrimitive(cobj);
			m_wraparound_demo_0->RemovePrimitive(cobj);
			m_wraparound_demo_1->RemovePrimitive(cobj);

			delete cobj;
			i = i - 1;
		}
	}
}

void Physics3State::UpdateForceRegistries(float dt)
{
	if (m_particleRegistry != nullptr)
		m_particleRegistry->UpdateForces(dt);

	if (m_rigidRegistry != nullptr)
		m_rigidRegistry->UpdateForces(dt);
}

void Physics3State::UpdateGameobjectsCore(float deltaTime)
{
	UpdateGameobjectsDynamics(deltaTime);

	UpdateWrapArounds();
}

void Physics3State::UpdateGameobjectsDynamics(float deltaTime)
{
	for (std::vector<CollisionSphere*>::size_type idx = 0; idx < m_spheres.size(); ++idx)
		m_spheres[idx]->Update(deltaTime);

	for (std::vector<CollisionBox*>::size_type idx = 0; idx < m_boxes.size(); ++idx)
		m_boxes[idx]->Update(deltaTime);

	for (std::vector<CollisionPlane*>::size_type idx = 0; idx < m_planes.size(); ++idx)
		m_planes[idx]->Update(deltaTime);

	for (std::vector<CollisionConvexObject*>::size_type idx = 0; idx < m_convex_objs.size(); ++idx)
		m_convex_objs[idx]->Update(deltaTime);

	for (std::vector<CollisionPoint*>::size_type idx = 0; idx < m_points.size(); ++idx)
		m_points[idx]->Update(deltaTime);
}

void Physics3State::UpdateContacts(float deltaTime)
{
	UpdateContactGeneration();				// update contact generation

	UpdateContactResolution(deltaTime);		// contact resolution
}

void Physics3State::UpdateContactGeneration()
{
	// set up collision keep
	m_keep.Reset(MAX_CONTACT_NUM);
	m_keep.m_tolerance = .1f;

	// generate collisions
	for (std::vector<CollisionSphere*>::size_type idx0 = 0; idx0 < m_spheres.size(); ++idx0)
	{
		CollisionSphere* sph0 = m_spheres[idx0];

		// sphere vs sphere
		for (std::vector<CollisionSphere*>::size_type idx1 = idx0 + 1; idx1 < m_spheres.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionSphere* sph1 = m_spheres[idx1];

			// spawn collisions in the keep
			CollisionSensor::SphereVsSphere(*sph0, *sph1, &m_keep);
		}

		// sphere vs plane
		for (std::vector<CollisionPlane*>::size_type idx1 = 0; idx1 < m_planes.size(); ++idx1)
		{
			if (sph0->GetContinuity() != COL_CCD)
			{
				if (!m_keep.AllowMoreCollision())
					return;

				CollisionPlane* pl = m_planes[idx1];

				CollisionSensor::SphereVsPlane(*sph0, *pl, &m_keep);
			}
			else
			{
				// we do not put data into keep here, we want the ccd ball to get stuck
				CollisionPlane* pl = m_planes[idx1];

				// hit time and position
				// t is normalized 0 to 1, p is on the plane
				float t = 0.f;
				Vector3 p = Vector3::ZERO;
				const Vector3& v = sph0->GetRigidBody()->GetLinearVelocity();
				uint collided_ccd = CollisionSensor::SphereVsPlaneContinuous(*sph0, *pl, v, t, p, &m_keep);

				if (collided_ccd != 0)
				{
					Vector3 next_frame = sph0->GetCenter() + v * t;
					sph0->SetNextFrameTeleport(next_frame);
				}
			}
		}
	}

	for (std::vector<CollisionBox*>::size_type idx0 = 0; idx0 < m_boxes.size(); ++idx0)
	{
		CollisionBox* box0 = m_boxes[idx0];

		// box vs box
		for (std::vector<CollisionBox*>::size_type idx1 = idx0 + 1; idx1 < m_boxes.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionBox* box1 = m_boxes[idx1];

			CollisionSensor::BoxVsBox(*box0, *box1, &m_keep);
		}

		// box vs plane
		for (std::vector<CollisionPlane*>::size_type idx1 = 0; idx1 < m_planes.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionPlane* pl = m_planes[idx1];

			CollisionSensor::BoxVsHalfPlane(*box0, *pl, &m_keep);
		} 

		// box vs sphere
		for (std::vector<CollisionSphere*>::size_type idx1 = 0; idx1 < m_spheres.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionSphere* sph = m_spheres[idx1];

			CollisionSensor::BoxVsSphere(*box0, *sph, &m_keep);
		}
	}

	for (std::vector<CollisionConvexObject*>::size_type idx0 = 0; idx0 < m_convex_objs.size(); ++idx0)
	{
		CollisionConvexObject* c_obj_0 = m_convex_objs[idx0];

		// convex vs plane
		for (std::vector<CollisionPlane*>::size_type idx1 = 0; idx1 < m_planes.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionPlane* pl = m_planes[idx1];

			CollisionSensor::ConvexVsHalfPlane(*c_obj_0, *pl, &m_keep);
		}

		/*
		// ...vs box
		for (std::vector<CollisionBox*>::size_type idx1 = 0; idx1 < m_boxes.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionBox* bx = m_boxes[idx1];

			CollisionSensor::ConvexVsBox(*c_obj_0, *bx, &m_keep);
		}
		*/
		
		/*
		// ...vs sphere
		for (std::vector<CollisionSphere*>::size_type idx1 = 0; idx1 < m_spheres.size(); ++idx1)
		{
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionSphere* sph = m_spheres[idx1];

			CollisionSensor::ConvexVsSphere(*c_obj_0, *sph, &m_keep);
		}
		*/
	}
}


void Physics3State::UpdateContactResolution(float deltaTime)
{
	m_solver.SolveCollision(m_keep.m_collision_head, m_keep.m_collision_count, deltaTime);
}


void Physics3State::Render(Renderer* renderer)
{
	renderer->SetCamera(m_UICamera);
	renderer->ClearScreen(Rgba::BLACK);

	RenderUI(renderer);

	renderer->SetCamera(m_camera);

	RenderGameobjects(renderer);

	RenderForwardPath(renderer);

	RenderWrapArounds(renderer);

	g_theApp->PhysxRender(renderer);
}

void Physics3State::RenderGameobjects(Renderer* renderer)
{
	for (std::vector<CollisionSphere*>::size_type idx = 0; idx < m_spheres.size(); ++idx)
		m_spheres[idx]->Render(renderer);

	for (std::vector<CollisionBox*>::size_type idx = 0; idx < m_boxes.size(); ++idx)
		m_boxes[idx]->Render(renderer);

	for (std::vector<CollisionPlane*>::size_type idx = 0; idx < m_planes.size(); ++idx)
		m_planes[idx]->Render(renderer);

	for (std::vector<CollisionConvexObject*>::size_type idx = 0; idx < m_convex_objs.size(); ++idx)
		m_convex_objs[idx]->Render(renderer);

	for (std::vector<CollisionPoint*>::size_type idx = 0; idx < m_points.size(); ++idx)
		m_points[idx]->Render(renderer);
}

void Physics3State::RenderWrapArounds(Renderer* renderer)
{
	m_wraparound_plane->Render(renderer);
	m_wraparound_demo_0->Render(renderer);
	m_wraparound_demo_1->Render(renderer);
	m_wraparound_verlet->Render(renderer);
	m_wraparound_ccd->Render(renderer);
}

void Physics3State::RenderForwardPath(Renderer*)
{
	m_forwardPath->RenderScene(m_sceneGraph);
}

void Physics3State::RenderUI(Renderer*)
{
	DrawTextCut(m_motion_ui);
	DrawTextCut(m_mass_ui);

	for (int i = 0; i < m_tensor_ui.size(); ++i)
		DrawTextCut(m_tensor_ui[i]);

	DrawTextCut(m_vel_ui);
}

CollisionSphere* Physics3State::WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, 
	bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, 
	const Vector3& scale, float mass, const std::string& fp, const std::string& tx)
{
	CollisionSphere* sph = new CollisionSphere(scale.x, fp, tx);

	CollisionRigidBody* rb = new CollisionRigidBody(mass, position, rot);
	rb->SetAwake(true);
	rb->SetSleepable(false);

	sph->AttachToRigidBody(rb);

	m_spheres.push_back(sph);

	if(register_g)
	{
		Vector3 gravity = (Vector3::GRAVITY / 2.f);
		rb->SetBaseLinearAcceleration(gravity);
	}

	if (give_lin_vel)
		rb->SetLinearVelocity(GetRandomVector3() * 5.f);

	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rb->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}

	if (wpa != nullptr)
		wpa->m_primitives.push_back(sph);

	return sph;
}

CollisionBox* Physics3State::WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, 
	bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale, const bool& awake, const bool& sleepable)
{
	CollisionBox* box = new CollisionBox(scale / 2.f);

	CollisionRigidBody* rb = new CollisionRigidBody(1.f, position, rot);
	rb->SetAwake(awake);
	rb->SetSleepable(sleepable);

	box->AttachToRigidBody(rb);

	m_boxes.push_back(box);

	if(register_g)
	{
		Vector3 gravity = (Vector3::GRAVITY / 2.f);
		rb->SetBaseLinearAcceleration(gravity);
	}

	if (give_lin_vel)
		rb->SetLinearVelocity(GetRandomVector3() * 5.f);

	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rb->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}

	wpa->m_primitives.push_back(box);

	return box;
}

CollisionConvexObject* Physics3State::WrapAroundTestConvex(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, 
	const Vector3& position, const Vector3&, const Vector3&, const bool& awake, const bool& sleepable)
{
	Plane p1 = Plane(Vector3(.1f, 0.f, .9f), 1.f);
	Plane p2 = Plane(Vector3(.1f, 0.f, -.9f), 1.f);
	Plane p3 = Plane(Vector3(-.9f, 0.f, -.1f), 1.f);
	Plane p4 = Plane(Vector3(.9f, 0.f, -.1f), 1.f);
	Plane p5 = Plane(Vector3(-.1f, .9f, -.1f), 1.f);
	Plane p6 = Plane(Vector3(-.05f, -.9f, .1f), 1.f);

	std::vector<Plane> hull_planes;
	hull_planes.push_back(p1);
	hull_planes.push_back(p2);
	hull_planes.push_back(p3);
	hull_planes.push_back(p4);
	hull_planes.push_back(p5);
	hull_planes.push_back(p6);
	ConvexHull* cHull = new ConvexHull(hull_planes);

	CollisionConvexObject* cObj = new CollisionConvexObject(*cHull);

	const float& mass = cObj->GetInitialMass();
	CollisionRigidBody* rb = new CollisionRigidBody(mass, position, Vector3::ZERO);
	rb->SetAwake(awake);
	rb->SetSleepable(sleepable);

	cObj->AttachToRigidBody(rb);

	m_convex_objs.push_back(cObj);

	if(register_g)
	{
		Vector3 gravity = (Vector3::GRAVITY / 2.f);
		rb->SetBaseLinearAcceleration(gravity);
	}

	if (give_lin_vel)
		rb->SetLinearVelocity(GetRandomVector3() * 5.f);

	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rb->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}

	wpa->m_primitives.push_back(cObj);

	return cObj;
}

CollisionPoint* Physics3State::WrapAroundTestPoint(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale, const bool& awake /*= true*/, const bool& sleepable /*= false*/)
{
	// pt scale should be uniform
	CollisionPoint* pt = new CollisionPoint(scale.x);

	CollisionRigidBody* rb = new CollisionRigidBody(5.f, position, rot);
	rb->SetParticle(true);
	rb->SetAwake(awake);
	rb->SetSleepable(sleepable);

	pt->AttachToRigidBody(rb);

	m_points.push_back(pt);

	if(register_g)
	{
		Vector3 gravity = (Vector3::GRAVITY / 2.f);
		rb->SetBaseLinearAcceleration(gravity);
	}

	if (give_lin_vel)
		rb->SetLinearVelocity(GetRandomVector3() * 5.f);

	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rb->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}

	// add to primitive vector in wpa
	if (wpa != nullptr)
		wpa->m_primitives.push_back(pt);

	return pt;
}

Spring* Physics3State::SetupSpring(CollisionPoint* end1, CollisionPoint* end2, float coef, float rl)
{
	Spring* sp = new Spring(end1, end2, coef, rl);

	// get the entities of points
	CollisionRigidBody* e1 = end1->GetRigidBody();
	CollisionRigidBody* e2 = end2->GetRigidBody();

	// use euler
	e1->SetVerlet(false);
	e2->SetVerlet(false);

	e1->SetFrozen(true);
	e2->SetFrozen(true);

	// initialize spring force generators
	SpringGenerator* sg1 = new SpringGenerator(e2, coef, rl);
	SpringGenerator* sg2 = new SpringGenerator(e1, coef, rl);

	// register the generator with corresponding entity
	m_particleRegistry->Register(e1, sg1);
	m_particleRegistry->Register(e2, sg2);

	return sp;
}

AnchorSpring* Physics3State::SetupAnchorSpring(CollisionPoint* end1, CollisionPoint* end2, float coef, float rl)
{
	AnchorSpring* asp = new AnchorSpring(end1, end2, coef, rl);

	// get entities 
	CollisionRigidBody* e1 = end1->GetRigidBody();
	CollisionRigidBody* e2 = end2->GetRigidBody();

	// use euler
	e1->SetVerlet(false);
	e2->SetVerlet(false);

	e1->SetFrozen(true);
	e2->SetFrozen(true);

	// anchored spring generator
	AnchorSpringGenerator* asg1 = new AnchorSpringGenerator(e2->GetCenter(), coef, rl);

	// register the generator
	m_particleRegistry->Register(e1, asg1);

	return asp;
}

void Physics3State::SpawnStack(const Vector3& origin , uint sideLength, uint stackHeight)
{
	Vector3 stack_pos_origin = origin;		 // copy the origin, the origin is at bottom left corner of the stack

	for (uint k = 0; k < stackHeight; ++k)
	{
		for (uint i = 0; i < sideLength; ++i)
		{
			float stack_x = stack_pos_origin.x + i * 1.f; 

			for (uint j = 0; j < sideLength; ++j)
			{
				float stack_z = stack_pos_origin.z + j * 1.f;

				Vector3 stack_pos = Vector3(stack_x, stack_pos_origin.y, stack_z);

				CollisionBox* box = WrapAroundTestBox(m_wraparound_plane, false, false, true, stack_pos, Vector3::ZERO, Vector3::ONE, true, true);
				box->SetStack(true);
				m_my_stack.push_back(box);
			}
		}

		sideLength--;
		stack_pos_origin = Vector3(stack_pos_origin.x + .5f, stack_pos_origin.y + 1.f, stack_pos_origin.z + .5f);
	}
}

void Physics3State::SpawnRandomBox(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max)
{
	AABB3 bound(min, max);

	for (uint i = 0; i < num; ++i)
	{
		// get box with random scales
		const float& scale_x = GetRandomFloatInRange(1.f, 5.f);
		const float& scale_y = GetRandomFloatInRange(1.f, 5.f);
		const float& scale_z = GetRandomFloatInRange(1.f, 5.f);
		Vector3 rand_scale = Vector3(scale_x, scale_y, scale_z);

		const Vector3& rand_pos = GetRandomLocationWithin(bound);
		WrapAroundTestBox(wpa, true, false, true, rand_pos, Vector3::ZERO, rand_scale, true, true);
	}
}

void Physics3State::SpawnRandomSphere(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max)
{
	AABB3 bound(min, max);

	for (uint i = 0; i < num; ++i)
	{
		const float& scale = GetRandomFloatInRange(1.f, 5.f);
		Vector3 s = Vector3(scale, scale, scale);

		const Vector3& rand_pos = GetRandomLocationWithin(bound);
		WrapAroundTestSphere(wpa, true, false, true, rand_pos, Vector3::ZERO, s);
	}
}

void Physics3State::SpawnRandomConvex(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max)
{
	AABB3 bound(min, max);

	for (uint i = 0; i < num; ++i)
	{
		const Vector3& rand_pos = GetRandomLocationWithin(bound);
		WrapAroundTestConvex(wpa, true, false, true, rand_pos, Vector3::ZERO, Vector3::ONE, true, true);
	}
}

void Physics3State::ShootSphere(WrapAround* wpa)
{
	if (wpa == m_wraparound_plane)
	{
		// handle_0 is a sphere rb
		CollisionSphere* sph = WrapAroundTestSphere(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE);
		sph->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 50.f);
	}
}

void Physics3State::ShootBox(WrapAround* wpa)
{
	if (wpa == m_wraparound_plane)
	{
		CollisionBox* bx = WrapAroundTestBox(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE, true, true);
		bx->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 50.f);		// give it a speed boost
	}
}

void Physics3State::ResetCollisionCornerCase(const Vector3& pos1, const Vector3& pos2, const Vector3& rot1, const Vector3& rot2)
{
	if (!m_corner_case_1 && !m_corner_case_2)
	{
		m_corner_case_1 = new CollisionBox(.5f);
		CollisionRigidBody* rb = new CollisionRigidBody(1.f, pos1, rot1);
		rb->SetAwake(true);
		rb->SetSleepable(false);

		m_corner_case_1->AttachToRigidBody(rb);
		m_corner_case_1->m_cmp->SetPosAndOrient(pos1, Quaternion::FromEuler(rot1));
		m_wraparound_demo_0->m_phys_obj.push_back(m_corner_case_1->m_cmp);

		m_corner_case_2 = new CollisionBox(.5f);
		rb = new CollisionRigidBody(1.f, pos2, rot2);
		rb->SetAwake(true);
		rb->SetSleepable(false);

		m_corner_case_2->AttachToRigidBody(rb);
		m_corner_case_2->m_cmp->SetPosAndOrient(pos2, Quaternion::FromEuler(rot2));
		m_wraparound_demo_0->m_phys_obj.push_back(m_corner_case_2->m_cmp);
	}

	else if (m_corner_case_1 && m_corner_case_2)
	{
		m_corner_case_1->m_cmp->SetPosAndOrient(pos1, Quaternion::FromEuler(rot1));
		m_corner_case_2->m_cmp->SetPosAndOrient(pos2, Quaternion::FromEuler(rot2));

		m_corner_case_1->m_cmp->SetLinearVel(Vector3::ZERO);
		m_corner_case_2->m_cmp->SetLinearVel(Vector3::ZERO);

		m_corner_case_1->m_cmp->SetAngularVel(Vector3::ZERO);
		m_corner_case_2->m_cmp->SetAngularVel(Vector3::ZERO);
	}
}

std::pair<PhysXObject*, PhysXObject*> Physics3State::ResetCollisionCornerCasePhysX(const Vector3& pos1, const Vector3& pos2, const Vector3& rot1, const Vector3& rot2)
{
	std::pair<PhysXObject*, PhysXObject*> m_pair;

	// see if cc3 is there yet
	if (g_theApp->m_corner_case_3 != nullptr)
	{
		g_theApp->m_corner_case_3->SetPosAndOrient(pos1, Quaternion::FromEuler(rot1));
		g_theApp->m_corner_case_3->SetLinearVel(Vector3::ZERO);
		g_theApp->m_corner_case_3->SetAngularVel(Vector3::ZERO);
		m_pair.first = nullptr;
	}
	else
	{
		g_theApp->m_corner_case_3 = g_theApp->SpawnPhysxBox(pos1);
		g_theApp->m_corner_case_3->DisableGravity();
		m_pair.first = g_theApp->m_corner_case_3;
	}

	if (g_theApp->m_corner_case_4 != nullptr)
	{
		g_theApp->m_corner_case_4->SetPosAndOrient(pos2, Quaternion::FromEuler(rot2));
		g_theApp->m_corner_case_4->SetLinearVel(Vector3::ZERO);
		g_theApp->m_corner_case_4->SetAngularVel(Vector3::ZERO);
		m_pair.second = nullptr;
	}
	else
	{
		g_theApp->m_corner_case_4 = g_theApp->SpawnPhysxBox(pos2);
		g_theApp->m_corner_case_4->DisableGravity();
		m_pair.second = g_theApp->m_corner_case_4;
	}

	return m_pair;
}