#include "Game/Physics3State.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/Console/DevConsole.hpp"  
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"
#include <algorithm>

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

	m_cameraInitialPos = Vector3(0.f, 0.f, -7.f);
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

	// force registry 
	m_particleRegistry = new ParticleForceRegistry();
	m_rigidRegistry = new RigidForceRegistry();

	// or, initialize into other shapes (cube, etc)
	m_g0 = InitializePhysSphere(Vector3(-5.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_KINEMATIC, BODY_PARTICLE);
	m_g1 = InitializePhysCube(Vector3(0.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_KINEMATIC, BODY_PARTICLE);
	InitializePhysCube(Vector3(0.f, 0.f, 2.f), Vector3::ZERO, Vector3::ONE, Rgba::BLUE, MOVE_KINEMATIC, BODY_PARTICLE);
	m_g2 = InitializePhysQuad(Vector3(0.f, -2.f, 0.f), Vector3(90.f, 0.f, 0.f), Vector3(200.f, 200.f, 1.f), Rgba::GREEN, MOVE_STATIC, BODY_PARTICLE);
	m_g3 = InitializePhysPoint(Vector3(5.f, 0.f, 0.f), Vector3::ZERO, 10.f, Rgba::WHITE, MOVE_STATIC, BODY_PARTICLE);
	
	// more customized game objects
	m_ballistics = SetupBallistics(ARTILLERY, Vector3(6.f, 0.f, 0.f), true, Rgba::GREEN);
	SetupBallistics(PISTOL, Vector3(7.f, 0.f, 0.f), true, Rgba::WHITE);
	SetupBallistics(LASER, Vector3(8.f, 0.f, 0.f), true, Rgba::WHITE);
	SetupBallistics(FIREBALL, Vector3(9.f, 0.f, 0.f), true, Rgba::WHITE);
	SetupBallistics(FREEFALL, Vector3(6.f, 0.f, -.5f), true, Rgba::CYAN);

	// generate one particle for firework
	SetupFireworks(5.f, Vector3(0.f, 0.f, 5.f), Vector3::ZERO, Vector3(0.f, 4.f, 0.f), Vector3(0.f, 4.f, 0.f), false);

	// points for springs
	Point* sp_point_0 = InitializePhysPoint(Vector3(0.f, 10.f, 5.f), Vector3::ZERO, 10.f, Rgba::RED, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* sp_point_1 = InitializePhysPoint(Vector3(0.f, 5.f, 5.f), Vector3::ZERO, 10.f, Rgba::CYAN, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* asp_point_0 = InitializePhysPoint(Vector3(-5.f, 10.f, 5.f), Vector3::ZERO, 10.f, Rgba::RED, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* asp_point_1 = InitializePhysPoint(Vector3(-5.f, 5.f, 5.f), Vector3::ZERO, 10.f, Rgba::CYAN, MOVE_STATIC, BODY_PARTICLE);

	// setting up registrations in the registry
	// A. springs
	m_spring = SetupSpring(sp_point_0, sp_point_1, 3.f, 3.f);		
	// two points initialized to be 5 units away, constraint by a spring system with rest length of 3
	// B. anchored spring
	m_anchorSpring = SetupAnchorSpring(asp_point_0, asp_point_1, 3.f, 3.f);

	// rod
	Point* rod_point_0 = InitializePhysPoint(Vector3(-10.f, 5.f, 5.f), Vector3::ZERO, 10.f, Rgba::GREEN, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* rod_point_1 = InitializePhysPoint(Vector3(-15.f, 8.f, 5.f), Vector3::ZERO, 10.f, Rgba::GREEN, MOVE_DYNAMIC, BODY_PARTICLE);
	float dist = (rod_point_0->GetWorldPosition() - rod_point_1->GetWorldPosition()).GetLength();
	m_rod = SetupRod(dist, rod_point_0, rod_point_1);

	// verlet
	Ballistics* verlet_basic_ballistics = SetupBallistics(FREEFALL, Vector3(7.f, 0.f, -.5f), true, Rgba::MEGENTA);
	verlet_basic_ballistics->m_physEntity->SetVerlet(true);
	verlet_basic_ballistics->m_physEntity->SetVerletScheme(BASIC_VERLET);
	verlet_basic_ballistics->m_physEntity->SetEntityLastCenter(verlet_basic_ballistics->m_physEntity->GetEntityCenter());

	Ballistics* verlet_vel_ballistics = SetupBallistics(FREEFALL, Vector3(8.f, 0.f, -.5f), true, Rgba::PINK);
	verlet_vel_ballistics->m_physEntity->SetVerlet(true);
	verlet_vel_ballistics->m_physEntity->SetVerletScheme(VELOCITY_VERLET);

	// rigid bodies and force registry
	m_r0 = InitializePhysSphere(Vector3(5.f, 0.f, 5.f), Vector3::ZERO, Vector3::ONE, Rgba::GREEN, MOVE_DYNAMIC, BODY_RIGID);
	m_r0->m_physEntity->SetFrozen(true);
	GravityRigidForceGenerator* grg = new GravityRigidForceGenerator(Vector3::GRAVITY);
	Rigidbody3* body = dynamic_cast<Rigidbody3*>(m_r0->m_physEntity);
	m_rigidRegistry->Register(body, grg);

	//////////////////////////////// For rigid spring ////////////////////////////////
	// anchor
	Point* theAnchor = InitializePhysPoint(
		Vector3(10.f, 13.f, 5.f),
		Vector3::ZERO, 10.f, Rgba::MEGENTA,
		MOVE_STATIC, BODY_PARTICLE);

	// the rigid ball
	m_r1 = InitializePhysSphere(
		Vector3(10.f, 0.f, 5.f),
		Vector3::ZERO, Vector3::ONE, Rgba::MEGENTA,
		MOVE_DYNAMIC, BODY_RIGID);
	m_r1->m_physEntity->SetFrozen(true);

	// entities
	Entity3* anchor_ent = theAnchor->GetEntity();
	Rigidbody3* other_rigid = static_cast<Rigidbody3*>(m_r1->GetEntity());

	// set up anchor spring
	m_rigidAnchorSpring = new GeneralRigidAnchorSpring(anchor_ent, other_rigid, 3.f, 5.f);

	// force generator
	Vector3 attachLocal = Vector3::ZERO;
	Sphere* theSphere = static_cast<Sphere*>(m_r1);
	ProjectPlaneToSphere(Vector2(.5f, .5f), theSphere->GetRadius(), attachLocal);
	AnchorSpringRigidForceGenerator* asrfg = new AnchorSpringRigidForceGenerator(
		theAnchor->GetWorldPosition(), other_rigid, attachLocal, 3.f, 5.f);
	
	// force registration
	m_rigidRegistry->Register(other_rigid, asrfg);
	m_rigidRegistry->Register(other_rigid, grg);
	//////////////////////////////////////////////////////////////////////////////////

	//m_collisionData = new CollisionData3(20);	// allowing specified number of contact at max
	m_iterResolver = new ContactResolver(2, RESOLVE_ITERATIVE);
	m_allResolver = new ContactResolver();

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

Physics3State::~Physics3State()
{
	//delete m_collisionData;
	//m_collisionData = nullptr;

	m_spheres.clear();
	m_cubes.clear();
	m_quads.clear();
	m_points.clear();

	delete m_particleRegistry;
	m_particleRegistry = nullptr;

	delete m_spring;
	m_spring = nullptr;

	delete m_anchorSpring;
	m_anchorSpring = nullptr;
}


Sphere* Physics3State::InitializePhysSphere(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	Sphere* s = new Sphere(pos, rot, scale, tint, "sphere_pcu", "default", moveStat, bid);
	s->m_physDriven = true;
	m_gameObjects.push_back(s);
	if (bid != BODY_RIGID)
		m_spheres.push_back(s);
	s->m_physEntity->SetGameobject(s);

	return s;
}

Cube* Physics3State::InitializePhysCube(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	Cube* c = new Cube(pos, rot, scale, tint, "cube_pcu", "default", moveStat, bid);
	c->m_physDriven = true;
	m_gameObjects.push_back(c);
	m_cubes.push_back(c);
	c->m_physEntity->SetGameobject(c);

	return c;
}

Quad* Physics3State::InitializePhysQuad(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	Quad* q = new Quad(pos, rot, scale, tint, "quad_pcu", "default", moveStat, bid, false, COMPARE_LESS, CULLMODE_FRONT);
	q->m_physDriven = true;
	m_gameObjects.push_back(q);
	m_quads.push_back(q);
	q->m_physEntity->SetGameobject(q);

	return q;
}

/*
Sphere* Physics3State::InitializeRigidSphere(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint)
{
	Sphere* s = new Sphere(pos, rot, scale, tint, "sphere_pcu", "default", true);
	s->m_physDriven = true;
	m_gameObjects.push_back(s);
	m_spheres.push_back(s);
	s->m_physEntity->SetGameobject(s);

	return s;
}
*/

Point* Physics3State::InitializePhysPoint(Vector3 pos, Vector3 rot, float size,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	Point* pt = new Point(pos, rot, size, tint, "point_pcu", "default", moveStat, bid);
	pt->m_physDriven = true;
	m_gameObjects.push_back(pt);
	m_points.push_back(pt);
	pt->m_physEntity->SetGameobject(pt);

	return pt;
}

Fireworks* Physics3State::SetupFireworks(float age, Vector3 pos, Vector3 inheritVel, Vector3 maxVel, Vector3 minVel, bool lastRound)
{
	Fireworks* fw = new Fireworks(pos);
	fw->Configure(age, inheritVel, maxVel, minVel, lastRound);
	m_gameObjects.push_back(fw);
	m_points.push_back(fw);
	fw->m_physEntity->SetGameobject(fw);
	return fw;
}

Ballistics* Physics3State::SetupBallistics(eBallisticsType type, Vector3 pos, bool frozen, Rgba color)
{
	Ballistics* b = new Ballistics(type, pos, frozen, color);
	b->m_physDriven = true;
	m_gameObjects.push_back(b);
	m_points.push_back(b);
	b->m_physEntity->SetGameobject(b);
	b->m_physEntity->SetNetForcePersistent(true);

	return b;
}

Spring* Physics3State::SetupSpring(Point* end1, Point* end2, float coef, float rl)
{
	Spring* sp = new Spring(end1, end2, coef, rl);

	// get the entities of points
	Entity3* e1 = end1->m_physEntity;
	Entity3* e2 = end2->m_physEntity;

	// turn off damping for springs
	e1->SetConsiderDamp(false);
	e2->SetConsiderDamp(false);

	// initialize spring force generators
	SpringGenerator* sg1 = new SpringGenerator(e2, coef, rl);
	SpringGenerator* sg2 = new SpringGenerator(e1, coef, rl);

	// register the generator with corresponding entity
	m_particleRegistry->Register(e1, sg1);
	m_particleRegistry->Register(e2, sg2);

	return sp;
}

AnchorSpring* Physics3State::SetupAnchorSpring(Point* end1, Point* end2, float coef, float rl)
{
	AnchorSpring* asp = new AnchorSpring(end1, end2, coef, rl);
	
	// get entities 
	Entity3* e1 = end1->m_physEntity;
	Entity3* e2 = end2->m_physEntity;

	// turn off damping for springs
	e1->SetConsiderDamp(false);
	e2->SetConsiderDamp(false);

	// anchored spring generator
	AnchorSpringGenerator* asg1 = new AnchorSpringGenerator(e2->GetEntityCenter(), coef, rl);

	// register the generator
	m_particleRegistry->Register(e1, asg1);

	return asp;
}

Rod* Physics3State::SetupRod(float length, Point* p1, Point* p2)
{
	Rod* rod = new Rod(length, p1, p2);
	return rod;
}

/*
void Physics3State::ParticlePhysicsUpdate(float deltaTime)
{
	// force registry update
	if (m_registry != nullptr)
		m_registry->UpdateForces(deltaTime);

	// GO update
	UpdateGameobjects(deltaTime);

	// contact generation
	SingleContactUpdate();
	m_rod->FillContact(m_collisionData->m_impulsive_contacts, 1);

	// contact resolution
	TODO("bring this to contact resolver class");
	for (int i = 0; i < m_collisionData->m_contacts.size(); ++i)
	{
		// iterate contacts
		Contact3& contact = m_collisionData->m_contacts[i];
		contact.ResolveContactNaive();
	}
	if (!m_collisionData->m_impulsive_contacts.empty())
		m_contactResolver->ResolveContacts(m_collisionData->m_impulsive_contacts, 
		(uint)m_collisionData->m_impulsive_contacts.size(), deltaTime);
}

void Physics3State::RigidbodyPhysicsUpdate(float deltaTime)
{
	if (m_rigidRegistry != nullptr)
		m_rigidRegistry->UpdateForces(deltaTime);

	// start of frame
	for (std::vector<Rigidbody3*>::size_type idx = 0; idx < m_rigidBodyObjects.size(); ++idx)
	{
		Entity3* rb_ent3 = m_rigidBodyObjects[idx]->m_physEntity;
		Rigidbody3* rb3 = dynamic_cast<Rigidbody3*>(rb_ent3);

		rb3->ClearAccs();
		rb3->CacheData();

		rb3->Integrate(deltaTime);
	}
}
*/

void Physics3State::Update(float deltaTime)
{
	UpdateInput(deltaTime);			// update input
	UpdateGameobjects(deltaTime);	// update gameobjects
	UpdateDebugDraw(deltaTime);		// update debug draw
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
		//m_camera->GetView().Print();
	}
}

void Physics3State::UpdateKeyboard(float deltaTime)
{
	// Apply movement
	float leftRight = 0.f;
	float forwardBack = 0.f;
	float upDown = 0.f; 

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
		{
			GameObject* gameobject = m_gameObjects[idx];
			gameobject->m_drawBasis = !gameobject->m_drawBasis;
		}
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_V))
	{
		for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
		{
			GameObject* gameobject = m_gameObjects[idx];
			gameobject->m_debugOn = !gameobject->m_debugOn;
		}
	}
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
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_0) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_NONE;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_1) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_COLOR;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_2) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_UV;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_3) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_SURFACE_NORMAL;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_4) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_WORLD_NORMAL;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_8) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_DIFFUSE;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_9) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_SPECULAR;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_5))
	{
		// toggle debug draw of entity bounding sphere
		//m_g0->ToggleBoundSphereDebugDraw();
		//m_g1->ToggleBoundSphereDebugDraw();
		//m_g2->ToggleBoundSphereDebugDraw();
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_6))
	{
		// toggle debug draw of entity bounding box
		//m_g0->ToggleBoundBoxDebugDraw();
		//m_g1->ToggleBoundBoxDebugDraw();
		//m_g2->ToggleBoundBoxDebugDraw();
	}

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
		m_g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
		m_g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
		m_g0->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
		m_g0->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
		m_g0->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
		m_g0->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		m_g0->GetEntity()->SetLinearVelocity(Vector3::ZERO);

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_I))
		m_g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
		m_g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
		m_g1->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
		m_g1->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
		m_g1->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
		m_g1->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		m_g1->GetEntity()->SetLinearVelocity(Vector3::ZERO);

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
	UpdateForceRegistry(deltaTime);		// update force registry
	UpdateGameobjectsCore(deltaTime);	// update GO core
	UpdateContactGeneration();			// update contact generation
	UpdateContactResolution(deltaTime);	// update contact resolution
}

void Physics3State::UpdateDebugDraw(float deltaTime)
{
	DebugRenderUpdate(deltaTime);
	for (int i = 0; i < m_allResolver->GetCollisionData()->m_contacts.size(); ++i)
	{
		Contact3& contact = m_allResolver->GetCollisionData()->m_contacts[i];

		Vector3 point = contact.m_point;
		Vector3 end = point + contact.m_normal * contact.m_penetration;

		// causing lead due to property block
		//DebugRenderPoint(0.01f, 5.f, point, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);

		DebugRenderLine(0.f, point, end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
	}
}

void Physics3State::UpdateForceRegistry(float deltaTime)
{
	if (m_particleRegistry != nullptr)
		m_particleRegistry->UpdateForces(deltaTime);
	if (m_rigidRegistry != nullptr)
		m_rigidRegistry->UpdateForces(deltaTime);
}

void Physics3State::UpdateGameobjectsCore(float deltaTime)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
	{
		m_gameObjects[idx]->Update(deltaTime);
		if (m_gameObjects[idx]->m_dead)
		{
			delete m_gameObjects[idx];
			m_gameObjects[idx] = nullptr;
			std::vector<GameObject*>::iterator it = m_gameObjects.begin() + idx;
			m_gameObjects.erase(it);
			idx--;
		}
	}
}

void Physics3State::UpdateContactGeneration()
{
	m_iterResolver->ClearRecords();
	m_allResolver->ClearRecords();

	// hard constraints use iterative solver
	m_rod->FillContact(m_iterResolver->GetCollisionData()->m_contacts);

	// sphere
	for (uint idx1 = 0; idx1 < m_spheres.size(); ++idx1)
	{
		// sphere vs sphere
		for (uint idx2 = idx1 + 1; idx2 < m_spheres.size(); ++idx2)
		{
			Sphere* s1 = m_spheres[idx1];
			Sphere* s2 = m_spheres[idx2];

			SphereEntity3* se1 = dynamic_cast<SphereEntity3*>(s1->m_physEntity);
			SphereEntity3* se2 = dynamic_cast<SphereEntity3*>(s2->m_physEntity);

			const Sphere3& sph1 = se1->GetSpherePrimitive();
			const Sphere3& sph2 = se2->GetSpherePrimitive();

			CollisionDetector::Sphere3VsSphere3(sph1, sph2, m_allResolver->GetCollisionData());
		}

		// sphere vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Quad* quad = m_quads[idx2];

			SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
			QuadEntity3* qe = dynamic_cast<QuadEntity3*>(quad->m_physEntity);

			const Sphere3& sph = se->GetSpherePrimitive();
			const Plane& pl = qe->GetPlanePrimitive();

			CollisionDetector::Sphere3VsPlane3(sph, pl, m_allResolver->GetCollisionData());
		}

		// sphere vs aabb3
		for (uint idx2 = 0; idx2 < m_cubes.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Cube* cube = m_cubes[idx2];

			SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
			CubeEntity3* ce = dynamic_cast<CubeEntity3*>(cube->m_physEntity);

			const Sphere3& sph = se->GetSpherePrimitive();
			const AABB3& aabb3 = ce->GetCubePrimitive();

			CollisionDetector::Sphere3VsAABB3(sph, aabb3, m_allResolver->GetCollisionData());
		}
	}

	// cubes
	for (uint idx1 = 0; idx1 < m_cubes.size(); ++idx1)
	{
		// aabb3 vs aabb3
		for (uint idx2 = idx1 + 1; idx2 < m_cubes.size(); ++idx2)
		{
			Cube* c1 = m_cubes[idx1];
			Cube* c2 = m_cubes[idx2];

			CubeEntity3* ce1 = dynamic_cast<CubeEntity3*>(c1->m_physEntity);
			CubeEntity3* ce2 = dynamic_cast<CubeEntity3*>(c2->m_physEntity);

			const AABB3& aabb3_1 = ce1->GetCubePrimitive();
			const AABB3& aabb3_2 = ce2->GetCubePrimitive();

			CollisionDetector::AABB3VsAABB3Single(aabb3_1, aabb3_2, m_allResolver->GetCollisionData());
		}

		// aabb3 vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Cube* cube = m_cubes[idx1];
			Quad* plane = m_quads[idx2];

			CubeEntity3* ce = dynamic_cast<CubeEntity3*>(cube->m_physEntity);
			QuadEntity3* qe = dynamic_cast<QuadEntity3*>(plane->m_physEntity);

			const AABB3& aabb = ce->GetCubePrimitive();
			const Plane& pl = qe->GetPlanePrimitive();

			// todo: modify this so that it returns one contact
			CollisionDetector::AABB3VsPlane3Single(aabb, pl, m_allResolver->GetCollisionData());
		}
	}

	// plane
	for (uint idx1 = 0; idx1 < m_quads.size(); ++idx1)
	{
		// plane vs plane
		for (uint idx2 = idx1; idx2 < m_quads.size(); ++idx2)
		{

		}
	}

	TODO("Generate contact for free points later");
}

void Physics3State::UpdateContactResolution(float deltaTime)
{
	m_iterResolver->ResolveContacts(deltaTime);
	m_allResolver->ResolveContacts(deltaTime);
}

/*
void Physics3State::SingleContactUpdate()
{
	m_collisionData->ClearContacts();
	// sphere
	for (uint idx1 = 0; idx1 < m_spheres.size(); ++idx1)
	{
		// sphere vs sphere
		for (uint idx2 = idx1 + 1; idx2 < m_spheres.size(); ++idx2)
		{
			Sphere* s1 = m_spheres[idx1];
			Sphere* s2 = m_spheres[idx2];

			SphereEntity3* se1 = dynamic_cast<SphereEntity3*>(s1->m_physEntity);
			SphereEntity3* se2 = dynamic_cast<SphereEntity3*>(s2->m_physEntity);

			Sphere3& sph1 = se1->GetSpherePrimitive();
			Sphere3& sph2 = se2->GetSpherePrimitive();

			CollisionDetector::Sphere3VsSphere3(sph1, sph2, m_collisionData);
		}

		// sphere vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Quad* quad = m_quads[idx2];

			SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
			QuadEntity3* qe = dynamic_cast<QuadEntity3*>(quad->m_physEntity);

			Sphere3& sph = se->GetSpherePrimitive();
			Plane& pl = qe->GetPlanePrimitive();

			CollisionDetector::Sphere3VsPlane3(sph, pl, m_collisionData);
		}

		// sphere vs aabb3
		for (uint idx2 = 0; idx2 < m_cubes.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Cube* cube = m_cubes[idx2];

			SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
			CubeEntity3* ce = dynamic_cast<CubeEntity3*>(cube->m_physEntity);

			Sphere3& sph = se->GetSpherePrimitive();
			AABB3& aabb3 = ce->GetCubePrimitive();

			CollisionDetector::Sphere3VsAABB3(sph, aabb3, m_collisionData);
		}
	}

	// cubes
	for (uint idx1 = 0; idx1 < m_cubes.size(); ++idx1)
	{
		// aabb3 vs aabb3
		for (uint idx2 = idx1 + 1; idx2 < m_cubes.size(); ++idx2)
		{
			Cube* c1 = m_cubes[idx1];
			Cube* c2 = m_cubes[idx2];

			CubeEntity3* ce1 = dynamic_cast<CubeEntity3*>(c1->m_physEntity);
			CubeEntity3* ce2 = dynamic_cast<CubeEntity3*>(c2->m_physEntity);

			AABB3& aabb3_1 = ce1->GetCubePrimitive();
			AABB3& aabb3_2 = ce2->GetCubePrimitive();

			CollisionDetector::AABB3VsAABB3Single(aabb3_1, aabb3_2, m_collisionData);
		}

		// aabb3 vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Cube* cube = m_cubes[idx1];
			Quad* plane = m_quads[idx2];

			CubeEntity3* ce = dynamic_cast<CubeEntity3*>(cube->m_physEntity);
			QuadEntity3* qe = dynamic_cast<QuadEntity3*>(plane->m_physEntity);

			AABB3& aabb = ce->GetCubePrimitive();
			Plane& pl = qe->GetPlanePrimitive();

			// todo: modify this so that it returns one contact
			CollisionDetector::AABB3VsPlane3Single(aabb, pl, m_collisionData);
		}
	}

	// plane
	for (uint idx1 = 0; idx1 < m_quads.size(); ++idx1)
	{
		for (uint idx2 = idx1; idx2 < m_quads.size(); ++idx2)
		{

		}
	}
}
*/

void Physics3State::Render(Renderer* renderer)
{
	renderer->SetCamera(m_camera);
	renderer->ClearScreen(Rgba::BLACK);

	RenderGameobjects(renderer);
	m_forwardPath->RenderScene(m_sceneGraph);
}

void Physics3State::RenderGameobjects(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
	{
		if (!m_gameObjects[idx]->m_isInForwardPath)
			m_gameObjects[idx]->Render(renderer);
	}
}
