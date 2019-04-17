#include "Game/Physics3State.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/Console/DevConsole.hpp"  
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
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

#include "PsThread.h"
#include "CmPhysXCommon.h"

#include <algorithm>

#define PVD_HOST "127.0.0.1"

// this is the physics state for thesis
const Vector3 Physics3State::ORIGIN_CAMERA = Vector3(0.f, 0.f, -20.f);

bool IsGameobjectDead(GameObject* go) { return go->m_dead; }

Physics3State::Physics3State()
{
	PhysxStartup();

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
	// also set as part of engine commons for engine access
	m_keep.m_collision_head = m_storage;
	g_col_keep = &m_keep;

	// solver
	m_solver = CollisionSolver(3, .01f, .01f);

	// a plane
	m_wraparound_plane = new WrapAround(Vector3(20.f, 340.f, -10.f), Vector3(130.f, 400.f, 100.f));

	CollisionPlane* plane = new CollisionPlane(Vector2(110.f), Vector3(0.f, 1.f, 0.f), 342.f);
	CollisionRigidBody* rb = new CollisionRigidBody(1.f, Vector3(75.f, 342.f, 45.f), Vector3(90.f, 0.f, 0.f));
	rb->SetAwake(true);
	rb->SetSleepable(false);

	plane->AttachToRigidBody(rb);
	m_planes.push_back(plane);

	// do not include plane in wraparound

	// demo 0, my api
	m_wraparound_demo_0 = new WrapAround(Vector3(20.f, 300.f, -10.f), Vector3(40.f, 320.f, 10.f));

	// UI
	// local tensor is fixed
	// the format is motion - mass - tensor - velocity (to be modified)
	BitmapFont* font = theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = height / 50.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - txtHeight);
	titleMin -= Vector2(0.f, txtHeight);
	titleMin -= Vector2(0.f, txtHeight);

	std::string tensor_ui = Stringf("Local Tensor:");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::LIGHT_BLUE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
	m_tensor_ui.push_back(t_mesh);
	titleMin -= Vector2(0.f, txtHeight);

	Matrix33 tensor_mat;
	if (m_focus != nullptr)
		tensor_mat = m_focus->GetRigidBody()->GetTensor();
	tensor_ui = Stringf("%.3f, %.3f, %.3f", tensor_mat.GetI().x, tensor_mat.GetJ().x, tensor_mat.GetK().x);
	t_mesh = Mesh::CreateTextImmediate(Rgba::LIGHT_BLUE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
	m_tensor_ui.push_back(t_mesh);
	titleMin -= Vector2(0.f, txtHeight);

	tensor_ui = Stringf("%.3f, %.3f, %.3f", tensor_mat.GetI().y, tensor_mat.GetJ().y, tensor_mat.GetK().y);
	t_mesh = Mesh::CreateTextImmediate(Rgba::LIGHT_BLUE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
	m_tensor_ui.push_back(t_mesh);
	titleMin -= Vector2(0.f, txtHeight);

	tensor_ui = Stringf("%.3f, %.3f, %.3f", tensor_mat.GetI().z, tensor_mat.GetJ().z, tensor_mat.GetK().z);
	t_mesh = Mesh::CreateTextImmediate(Rgba::LIGHT_BLUE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
	m_tensor_ui.push_back(t_mesh);
	titleMin -= Vector2(0.f, txtHeight);

	InitPhysxScene(true);

	// demo 1, physx api
	m_wraparound_demo_1 = new WrapAround(Vector3(60.f, 300.f, -10.f), Vector3(80.f, 320.f, 10.f));

	// force registry 
	m_particleRegistry = new ParticleForceRegistry();
	m_rigidRegistry = new RigidForceRegistry();

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
	m_inspection.push_back(Vector3(-5.f, 220.f, -20.f));
	m_inspection.push_back(Vector3(200.f, 200.f, -20.f));

	// particle springs
	CollisionPoint* sp_point_0 = WrapAroundTestPoint(nullptr, false, false, false,
		Vector3(10.f, 225.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	CollisionPoint* sp_point_1 = WrapAroundTestPoint(nullptr, false, false, false,
		Vector3(10.f, 235.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	//CollisionPoint* asp_point_0 = WrapAroundTestPoint(nullptr, false, false, false,
	//	Vector3(15.f, 225.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	//CollisionPoint* asp_point_1 = WrapAroundTestPoint(nullptr, false, false, false,
	//	Vector3(15.f, 235.f, -5.f), Vector3::ZERO, Vector3(10.f), true, false);
	//asp_point_0->GetRigidBody()->SetFrozen(true);
	//asp_point_1->GetRigidBody()->SetFrozen(true);
	// setting up registrations in the registry
	// A. springs
	m_spring = SetupSpring(sp_point_0, sp_point_1, 2.f, 9.5f);		
	//// two points initialized to be 5 units away, constraint by a spring system with rest length of 3
	//// B. anchored spring
	//m_anchorSpring = SetupAnchorSpring(asp_point_0, asp_point_1, 2.f, 8.f);

	// continuity demo
	m_wraparound_ccd = new WrapAround(Vector3(190.f, 190.f, -10.f), Vector3(210.f, 210.f, 10.f));
	m_discrete_ball = WrapAroundTestSphere(m_wraparound_ccd, false, false, false, 
		Vector3(195.f, 195.f, 0.f), Vector3::ZERO, Vector3::ONE,
		"wireframe", "Data/Images/white.png");
	m_discrete_ball->GetRigidBody()->SetFrozen(true);

	// debug render
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

	delete m_spring;
	m_spring = nullptr;

	delete m_rigidRegistry;
	m_rigidRegistry = nullptr;
	
	delete m_particleRegistry;
	m_particleRegistry = nullptr;

	PhysxShutdown(true);
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
	Profiler* profiler = Profiler::GetInstance();

	// my API
	profiler->ProfileMyMarkFrame();
	MyPhysicsUpdate(deltaTime);
	profiler->ProfileMyMarkEndFrame();

	// physx API
	profiler->ProfilePhysXMarkFrame();
	PhysxUpdate(true, deltaTime);
	profiler->ProfilePhysXMarkEndFrame();
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
		leftRight = -50.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_D))
		leftRight = 50.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_W))
		forwardBack = 50.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_S))
		forwardBack = -50.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_Q))
		upDown = 50.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_E))
		upDown = -50.f;

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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_F))
		DebugRenderTasksFlush();

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_X) && DebugRenderOn())
	{
		eDebugDrawMode mode = DEBUG_RENDER_USE_DEPTH;
		float lifetime = 1000.f;
		float lineThickness = 5.f;

		DebugRenderBasis(lifetime, Vector3::ZERO, Vector3(1.f, 0.f, 0.f), 
			Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f), 
			lineThickness, mode);

		Vector3 gridBL = Vector3(-100.f, 0.f, -100.f);
		Vector3 gridBR = Vector3(100.f, 0.f,-100.f);
		Vector3 gridTL = Vector3(-100.f, 0.f, 100.f);
		Vector3 gridTR = Vector3(100.f, 0.f, 100.f);
		DebugRenderPlaneGrid(lifetime, gridBL, gridTL, gridTR, gridBR, 10.f, 10.f, 2.5f, mode);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_7))
		SpawnRandomConvex(m_wraparound_plane, 1, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 380.f, 100.f));

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_8))
		SpawnRandomSphere(m_wraparound_plane, 1, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 380.f, 100.f));
		
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_9))
		SpawnRandomBox(m_wraparound_plane, 1, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 380.f, 100.f));

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE))
		// cam better be in the correct wrapbox
		ShootBox(m_wraparound_plane);
	
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_CONTROL))
		// cam better be in the correct wrapbox
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_1))
	{
		m_ff_test = true; m_fp_test = false; m_pp_test = false; m_ee_test = false; m_pe_test = false; m_fe_test = false;
		ResetCollisionCornerCase(CORNER_CASE_POS_FF_1, CORNER_CASE_POS_FF_2, CORNER_CASE_ORIENT_FF_1, CORNER_CASE_ORIENT_FF_2);
	}

	// physx corner case comparison group 
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
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_2))
	{
		m_phys_corner_case = FCC_FE;

		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_FE_3, CORNER_CASE_POS_FE_4, CORNER_CASE_ORIENT_FE_3, CORNER_CASE_ORIENT_FE_4);

		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);

		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3))
	{
		m_phys_corner_case = FCC_PP;

		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_PP_3, CORNER_CASE_POS_PP_4, CORNER_CASE_ORIENT_PP_3, CORNER_CASE_ORIENT_PP_4);

		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);

		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_4))
	{
		m_phys_corner_case = FCC_EE;

		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_EE_3, CORNER_CASE_POS_EE_4, CORNER_CASE_ORIENT_EE_3, CORNER_CASE_ORIENT_EE_4);

		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);

		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_5))
	{
		m_phys_corner_case = FCC_PE;

		std::pair<PhysXObject*, PhysXObject*> obj_pair = ResetCollisionCornerCasePhysX(CORNER_CASE_POS_PE_3, CORNER_CASE_POS_PE_4, CORNER_CASE_ORIENT_PE_3, CORNER_CASE_ORIENT_PE_4);

		if (obj_pair.first != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.first);

		if (obj_pair.second != nullptr)
			m_wraparound_demo_1->m_phys_obj.push_back(obj_pair.second);
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_U))
	{
		// to fire the corner case test, give opposite velocities
		if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_ff_test)
		{
			m_corner_case_1->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_FF_1);
			m_corner_case_2->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_FF_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_fp_test)
		{
			m_corner_case_1->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_FP_1);
			m_corner_case_2->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_FP_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_pp_test)
		{
			m_corner_case_1->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_PP_1);
			m_corner_case_2->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_PP_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_ee_test)
		{
			m_corner_case_1->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_EE_1);
			m_corner_case_2->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_EE_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_pe_test)
		{
			m_corner_case_1->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_PE_1);
			m_corner_case_2->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_PE_2);
		}
		else if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr && m_fe_test)
		{
			m_corner_case_1->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_FE_1);
			m_corner_case_2->GetRigidBody()->SetLinearVelocity(CORNER_CASE_LIN_VEL_FE_2);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_I))
	{
		if (m_phys_corner_case == FCC_FF)
		{
			m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_FF_3);
			m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_FF_4);
		}
		else if (m_phys_corner_case == FCC_FP)
		{
			m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_FP_3);
			m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_FP_4);
		}
		else if (m_phys_corner_case == FCC_FE)
		{
			m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_FE_3);
			m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_FE_4);
		}
		else if (m_phys_corner_case == FCC_PP)
		{
			m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_PP_3);
			m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_PP_4);
		}
		else if (m_phys_corner_case == FCC_EE)
		{
			m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_EE_3);
			m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_EE_4);
		}
		else if (m_phys_corner_case == FCC_PE)
		{
			m_corner_case_3->SetLinearVel(CORNER_CASE_LIN_VEL_PE_3);
			m_corner_case_4->SetLinearVel(CORNER_CASE_LIN_VEL_PE_4);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_MINUS))
		SpawnPhysxStack(Vector3(100.f, 342.5f, 45.f), 5, 5);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_PLUS))
	{
		for (int i = 0; i < m_physx_stack.size(); ++i)
			m_physx_stack[i]->SetShouldDelete(true);

		m_physx_stack.clear();
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_2))
		m_debug_ui = !m_debug_ui;

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_4))
		SpawnStack(Vector3(75.f, 342.5f, 45.f), 5, 5);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_6))
	{
		for (int i = 0; i < m_my_stack.size(); ++i)
			m_my_stack[i]->SetShouldDelete(true);

		m_my_stack.clear();
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Y))
	{
		// delete all in wpa plane (or the most general wpa)
		for (int i = 0; i < m_wraparound_plane->m_primitives.size(); ++i)
			m_wraparound_plane->m_primitives[i]->SetShouldDelete(true);

		m_wraparound_plane->m_primitives.clear();
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		if (!m_inspection.empty())
		{
			Vector3 pos = m_inspection[m_insepction_count];

			// set camera pos to this inspection position
			m_camera->GetTransform().SetLocalPosition(pos);
			m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);

			int size = m_inspection.size();
			++m_insepction_count;
			m_insepction_count = (m_insepction_count % size);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_L))
	{

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

void Physics3State::UpdateForceRegistries(float dt)
{
	if (m_particleRegistry != nullptr)
		m_particleRegistry->UpdateForces(dt);
	//if (m_rigidRegistry != nullptr)
	//	m_rigidRegistry->UpdateForces(dt);
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
	if (m_debug_ui)
	{
		// motion
		Renderer* theRenderer = Renderer::GetInstance();
		Window* window = Window::GetInstance();
		float window_height = window->GetWindowHeight();
		float window_width = window->GetWindowWidth();
		BitmapFont* font = theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		float txtHeight = window_height / 50.f;
		Vector2 titleMin = Vector2(-window_width / 2.f, window_height / 2.f - txtHeight);

		delete m_motion_ui;
		m_motion_ui = nullptr;

		std::string motion_ui = Stringf("Motion of focused: %.3f", m_focus->GetRigidBody()->GetRealTimeMotion());
		m_motion_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, motion_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		// mass
		delete m_mass_ui;
		m_mass_ui = nullptr;

		std::string mass_ui = Stringf("Mass: %.3f", m_focus->GetRigidBody()->GetMass());
		m_mass_ui = Mesh::CreateTextImmediate(Rgba::RED, titleMin, font, txtHeight, .5f, mass_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		// considering tensor takes 4 lines
		for (int i = 0; i < 4; ++i)
			titleMin -= Vector2(0.f, txtHeight);

		// world inv tensor
		DeleteVector(m_world_inv_tensor_ui);

		std::string inv_tensor_ui = Stringf("World Inverse Tensor:");
		Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::MEGENTA, titleMin, font, txtHeight, .5f, inv_tensor_ui, VERT_PCU);
		m_world_inv_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);

		const Matrix33& inv_tensor_mat = m_focus->GetRigidBody()->GetIITWorldCopy();
		inv_tensor_ui = Stringf("%.3f, %.3f, %.3f", inv_tensor_mat.GetI().x, inv_tensor_mat.GetJ().x, inv_tensor_mat.GetK().x);
		t_mesh = Mesh::CreateTextImmediate(Rgba::MEGENTA, titleMin, font, txtHeight, .5f, inv_tensor_ui, VERT_PCU);
		m_world_inv_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);

		inv_tensor_ui = Stringf("%.3f, %.3f, %.3f", inv_tensor_mat.GetI().y, inv_tensor_mat.GetJ().y, inv_tensor_mat.GetK().y);
		t_mesh = Mesh::CreateTextImmediate(Rgba::MEGENTA, titleMin, font, txtHeight, .5f, inv_tensor_ui, VERT_PCU);
		m_world_inv_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);

		inv_tensor_ui = Stringf("%.3f, %.3f, %.3f", inv_tensor_mat.GetI().z, inv_tensor_mat.GetJ().z, inv_tensor_mat.GetK().z);
		t_mesh = Mesh::CreateTextImmediate(Rgba::MEGENTA, titleMin, font, txtHeight, .5f, inv_tensor_ui, VERT_PCU);
		m_world_inv_tensor_ui.push_back(t_mesh);
		titleMin -= Vector2(0.f, txtHeight);

		// velocity
		delete m_vel_ui;
		m_vel_ui = nullptr;

		const Vector3& lin_vel = m_focus->GetRigidBody()->GetLinearVelocity();
		const Vector3& ang_vel = m_focus->GetRigidBody()->GetAngularVelocity();
		std::string vel_ui = Stringf("Linear and angular velocity of the focused: (%.3f, %.3f, %.3f), (%.3f, %.3f, %.3f)", 
			lin_vel.x, lin_vel.y, lin_vel.z, ang_vel.x, ang_vel.y, ang_vel.z);
		m_vel_ui = Mesh::CreateTextImmediate(Rgba::GREEN, titleMin, font, txtHeight, .5f, vel_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		// first order
		delete m_pos_ui;
		m_pos_ui = nullptr;

		delete m_orient_ui;
		m_orient_ui = nullptr;

		const Vector3& center = m_focus->GetRigidBody()->GetCenter();
		const Quaternion& orient = m_focus->GetRigidBody()->GetOrientation();

		std::string pos_ui = Stringf("Center: (%.3f, %.3f, %.3f)", center.x, center.y, center.z);
		std::string orient_ui = Stringf("Orientation: (%.3f, %.3f, %.3f, %.3f)", 
			orient.m_imaginary.x, orient.m_imaginary.y, orient.m_imaginary.z, orient.m_real);

		m_pos_ui = Mesh::CreateTextImmediate(Rgba::CYAN, titleMin, font, txtHeight, .5f, pos_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		m_orient_ui = Mesh::CreateTextImmediate(Rgba::CYAN, titleMin, font, txtHeight, .5f, orient_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		// second order
		delete m_base_acc_ui;
		m_base_acc_ui = nullptr;
		const Vector3& base_acc = m_focus->GetRigidBody()->GetBaseLinearAcc();
		std::string base_acc_ui = Stringf("Base Linear Acc: %.3f, %.3f, %.3f",
			base_acc.x, base_acc.y, base_acc.z);
		m_base_acc_ui = Mesh::CreateTextImmediate(Rgba::PURPLE, titleMin, font, txtHeight, .5f, base_acc_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		delete m_net_acc_ui;
		m_net_acc_ui = nullptr;
		const Vector3& net_acc = m_focus->GetRigidBody()->GetLastFrameLinearAcc();
		std::string net_acc_ui = Stringf("Net Linear Acc: %.3f, %.3f, %.3f",
			net_acc.x, net_acc.y, net_acc.z);
		m_net_acc_ui = Mesh::CreateTextImmediate(Rgba::PURPLE, titleMin, font, txtHeight, .5f, net_acc_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		delete m_net_force_ui;
		m_net_force_ui = nullptr;
		const Vector3& force = m_focus->GetRigidBody()->GetNetForce();
		std::string force_ui = Stringf("Net Force: %.3f, %.3f, %.3f", force.x, force.y, force.z);
		m_net_force_ui = Mesh::CreateTextImmediate(Rgba::PURPLE, titleMin, font, txtHeight, .5f, force_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		delete m_net_torque_ui;
		m_net_torque_ui = nullptr;
		const Vector3& torque = m_focus->GetRigidBody()->GetNetTorque();
		std::string torque_ui = Stringf("Net Torque: %.3f, %.3f, %.3f", torque.x, torque.y, torque.z);
		m_net_torque_ui = Mesh::CreateTextImmediate(Rgba::PURPLE, titleMin, font, txtHeight, .5f, torque_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		delete m_lin_damp_ui;
		m_lin_damp_ui = nullptr;
		const float& lin_damp = m_focus->GetRigidBody()->GetLinDamp();
		std::string lin_damp_ui = Stringf("Linear Damp: %.3f", lin_damp);
		m_lin_damp_ui = Mesh::CreateTextImmediate(Rgba::PINK, titleMin, font, txtHeight, .5f, lin_damp_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		delete m_ang_damp_ui;
		m_ang_damp_ui = nullptr;
		const float& ang_damp = m_focus->GetRigidBody()->GetAngDamp();
		std::string ang_damp_ui = Stringf("Angular Damp: %.3f", ang_damp);
		m_ang_damp_ui = Mesh::CreateTextImmediate(Rgba::PINK, titleMin, font, txtHeight, .5f, ang_damp_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);

		delete m_slow_ui;
		m_slow_ui = nullptr;
		const float& slow_factor = m_focus->GetRigidBody()->GetSlowFactor();
		std::string slow_factor_ui = Stringf("Slow Factor: %.3f", slow_factor);
		m_slow_ui = Mesh::CreateTextImmediate(Rgba::PINK, titleMin, font, txtHeight, .5f, slow_factor_ui, VERT_PCU);
		titleMin -= Vector2(0.f, txtHeight);
	}
}

void Physics3State::UpdateDeletePhysx()
{
	// stack vector has already been emptied, objs are already marked as deleted

	// delete those obj prescence in general vector
	// remove those actors in physx scene
	for (int i = 0; i < m_physx_objs.size(); ++i)
	{
		PhysXObject* phys_obj = m_physx_objs[i];
		if (phys_obj->ShouldDelete())
		{
			// obj vector
			std::vector<PhysXObject*>::iterator it = m_physx_objs.begin() + i;
			m_physx_objs.erase(it);

			// we do not know which vector of physx obj is affected, so we check all
			// still check those in stacks just to be safe
			for (int j = 0; j < m_physx_stack.size(); ++j)
			{
				if (m_physx_stack[j] == phys_obj)
				{
					std::vector<PhysXObject*>::iterator it_del = m_physx_stack.begin() + j;

					m_physx_stack.erase(it_del);
					j--;
				}
			}

			// check those in wraparounds
			m_wraparound_demo_0->RemovePhysXObj(phys_obj);
			m_wraparound_demo_1->RemovePhysXObj(phys_obj);

			// todo: check those in plane wraparound after they are actually added to it...
			//m_wraparound_plane...

			// check corner case place holder
			if (m_corner_case_3 == phys_obj)
				m_corner_case_3 = nullptr;
			else if (m_corner_case_4 == phys_obj)
				m_corner_case_4 = nullptr;

			// scene actor
			PxRigidActor* ra = phys_obj->GetRigidActor();
			m_physx_scene->removeActor(*ra);
			ra->release();
			
			delete phys_obj;
			i = i - 1;
		}
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

void Physics3State::MyPhysicsUpdate(float deltaTime)
{
	//PROFILE_LOG_SCOPED("My physics path");
	ProfileLogScoped my_api("Physics3State::MyPhysicsUpdate", true);
	UpdateInput(deltaTime);				// update input
	UpdateForceRegistries(deltaTime);	// update force
	UpdateGameobjects(deltaTime);		// update gameobjects
	UpdateContacts(deltaTime);
	UpdateDebug(deltaTime);			
	UpdateUI();
	UpdateDelete();
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
			if (!m_keep.AllowMoreCollision())
				return;

			CollisionPlane* pl = m_planes[idx1];

			CollisionSensor::SphereVsPlane(*sph0, *pl, &m_keep);
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

		// convex vs box and shpere need debug...
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

	PhysxRender(renderer);
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
	if (m_debug_ui)
	{
		DrawTextCut(m_motion_ui);
		DrawTextCut(m_mass_ui);

		for (int i = 0; i < m_tensor_ui.size(); ++i)
			DrawTextCut(m_tensor_ui[i]);

		for (int i = 0; i < m_world_inv_tensor_ui.size(); ++i)
			DrawTextCut(m_world_inv_tensor_ui[i]);

		DrawTextCut(m_vel_ui);

		DrawTextCut(m_pos_ui);
		DrawTextCut(m_orient_ui);

		DrawTextCut(m_base_acc_ui);
		DrawTextCut(m_net_acc_ui);
		DrawTextCut(m_net_force_ui);
		DrawTextCut(m_net_torque_ui);

		DrawTextCut(m_lin_damp_ui);
		DrawTextCut(m_ang_damp_ui);
		DrawTextCut(m_slow_ui);
	}
}

CollisionSphere* Physics3State::WrapAroundTestSphere(WrapAround* wpa, 
	bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, 
	const Vector3& rot, const Vector3&,
	const std::string& fp, const std::string& tx)
{
	CollisionSphere* sph = new CollisionSphere(1.f, fp, tx);

	CollisionRigidBody* rb = new CollisionRigidBody(1.f, position, rot);
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

	wpa->m_primitives.push_back(sph);

	return sph;
}

CollisionPoint* Physics3State::WrapAroundTestPoint(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, 
	bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale, const bool& awake, const bool& sleepable)
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

CollisionBox* Physics3State::WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, 
	bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale, const bool& awake, const bool& sleepable)
{
	CollisionBox* box = new CollisionBox(scale / 2.f);

	CollisionRigidBody* rb = new CollisionRigidBody(1.f, position, rot);
	rb->SetAwake(awake);
	rb->SetSleepable(sleepable);

	box->AttachToRigidBody(rb);

	// add to general box vector
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

	// add to primitive vector in wpa
	wpa->m_primitives.push_back(box);

	return box;
}

CollisionConvexObject* Physics3State::WrapAroundTestConvex(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale, const bool& awake /*= true*/, const bool& sleepable /*= false*/)
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
		WrapAroundTestBox(wpa, true, false, true, rand_pos, Vector3::ZERO, rand_scale);
	}
}

void Physics3State::SpawnRandomSphere(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max)
{
	AABB3 bound(min, max);

	for (uint i = 0; i < num; ++i)
	{
		const Vector3& rand_pos = GetRandomLocationWithin(bound);
		WrapAroundTestSphere(wpa, true, false, true, rand_pos, Vector3::ZERO, Vector3::ONE);
	}
}

void Physics3State::SpawnRandomConvex(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max)
{
	AABB3 bound(min, max);

	for (uint i = 0; i < num; ++i)
	{
		const Vector3& rand_pos = GetRandomLocationWithin(bound);
		WrapAroundTestConvex(wpa, true, false, true, rand_pos, Vector3::ZERO, Vector3::ONE);
	}
}

void Physics3State::ShootSphere(WrapAround* wpa)
{
	// handle_0 is a sphere rb
	CollisionSphere* sph = WrapAroundTestSphere(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE);
	sph->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 100.f);
}

void Physics3State::ShootBox(WrapAround* wpa)
{
	CollisionBox* bx = WrapAroundTestBox(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE);
	bx->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 100.f);		// give it a speed boost
}

////////////////////////////////////////////////////////////////////////// PhysX config //////////////////////////////////////////////////////////////////////////

PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	// all initial and persisting reports for everything, with per-point data
	pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT
		|	PxPairFlag::eNOTIFY_TOUCH_FOUND 
		| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
		| PxPairFlag::eNOTIFY_CONTACT_POINTS;
	return PxFilterFlag::eDEFAULT;
}

std::vector<PxVec3> gContactPositions;
std::vector<PxVec3> gContactImpulses;

class ContactReportCallback: public PxSimulationEventCallback
{
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)	{ PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count)					{ PX_UNUSED(pairs); PX_UNUSED(count); }
	void onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) 
	{
		PX_UNUSED((pairHeader));
		std::vector<PxContactPairPoint> contactPoints;

		for(PxU32 i=0;i<nbPairs;i++)
		{
			PxU32 contactCount = pairs[i].contactCount;
			if(contactCount)
			{
				contactPoints.resize(contactCount);
				pairs[i].extractContacts(&contactPoints[0], contactCount);

				for(PxU32 j=0;j<contactCount;j++)
				{
					gContactPositions.push_back(contactPoints[j].position);
					gContactImpulses.push_back(contactPoints[j].impulse);
				}
			}
		}
	}
};

ContactReportCallback gContactReportCallback;

void Physics3State::InitPhysxScene(bool interactive)
{
	PX_UNUSED(interactive);

	// dispatcher
	PxU32 num_cores = Ps::Thread::getNbPhysicalCores();
	m_physx_dispatcher = PxDefaultCpuDispatcherCreate(num_cores == 0 ? 0 : num_cores - 1);

	// scene desc
	Vector3 half_gravity = (Vector3::GRAVITY / 2.f);
	PxSceneDesc scene_desc(m_physics->getTolerancesScale());
	scene_desc.cpuDispatcher = m_physx_dispatcher;
	scene_desc.gravity = PxVec3(half_gravity.x, half_gravity.y, half_gravity.z);
	scene_desc.filterShader = contactReportFilterShader;
	scene_desc.simulationEventCallback = &gContactReportCallback;
	m_physx_scene = m_physics->createScene(scene_desc);

	// with scene, config pvd client
	PxPvdSceneClient* pvd_client = m_physx_scene->getScenePvdClient();
	if (pvd_client)
		pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);

	// material
	m_physx_mat = m_physics->createMaterial(.5f, .5f, .6f);
}

void Physics3State::SpawnPhysxStack(const Vector3& origin, uint sideLength, uint stackHeight)
{
	// plane
	PxRigidStatic* pl = PxCreatePlane(*m_physics, PxPlane(0, 1, 0, -342), *m_physx_mat);
	m_physx_scene->addActor(*pl);

	// interface with my API
	PhysXObject* pl_obj = new PhysXObject(pl);
	m_physx_objs.push_back(pl_obj);
	m_physx_stack.push_back(pl_obj);

	PxVec3 stack_offset = PxVec3(0.f);
	PxVec3 stack_origin = PxVec3(origin.x, origin.y, origin.z);
	PxTransform pxt = PxTransform(stack_origin);
	PxReal half_ext = .5f;
	PxShape* shape = m_physics->createShape(PxBoxGeometry(half_ext, half_ext, half_ext), *m_physx_mat);
	for (PxU32 k = 0; k < stackHeight; ++k)
	{
		for (PxU32 i = 0; i < sideLength; ++i)
		{
			// * 2.f is why we use half_ext when setting up local transform
			PxReal stack_x = stack_offset.x + i * 2.f;			

			for (uint j = 0; j < sideLength; ++j)
			{
				PxReal stack_z = stack_offset.z + j * 2.f;

				PxVec3 offset = PxVec3(stack_x, stack_offset.y, stack_z);

				PxTransform local_t(offset * half_ext);
				PxRigidDynamic* body = m_physics->createRigidDynamic(pxt.transform(local_t));
				body->attachShape(*shape);
				PxRigidBodyExt::updateMassAndInertia(*body, 10.f);
				m_physx_scene->addActor(*body);

				PhysXObject* phys_obj = new PhysXObject(body);
				m_physx_objs.push_back(phys_obj);
				m_physx_stack.push_back(phys_obj);
			}
		}

		sideLength--;
		stack_origin = PxVec3(stack_origin.x + .5f, stack_origin.y + 1.f, stack_origin.z + .5f);
		pxt = PxTransform(stack_origin);
	}
	shape->release();
}

PhysXObject* Physics3State::SpawnPhysxBox(const Vector3& pos)
{
	PxVec3 pxp = PxVec3(pos.x, pos.y, pos.z);
	PxTransform pxt = PxTransform(pxp);
	PxReal half_ext = .5f;
	PxShape* shape = m_physics->createShape(PxBoxGeometry(half_ext, half_ext, half_ext), *m_physx_mat);

	// offset
	PxVec3 offset = PxVec3(0.f, 0.f, 0.f);
	PxTransform local = PxTransform(offset);
	
	// dynamic rigidbody
	PxRigidDynamic* body = m_physics->createRigidDynamic(pxt.transform(local));
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 1.f);
	m_physx_scene->addActor(*body);
	
	// encapsulation
	PhysXObject* px_obj = new PhysXObject(body);
	m_physx_objs.push_back(px_obj);

	// shape release
	shape->release();

	return px_obj;
}

void Physics3State::PhysxRender(Renderer* renderer)
{
	for (int i = 0; i < m_physx_objs.size(); ++i)
		m_physx_objs[i]->RenderActor(renderer);
}

void Physics3State::ResetCollisionCornerCase(const Vector3& pos1, const Vector3& pos2, const Vector3& rot1, const Vector3& rot2)
{
	if (m_corner_case_1 != nullptr && m_corner_case_2 != nullptr)
	{
		// to restore FF corner case, restore position, reset velocity, set orient, make sure gravity is null
		m_corner_case_1->GetRigidBody()->SetCenter(pos1);
		m_corner_case_2->GetRigidBody()->SetCenter(pos2);

		m_corner_case_1->GetRigidBody()->SetOrientation(Quaternion::FromEuler(rot1));
		m_corner_case_2->GetRigidBody()->SetOrientation(Quaternion::FromEuler(rot2));

		m_corner_case_1->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);
		m_corner_case_2->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);

		m_corner_case_1->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);
		m_corner_case_2->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);

		m_corner_case_1->GetRigidBody()->SetBaseLinearAcceleration(Vector3::ZERO);
		m_corner_case_2->GetRigidBody()->SetBaseLinearAcceleration(Vector3::ZERO);
	}
}

std::pair<PhysXObject*, PhysXObject*> Physics3State::ResetCollisionCornerCasePhysX(const Vector3& pos1, const Vector3& pos2, const Vector3& rot1, const Vector3& rot2)
{
	std::pair<PhysXObject*, PhysXObject*> m_pair;

	// see if cc3 is there yet
	if (m_corner_case_3 != nullptr)
	{
		m_corner_case_3->SetPosAndOrient(pos1, Quaternion::FromEuler(rot1));
		m_corner_case_3->SetLinearVel(Vector3::ZERO);
		m_corner_case_3->SetAngularVel(Vector3::ZERO);

		m_pair.first = nullptr;
	}
	else
	{
		m_corner_case_3 = SpawnPhysxBox(pos1);
		m_corner_case_3->DisableGravity();

		m_pair.first = m_corner_case_3;
	}

	if (m_corner_case_4 != nullptr)
	{
		m_corner_case_4->SetPosAndOrient(pos2, Quaternion::FromEuler(rot2));
		m_corner_case_4->SetLinearVel(Vector3::ZERO);
		m_corner_case_4->SetAngularVel(Vector3::ZERO);

		m_pair.second = nullptr;
	}
	else
	{
		m_corner_case_4 = SpawnPhysxBox(pos2);
		m_corner_case_4->DisableGravity();

		m_pair.second = m_corner_case_4;
	}

	return m_pair;
}

void Physics3State::PhysxUpdate(bool interactive, float deltaTime)
{
	//PROFILE_LOG_SCOPED("PhysX 4.0 path");
	ProfileLogScoped physx_api("Physics3State::PhysxUpdate", false);
	PX_UNUSED(interactive);
	gContactPositions.clear();
	gContactImpulses.clear();

	// in sample code, this is forced to be 60 fps...
	//deltaTime = 1.f / 60.f;
	m_physx_scene->simulate(deltaTime);
	m_physx_scene->fetchResults(true);
	DebuggerPrintf("%d contact reports\n", PxU32(gContactPositions.size()));

	UpdateDeletePhysx();
}

void Physics3State::PhysxStartup()
{
	static PhysErrorCallback gErrorCB;
	static PhysAllocator gAllocator;

	// create physx foundation
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCB);
	if (!m_foundation)
		ASSERT_OR_DIE(false, "PxCreateFoundation failed!");

	// create top-level physx object
	bool recordMemoryAllocations = true;

	// optional pvd instance, need a HOST
	m_pvd = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), recordMemoryAllocations, m_pvd);
	if (!m_physics)
		ASSERT_OR_DIE(false, "PxCreatePhysics failed!");

	TODO("optional startups: cooking, extensions, articulations, height fields");

	// extension
	if (!PxInitExtensions(*m_physics, m_pvd))
		ASSERT_OR_DIE(false, "PxInitExtensions failed!");
}

void Physics3State::PhysxShutdown(bool interactive)
{
	PX_UNUSED(interactive);
	m_physx_scene->release();
	m_physx_dispatcher->release();
	PxCloseExtensions();

	// release PxPhysics object
	m_physics->release();

	// release pvd
	PxPvdTransport* transport = m_pvd->getTransport();
	m_pvd->release();
	transport->release();

	// release foundation
	m_foundation->release();

	DebuggerPrintf("Physx unregistered.\n");
}