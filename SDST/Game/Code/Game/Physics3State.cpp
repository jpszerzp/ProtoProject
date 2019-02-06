#include "Game/Physics3State.hpp"
#include "Game/GameCommon.hpp"
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

// this is the physics state for thesis

static ConvexPolyhedron* s_convex_poly = nullptr;

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
	m_solver = CollisionSolver(MAX_CONTACT_NUM * 8, .01f, .01f);

	// a sphere
	m_wraparound_sphere = new WrapAround(Vector3(20.f, 300.f, -10.f), Vector3(40.f, 320.f, 10.f),
		Vector3(25.f, 305.f, -5.f), Vector3(35.f, 305.f, -5.f),
		Vector3(25.f, 305.f, 5.f), Vector3(35.f, 305.f, 5.f),
		Vector3(25.f, 315.f, -5.f), Vector3(35.f, 315.f, -5.f),
		Vector3(25.f, 315.f, 5.f), Vector3(35.f, 315.f, 5.f));

	m_handle_0 = new CollisionSphere(1.f);

	CollisionRigidBody* rb = new CollisionRigidBody(1.f, Vector3(30.f, 310.f, 0.f), Vector3(0.f));
	rb->SetAwake(true);
	rb->SetSleepable(false);

	m_handle_0->AttachToRigidBody(rb);

	m_spheres.push_back(m_handle_0);

	m_wraparound_sphere->m_primitives.push_back(m_handle_0);

	// a box
	m_wraparound_box = new WrapAround(Vector3(50.f, 300.f, -10.f), Vector3(70.f, 320.f, 10.f),
		Vector3(55.f, 305.f, -5.f), Vector3(65.f, 305.f, -5.f),
		Vector3(55.f, 305.f, 5.f), Vector3(65.f, 305.f, 5.f),
		Vector3(55.f, 315.f, -5.f), Vector3(65.f, 315.f, -5.f),
		Vector3(55.f, 315.f, 5.f), Vector3(65.f, 315.f, 5.f));

	CollisionBox* box_0 = new CollisionBox(Vector3(.5f));

	rb = new CollisionRigidBody(1.f, Vector3(60.f, 310.f, 0.f), Vector3::ZERO);
	rb->SetAwake(true);
	rb->SetSleepable(false);

	box_0->AttachToRigidBody(rb);

	m_boxes.push_back(box_0);

	m_wraparound_box->m_primitives.push_back(box_0);

	// a plane
	m_wraparound_plane = new WrapAround(Vector3(20.f, 340.f, -10.f), Vector3(130.f, 360.f, 100.f),
		Vector3(25.f, 345.f, -5.f), Vector3(35.f, 345.f, -5.f),
		Vector3(25.f, 345.f, 5.f), Vector3(35.f, 345.f, 5.f),
		Vector3(25.f, 355.f, -5.f), Vector3(35.f, 355.f, -5.f),
		Vector3(25.f, 355.f, 5.f), Vector3(35.f, 355.f, 5.f));

	CollisionPlane* plane_0 = new CollisionPlane(Vector2(110.f), Vector3(0.f, 1.f, 0.f), 342.f);

	rb = new CollisionRigidBody(1.f, Vector3(75.f, 342.f, 45.f), Vector3(90.f, 0.f, 0.f));
	rb->SetAwake(true);
	rb->SetSleepable(false);

	plane_0->AttachToRigidBody(rb);

	m_planes.push_back(plane_0);

	// do not include plane in wraparound
	//m_wraparound_sphere_plane->m_primitives.push_back(plane_0);

	// stack
	//SpawnStack(Vector3(75.f, 342.5f, 45.f), 5, 5);

	// convex hull
	Plane p1 = Plane(Vector3(0.f, -1.f, 0.f), 5.f);
	Plane p2 = Plane(Vector3(-1.f, 1.f, 1.f).GetNormalized(), 5.f);
	Plane p3 = Plane(Vector3(1.f, 1.f, 1.f).GetNormalized(), 5.f);
	Plane p4 = Plane(Vector3(0.f, 1.f, -1.f).GetNormalized(), 5.f);
	std::vector<Plane> hull_planes;
	hull_planes.push_back(p1);
	hull_planes.push_back(p2);
	hull_planes.push_back(p3);
	hull_planes.push_back(p4);
	ConvexHull* cHull = new ConvexHull(hull_planes);
	s_convex_poly = new ConvexPolyhedron(cHull);

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

Physics3State::~Physics3State()
{
	delete m_wraparound_sphere;
	m_wraparound_sphere = nullptr;

	delete m_wraparound_box;
	m_wraparound_box = nullptr;
}

void Physics3State::PostConstruct()
{
	m_wraparound_sphere->m_physState = this;
	m_wraparound_box->m_physState = this;
	m_wraparound_plane->m_physState = this;
}

void Physics3State::Update(float deltaTime)
{
	UpdateInput(deltaTime);				// update input
	UpdateGameobjects(deltaTime);		// update gameobjects
	UpdateContacts(deltaTime);
	UpdateDebug(deltaTime);			
	UpdateUI();
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_5))
		WrapAroundTestBox(m_wraparound_sphere, false, false, true, Vector3(31.2f, 315.f, 0.f), Vector3::ZERO, Vector3::ONE);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_6))
		WrapAroundTestSphere(m_wraparound_sphere, false, false, true, Vector3(31.8f, 315.f, 0.f), Vector3::ZERO, Vector3::ONE);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_7))
		WrapAroundTestBox(m_wraparound_box, false, false, true, Vector3(60.8f, 315.f, 0.f), Vector3::ZERO, Vector3::ONE);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_8))
		SpawnRandomSphere(m_wraparound_plane, 10, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 360.f, 100.f));
		
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_9))
		SpawnRandomBox(m_wraparound_plane, 10, Vector3(20.f, 345.f, -10.f), Vector3(130.f, 360.f, 100.f));

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE))
		ShootSphere(m_wraparound_plane);

	// slow
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_0))
	{
		for (CollisionPrimitive* primitive : m_wraparound_sphere->m_primitives)
			primitive->GetRigidBody()->SetSlow(.01f);

		for (CollisionPrimitive* primitive : m_wraparound_box->m_primitives)
			primitive->GetRigidBody()->SetSlow(.01f);

		for (CollisionPrimitive* primitive : m_wraparound_plane->m_primitives)
			primitive->GetRigidBody()->SetSlow(.01f);
	}
	else
	{
		for (CollisionPrimitive* primitive : m_wraparound_sphere->m_primitives)
			primitive->GetRigidBody()->SetSlow(1.f);

		for (CollisionPrimitive* primitive : m_wraparound_box->m_primitives)
			primitive->GetRigidBody()->SetSlow(1.f);

		for (CollisionPrimitive* primitive : m_wraparound_plane->m_primitives)
			primitive->GetRigidBody()->SetSlow(1.f);
	}

	//CollisionRigidBody* rb = m_handle_0->GetRigidBody();

	//if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
	//	rb->SetAngularVelocity(Vector3(5.f, 0.f, 0.f));
	//else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
	//	rb->SetAngularVelocity(Vector3(-5.f, 0.f, 0.f));
	//else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
	//	rb->SetAngularVelocity(Vector3(0.f, 5.f, 0.f));
	//else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
	//	rb->SetAngularVelocity(Vector3(0.f, -5.f, 0.f));
	//else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
	//	rb->SetAngularVelocity(Vector3(0.f, 0.f, 5.f));
	//else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
	//	rb->SetAngularVelocity(Vector3(0.f, 0.f, -5.f));
	//else
	//	rb->SetAngularVelocity(Vector3::ZERO);

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
	m_wraparound_sphere->Update();
	m_wraparound_box->Update();
	m_wraparound_plane->Update();
}


void Physics3State::UpdateUI()
{
	Renderer* theRenderer = Renderer::GetInstance();
	Window* window = Window::GetInstance();
	float window_height = window->GetWindowHeight();
	float window_width = window->GetWindowWidth();
	BitmapFont* font = theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = window_height / 50.f;
	Vector2 titleMin = Vector2(-window_width / 2.f, window_height / 2.f - txtHeight);

	if (m_time_ui != nullptr)
	{
		delete m_time_ui;
		m_time_ui= nullptr;
	}

	PhysTimeSystem& time = PhysTimeSystem::GetTimeSystem();
	std::string time_ui = Stringf("Time: %f", time.GetTimeSeconds());
	m_time_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, time_ui, VERT_PCU);

	titleMin -= Vector2(0.f, txtHeight);

	if (m_motion_ui != nullptr)
	{
		delete m_motion_ui;
		m_motion_ui = nullptr;
	}

	std::string motion_ui = Stringf("Motion of handle 0: %f", m_handle_0->GetRigidBody()->GetRealTimeMotion());
	m_motion_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, motion_ui, VERT_PCU);
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
	m_keep.m_global_friction = .9f;
	m_keep.m_global_restitution = .1f;
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

	s_convex_poly->Render(renderer);
	
	RenderForwardPath(renderer);

	RenderWrapArounds(renderer);
}

void Physics3State::RenderGameobjects(Renderer* renderer)
{
	for (std::vector<CollisionSphere*>::size_type idx = 0; idx < m_spheres.size(); ++idx)
		m_spheres[idx]->Render(renderer);

	for (std::vector<CollisionBox*>::size_type idx = 0; idx < m_boxes.size(); ++idx)
		m_boxes[idx]->Render(renderer);

	for (std::vector<CollisionPlane*>::size_type idx = 0; idx < m_planes.size(); ++idx)
		m_planes[idx]->Render(renderer);
}

void Physics3State::RenderWrapArounds(Renderer* renderer)
{
	m_wraparound_sphere->Render(renderer);
	m_wraparound_box->Render(renderer);
	m_wraparound_plane->Render(renderer);
}

void Physics3State::RenderForwardPath(Renderer*)
{
	m_forwardPath->RenderScene(m_sceneGraph);
}

void Physics3State::RenderUI(Renderer*)
{
	DrawTextCut(m_time_ui);
	DrawTextCut(m_motion_ui);
}


CollisionSphere* Physics3State::WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3&)
{
	CollisionSphere* sph = new CollisionSphere(1.f);

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

CollisionBox* Physics3State::WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3&)
{
	CollisionBox* box = new CollisionBox(Vector3(.5f));

	CollisionRigidBody* rb = new CollisionRigidBody(1.f, position, rot);
	rb->SetAwake(true);
	rb->SetSleepable(false);

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

void Physics3State::SpawnStack(const Vector3& origin , uint sideLength, uint stackHeight)
{
	Vector3 stack_pos_origin = origin;		 // copy the origin, the origin is at bottom left corner of the stack

	for (uint k = 0; k < stackHeight; ++k)
	{
		for (uint i = 0; i < sideLength; ++i)
		{
			float stack_x = stack_pos_origin.x + i * 1.f; 

			for (int j = 0; j < sideLength; ++j)
			{
				float stack_z = stack_pos_origin.z + j * 1.f;

				Vector3 stack_pos = Vector3(stack_x, stack_pos_origin.y, stack_z);

				WrapAroundTestBox(m_wraparound_plane, false, false, true, stack_pos, Vector3::ZERO, Vector3::ONE);
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
		const Vector3& rand_pos = GetRandomLocationWithin(bound);
		WrapAroundTestBox(wpa, true, false, true, rand_pos, Vector3::ZERO, Vector3::ONE);
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

void Physics3State::ShootSphere(WrapAround* wpa)
{
	// handle_0 is a sphere rb
	m_handle_0 = WrapAroundTestSphere(wpa, true, false, true, m_camera->GetWorldPosition(), Vector3::ZERO, Vector3::ONE);
	m_handle_0->GetRigidBody()->SetLinearVelocity(m_camera->GetWorldForward().GetNormalized() * 100.f);
}
