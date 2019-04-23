﻿#include "Game/Physics3State.hpp"
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
	float txtHeight = height / 70.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - txtHeight);
	titleMin -= Vector2(0.f, txtHeight);
	titleMin -= Vector2(0.f, txtHeight);

	std::string tensor_ui = Stringf("Local Tensor:");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, tensor_ui, VERT_PCU);
	m_tensor_ui.push_back(t_mesh);
	titleMin -= Vector2(0.f, txtHeight);

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

	// my demo
	m_wraparound_demo_0 = new WrapAround(Vector3(20.f, 300.f, -10.f), Vector3(40.f, 320.f, 10.f));

	// phsx demo
	m_wraparound_demo_1 = new WrapAround(Vector3(60.f, 300.f, -10.f), Vector3(80.f, 320.f, 10.f));

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
}

void Physics3State::PostConstruct()
{
	m_wraparound_plane->m_physState = this;
	m_wraparound_demo_0->m_physState = this;
	m_wraparound_demo_1->m_physState = this;
}

void Physics3State::Update(float deltaTime)
{
	UpdateInput(deltaTime);				// update input
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_F))
	{
		DebugRenderTasksFlush();
	}
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
		SpawnRandomConvex(m_wraparound_plane, 10, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 360.f, 100.f));

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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_MINUS))
		g_theApp->SpawnPhysxStack(Vector3(100.f, 342.5f, 45.f), 5, 5);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_PLUS))
	{
		for (int i = 0; i < g_theApp->m_physx_stack.size(); ++i)
			g_theApp->m_physx_stack[i]->SetShouldDelete(true);
		g_theApp->m_physx_stack.clear();
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_I))
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
}

void Physics3State::RenderWrapArounds(Renderer* renderer)
{
	m_wraparound_plane->Render(renderer);
	m_wraparound_demo_0->Render(renderer);
	m_wraparound_demo_1->Render(renderer);
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
	const Vector3& scale, const std::string& fp, const std::string& tx)
{
	CollisionSphere* sph = new CollisionSphere(scale.x, fp, tx);

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

CollisionConvexObject* Physics3State::WrapAroundTestConvex(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3&, const Vector3&, const bool& awake /*= true*/, const bool& sleepable /*= false*/)
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

				WrapAroundTestBox(m_wraparound_plane, false, false, true, stack_pos, Vector3::ZERO, Vector3::ONE, false, true);
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
		WrapAroundTestConvex(wpa, true, false, true, rand_pos, Vector3::ZERO, Vector3::ONE);
	}
}

void Physics3State::ShootSphere(WrapAround* wpa)
{
	// handle_0 is a sphere rb
	CollisionSphere* sph = WrapAroundTestSphere(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE);
	sph->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 50.f);
}

void Physics3State::ShootBox(WrapAround* wpa)
{
	CollisionBox* bx = WrapAroundTestBox(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE, true, true);
	bx->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 50.f);		// give it a speed boost
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
