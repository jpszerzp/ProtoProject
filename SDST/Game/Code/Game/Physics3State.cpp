#include "Game/Physics3State.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/Console/DevConsole.hpp"  
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"
#include "Engine/Physics/3D/BoxEntity3.hpp"
#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Physics/3D/QuadRB3.hpp"
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

	m_gravity = new GravityRigidForceGenerator(Vector3::GRAVITY);
	
	// or, initialize into other shapes (cube, etc)
	//m_g0 = InitializePhysSphere(Vector3(15.f, -1.5f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_KINEMATIC, BODY_PARTICLE);
	//m_g1 = InitializePhysSphere(Vector3(0.f, 40.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	//m_g1->m_physEntity->SetFrozen(true);
	//Rigidbody3* rigid_g1 = static_cast<Rigidbody3*>(m_g1->GetEntity());
	//m_rigidRegistry->Register(rigid_g1, grg);

	/*
	Sphere* s2 = InitializePhysSphere(Vector3(10.f, 40.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	s2->m_physEntity->SetFrozen(true);
	Rigidbody3* rigid_s1 = static_cast<Rigidbody3*>(s2->GetEntity());
	m_rigidRegistry->Register(rigid_s1, m_gravity);
	*/
	//rigid_s1->m_angularVelocity = Vector3(0.f, 0.f, 5.f);

	/*
	//////////////////////////////// For rigid spring ////////////////////////////////
	// the rigid ball
	m_r1 = InitializePhysSphere(
		Vector3(30.f, 40.f, 5.f),
		Vector3::ZERO, Vector3::ONE, Rgba::MEGENTA,
		MOVE_DYNAMIC, BODY_RIGID);
	m_r1->m_physEntity->SetFrozen(true);

	// anchor
	Point* theAnchor = InitializePhysPoint(
		Vector3(30.f, 44.f, 5.f),
		Vector3::ZERO, 10.f, Rgba::MEGENTA,
		MOVE_STATIC, BODY_PARTICLE);

	// entities
	Entity3* anchor_ent = theAnchor->GetEntity();
	Rigidbody3* other_rigid = static_cast<Rigidbody3*>(m_r1->GetEntity());

	// set up anchor spring
	m_rigidAnchorSpring = new GeneralRigidAnchorSpring(anchor_ent, other_rigid, 3.f, 5.f);

	// force generator
	Vector3 attachLocal = Vector3::ZERO;
	Sphere* theSphere = static_cast<Sphere*>(m_r1);
	ProjectPlaneToSphere(Vector2(.01f, .01f), theSphere->GetRadius(), attachLocal);
	AnchorSpringRigidForceGenerator* asrfg = new AnchorSpringRigidForceGenerator(
		theAnchor->GetWorldPosition(), other_rigid, attachLocal, 3.f, 5.f);

	// force registration
	m_rigidRegistry->Register(other_rigid, asrfg);
	m_rigidRegistry->Register(other_rigid, grg);
	//////////////////////////////////////////////////////////////////////////////////
	*/

	//InitializePhysQuad(Vector3(0.f, 20.f, 0.f), Vector3(90.f, 0.f, -45.f), Vector3(20.f, 20.f, 1.f), Rgba::BLUE, MOVE_STATIC, BODY_PARTICLE);
	InitializePhysQuad(Vector3(0.f, 20.f, 0.f), Vector3(90.f, 0.f, -10.f), Vector3(2000.f, 2000.f, 1.f), Rgba::GREEN, MOVE_STATIC, BODY_RIGID);
	InitializePhysQuad(Vector3(0.f, 20.f, 0.f), Vector3(90.f, 0.f, 10.f), Vector3(2000.f, 2000.f, 1.f), Rgba::GREEN, MOVE_STATIC, BODY_RIGID);
	//m_g2 = InitializePhysQuad(Vector3(20.f, -2.f, 0.f), Vector3(90.f, 0.f, 0.f), Vector3(800.f, 800.f, 1.f), Rgba::GREEN, MOVE_STATIC, BODY_PARTICLE);
	//m_g3 = InitializePhysPoint(Vector3(25.f, 0.f, 0.f), Vector3::ZERO, 10.f, Rgba::WHITE, MOVE_STATIC, BODY_PARTICLE);
	
	/*
	// more customized game objects
	m_ballistics = SetupBallistics(ARTILLERY, Vector3(26.f, 0.f, 0.f), true, Rgba::GREEN);
	SetupBallistics(PISTOL, Vector3(27.f, 0.f, 0.f), true, Rgba::WHITE);
	SetupBallistics(LASER, Vector3(28.f, 0.f, 0.f), true, Rgba::WHITE);
	SetupBallistics(FIREBALL, Vector3(29.f, 0.f, 0.f), true, Rgba::WHITE);
	SetupBallistics(FREEFALL, Vector3(26.f, 0.f, -.5f), true, Rgba::CYAN);

	// generate one particle for firework
	SetupFireworks(5.f, Vector3(20.f, 0.f, 5.f), Vector3::ZERO, Vector3(0.f, 4.f, 0.f), Vector3(0.f, 4.f, 0.f), false);
	*/

	/*
	// points for springs
	Point* sp_point_0 = InitializePhysPoint(Vector3(20.f, 10.f, 5.f), Vector3::ZERO, 10.f, Rgba::RED, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* sp_point_1 = InitializePhysPoint(Vector3(20.f, 5.f, 5.f), Vector3::ZERO, 10.f, Rgba::CYAN, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* asp_point_0 = InitializePhysPoint(Vector3(15.f, 10.f, 5.f), Vector3::ZERO, 10.f, Rgba::RED, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* asp_point_1 = InitializePhysPoint(Vector3(15.f, 5.f, 5.f), Vector3::ZERO, 10.f, Rgba::CYAN, MOVE_STATIC, BODY_PARTICLE);
	sp_point_0->GetEntity()->SetFrozen(true);
	sp_point_1->GetEntity()->SetFrozen(true);
	asp_point_0->GetEntity()->SetFrozen(true);
	asp_point_1->GetEntity()->SetFrozen(true);

	// setting up registrations in the registry
	// A. springs
	m_spring = SetupSpring(sp_point_0, sp_point_1, 3.f, 3.f);		
	// two points initialized to be 5 units away, constraint by a spring system with rest length of 3
	// B. anchored spring
	m_anchorSpring = SetupAnchorSpring(asp_point_0, asp_point_1, 3.f, 3.f);

	// rod
	Point* rod_point_0 = InitializePhysPoint(Vector3(10.f, 5.f, 5.f), Vector3::ZERO, 10.f, Rgba::GREEN, MOVE_DYNAMIC, BODY_PARTICLE);
	Point* rod_point_1 = InitializePhysPoint(Vector3(5.f, 8.f, 5.f), Vector3::ZERO, 10.f, Rgba::GREEN, MOVE_DYNAMIC, BODY_PARTICLE);
	float dist = (rod_point_0->GetWorldPosition() - rod_point_1->GetWorldPosition()).GetLength();
	m_rod = SetupRod(dist, rod_point_0, rod_point_1);
	*/

	/*
	// verlet
	Ballistics* verlet_basic_ballistics = SetupBallistics(FREEFALL, Vector3(27.f, 0.f, -.5f), true, Rgba::MEGENTA);
	verlet_basic_ballistics->m_physEntity->SetVerlet(true);
	verlet_basic_ballistics->m_physEntity->SetVerletScheme(BASIC_VERLET);
	verlet_basic_ballistics->m_physEntity->SetEntityLastCenter(verlet_basic_ballistics->m_physEntity->GetEntityCenter());

	Ballistics* verlet_vel_ballistics = SetupBallistics(FREEFALL, Vector3(28.f, 0.f, -.5f), true, Rgba::PINK);
	verlet_vel_ballistics->m_physEntity->SetVerlet(true);
	verlet_vel_ballistics->m_physEntity->SetVerletScheme(VELOCITY_VERLET);
	*/

	float obj_rand_x = GetRandomFloatInRange(0.f, 360.f);
	float obj_rand_y = GetRandomFloatInRange(0.f, 360.f);
	float obj_rand_z = GetRandomFloatInRange(0.f, 360.f);
	m_g4 = InitializePhysBox(Vector3(25.f, 0.f, 10.f), Vector3(45.f), Vector3(1.f), Rgba::CYAN, MOVE_KINEMATIC, BODY_PARTICLE);
	m_g5 = InitializePhysBox(Vector3(30.f, 0.f, 10.f), Vector3(obj_rand_x, obj_rand_y, obj_rand_z), Vector3(1.f), Rgba::TEAL, MOVE_KINEMATIC, BODY_PARTICLE);

	//m_g4 = InitializePhysBox(Vector3(25.f, 0.f, 10.f), Vector3::ZERO, Vector3(1.f), Rgba::CYAN, MOVE_KINEMATIC, BODY_PARTICLE);
	//m_g5 = InitializePhysBox(Vector3(30.f, 0.f, 10.f), Vector3::ZERO, Vector3(1.f), Rgba::TEAL, MOVE_KINEMATIC, BODY_PARTICLE);

	//m_g4 = InitializePhysBox(Vector3(25.f, 0.f, 10.f), Vector3::ZERO, Vector3(1.f), Rgba::CYAN, MOVE_DYNAMIC, BODY_RIGID);
	//m_g5 = InitializePhysBox(Vector3(30.f, 0.f, 10.f), Vector3::ZERO, Vector3(1.f), Rgba::TEAL, MOVE_DYNAMIC, BODY_RIGID);
	//Rigidbody3* rigid_b_4 = static_cast<Rigidbody3*>(m_g4->GetEntity());
	//Rigidbody3* rigid_b_5 = static_cast<Rigidbody3*>(m_g5->GetEntity());
	//rigid_b_4->SetAwake(true);
	//rigid_b_4->SetCanSleep(false);
	//rigid_b_5->SetAwake(true);
	//rigid_b_5->SetCanSleep(false);

	m_iterResolver = new ContactResolver(2);
	m_allResolver = new ContactResolver();
	m_coherentResolver = new ContactResolver(RESOLVE_COHERENT);

	// quick hull
	Vector3 qhMin = Vector3(-100.f, 110.f, 0.f);
	Vector3 qhMax = Vector3(-50.f, 160.f, 50.f);
	m_qh = new QuickHull(5, qhMin, qhMax);
	g_hull = m_qh;

	// wrap around field
	Vector3 wraparoundMin = Vector3(0.f, 110.f, 0.f);
	Vector3 wraparoundMax = Vector3(20.f, 140.f, 30.f);
	m_wraparound = new WrapAround(wraparoundMin, wraparoundMax);
	wraparoundMin = Vector3(28.f, 118.f, 8.f);
	wraparoundMax = Vector3(37.f, 127.f, 17.f);
	m_wraparound_0 = new WrapAround(wraparoundMin, wraparoundMax);

	// temporary - assimp test
	m_assimp_0 = new AssimpLoader("nanosuit/nanosuit.obj");
	int limit = 0;
	for (std::set<Vector3>::iterator it = m_assimp_0->m_vertPos.begin();
		it != m_assimp_0->m_vertPos.end(); ++it)
	{
		if (limit == 100)
		{
			m_modelPoints.emplace(*it);
			limit = 0;

			// also generate a mesh for this point
			Mesh* ptMesh = Mesh::CreatePointImmediate(VERT_PCU, *it, Rgba::RED);
			m_modelPointMeshes.push_back(ptMesh);
		}

		limit++;
	}

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

Physics3State::~Physics3State()
{
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

	delete m_wraparound;
	m_wraparound = nullptr;

	delete m_wraparound_0;
	m_wraparound_0 = nullptr;
}


Sphere* Physics3State::InitializePhysSphere(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	Sphere* s = new Sphere(pos, rot, scale, tint, "sphere_pcu", "default", moveStat, bid);
	s->m_physDriven = true;
	m_gameObjects.push_back(s);
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

Box* Physics3State::InitializePhysBox(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	Box* b = new Box(pos, rot, scale, tint, "cube_pcu", "default", moveStat, bid);
	b->m_physDriven = true;
	m_gameObjects.push_back(b);
	m_boxes.push_back(b);
	b->m_physEntity->SetGameobject(b);

	return b;
}


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

void Physics3State::Update(float deltaTime)
{
	m_broadPhase = g_broadphase;		// hacky...
	UpdateInput(deltaTime);				// update input
	UpdateGameobjects(deltaTime);		// update gameobjects
	UpdateDebugDraw(deltaTime);			// update debug draw
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
		if (m_g0 != nullptr)
			m_g0->ToggleBoundSphereDebugDraw();
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_6))
	{
		// toggle debug draw of entity bounding box
		if (m_g0 != nullptr)
			m_g0->ToggleBoundBoxDebugDraw();
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_T)
		&& !DevConsoleIsOpen())
	{
		if (m_broadPhase)
		{
			// get rigid body expect to insert
			Entity3* ent = m_gameObjects[m_nodeCount]->GetEntity();

			// add gameobject to BVH
			if (m_node == nullptr)
				m_node = new BVHNode<BoundingSphere>(nullptr, ent->GetBoundingSphere(), ent);
			else
				m_node->Insert(ent, ent->GetBoundingSphere());

			m_nodeCount++;
		}
	}
	// test removing from leaf
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_R)
		&& !DevConsoleIsOpen())
	{
		if (m_node != nullptr && m_broadPhase)
		{
			BVHNode<BoundingSphere>* leaf = m_node->GetRightLeaf();

			if (leaf != nullptr)
			{
				if (leaf->m_parent == nullptr)
					m_node = nullptr;

				delete leaf;
				m_nodeCount--;
			}
		}
	}
	// test removing branch
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Y)
		&& !DevConsoleIsOpen())
	{
		if (m_broadPhase)
		{
			BVHNode<BoundingSphere>* branch = m_node->m_children[1];

			if (branch != nullptr)
				delete branch;

			m_nodeCount = 1;
		}
	}

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
		m_g5->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
		m_g5->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
		m_g5->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
		m_g5->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
		m_g5->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
		m_g5->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		m_g5->GetEntity()->SetLinearVelocity(Vector3::ZERO);

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_I))
		m_g4->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
		m_g4->GetEntity()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
		m_g4->GetEntity()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
		m_g4->GetEntity()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
		m_g4->GetEntity()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
		m_g4->GetEntity()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		m_g4->GetEntity()->SetLinearVelocity(Vector3::ZERO);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0))
	{
		// add a conflict point
		bool removed = false;
		if (!m_debug_vert_complete)
		{
			QHVert* vert = g_hull->GetVert((int)m_debug_vert_count);
			removed = g_hull->AddConflictPointInitial(vert);

			// if there is nothing removed, we increment the index to go to next point
			// so that next time 0 is pressed, correct point is used
			if (!removed)
				m_debug_vert_count++;
			// if there is something removed, the index will just match

			// if we have gone thru every point in global candidate list,
			// we are done with adding conflict points, so we should not come to this spot anymore
			if (m_debug_vert_count == g_hull->GetVertNum())
				m_debug_vert_complete = true;
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_1))
	{
		// find eye
		if (!m_eye_found)
		{
			// get the farthest conflict point
			float farthest;
			g_hull->m_eyePair = g_hull->GetFarthestConflictPair(farthest);
			QHFace* conflict_face = std::get<0>(g_hull->m_eyePair);
			QHVert* conflict_pt = std::get<1>(g_hull->m_eyePair);

			// initialization of the horizon generation step: we need to push the chosen conflict face as a start
			if (conflict_face != nullptr && conflict_pt != nullptr)
			{
				g_hull->m_visibleFaces.push_back(conflict_face);
				g_hull->m_allFaces.push_back(conflict_face);
				g_hull->test_he = conflict_face->m_entry;
				g_hull->test_he_twin = g_hull->test_he->m_twin;
				g_hull->test_otherFace = g_hull->test_he_twin->m_parentFace;
				g_hull->test_start_he = g_hull->test_he;

				g_hull->ChangeCurrentHalfEdgeMesh();
			}

			m_eye_found = true;
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_2))
	{
		QHFace* conflict_face = std::get<0>(g_hull->m_eyePair);
		QHVert* conflict_pt = std::get<1>(g_hull->m_eyePair);

		if (!g_hull->m_visibleFaces.empty())
		{
			bool visited = std::find(g_hull->m_allFaces.begin(), 
				g_hull->m_allFaces.end(), g_hull->test_otherFace) != g_hull->m_allFaces.end();
			if (!visited)
			{
				if (g_hull->PointOutBoundFace(conflict_pt->vert, *g_hull->test_otherFace))
				{
					// visible 
					g_hull->m_visibleFaces.push_back(g_hull->test_otherFace);
					g_hull->m_allFaces.push_back(g_hull->test_otherFace);

					g_hull->test_he = g_hull->test_he_twin->m_next;
					g_hull->test_he_twin = g_hull->test_he->m_twin;
					g_hull->test_otherFace = g_hull->test_he_twin->m_parentFace;
				}
				else
				{
					// invisible
					bool explored = std::find(g_hull->m_horizon.begin(), 
						g_hull->m_horizon.end(), g_hull->test_he) != g_hull->m_horizon.end();
					bool twin_explored = std::find(g_hull->m_horizon.begin(), 
						g_hull->m_horizon.end(), g_hull->test_he->m_twin) != g_hull->m_horizon.end();
					if (!explored && !twin_explored)
					{
						g_hull->m_horizon.push_back(g_hull->test_he);
						g_hull->AddHorizonMesh(g_hull->test_he);
					}

					g_hull->test_he = g_hull->test_he->m_next;
					g_hull->test_he_twin = g_hull->test_he->m_twin;
					g_hull->test_otherFace = g_hull->test_he_twin->m_parentFace;

					// Situation 1 - we are still on this face, there is a chance this is the finish step
					if (g_hull->test_he == g_hull->test_start_he)
					{
						g_hull->m_visibleFaces.pop_back();
					}
				}
			}
			else
			{
				if (g_hull->m_visibleFaces.size() == 1U)
				{
					// Situation 2 - we know we are back on initial conflict face
					g_hull->test_he = g_hull->test_he->m_next;
					g_hull->m_visibleFaces.pop_back();
				}
				else
				{
					size_t size = g_hull->m_visibleFaces.size();
					QHFace* second = g_hull->m_visibleFaces[size - 2U];

					if (second == g_hull->test_otherFace)
					{
						if (g_hull->test_otherFace == conflict_face)
						{
							g_hull->test_he = g_hull->test_he_twin;
							g_hull->test_he_twin = g_hull->test_he->m_twin;
							g_hull->test_otherFace = conflict_face;				// force stay on initial face
							g_hull->m_visibleFaces.pop_back();
						}
						else
						{
							g_hull->test_he = g_hull->test_he_twin->m_next;
							g_hull->test_he_twin = g_hull->test_he->m_twin;
							g_hull->test_otherFace = g_hull->test_he_twin->m_parentFace;
							g_hull->m_visibleFaces.pop_back();
						}
					}
					else
					{
						g_hull->test_he = g_hull->test_he->m_next;
						g_hull->test_he_twin = g_hull->test_he->m_twin;
						g_hull->test_otherFace = g_hull->test_he_twin->m_parentFace;

						//// Situation 1 - we are still on this face, there is a chance this is the finish step - NOT FOR THIS ONE SINCE WE ARE NOT ON INITIAL FACE
						//if (g_hull->test_he == g_hull->test_start_he)
						//{
						//	g_hull->m_visibleFaces.pop_back();
						//}
					}
				}
			}
		}

		if (g_hull->m_visibleFaces.empty())
		{
			if (g_hull->test_he != g_hull->test_start_he)
			{
				// we may miss other half edges for the initial face, so we added it back and continue processing
				if (g_hull->m_visibleFaces.empty())
					g_hull->m_visibleFaces.push_back(conflict_face);

				g_hull->test_he_twin = g_hull->test_he->m_twin;
				g_hull->test_otherFace = g_hull->test_he_twin->m_parentFace;

				//bool visited = std::find(g_hull->m_allFaces.begin(), g_hull->m_allFaces.end(),
				//	g_hull->test_otherFace) != g_hull->m_allFaces.end();
				//if (visited)
				//	g_hull->test_he = g_hull->test_he->m_next;
				//else
				//	ASSERT_OR_DIE(false, "Should not have unvisited neighbor at this stage");
			}
			//else
			//	ASSERT_RECOVERABLE(false, "Looped back to initial edge, do nothing");
		}

		g_hull->ChangeCurrentHalfEdgeMesh();
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3))
	{
		// it is easier for us to first delete old faces and then generate new ones

		// we first get the list of faces that are visible to the eye:
		// these are the faces we need to delete from the hull
		QHVert* eye = std::get<1>(g_hull->m_eyePair);
		//std::vector<QHFace*>& eyeVisibles = eye->visibleFaces;
		std::deque<QHFace*>& eyeVisibles = g_hull->m_allFaces;
		
		// find face expected to be deleted in hull's storage
		for (std::deque<QHFace*>::size_type idx1 = 0; idx1 < eyeVisibles.size(); ++idx1)
		{
			QHFace* deleteThis = eyeVisibles[idx1];

			for (std::deque<QHFace*>::size_type idx2 = 0; idx2 < g_hull->m_faces.size(); ++idx2)
			{
				QHFace* hullFace = g_hull->m_faces[idx2];
				if (deleteThis == hullFace)
				{
					// face found in hull's storage 
					// we need to delete it from the storage
					std::vector<QHFace*>::iterator it = g_hull->m_faces.begin() + idx2;
					g_hull->m_faces.erase(it);
					idx2--;							// optional - adjust index

					// before releasing memory of faces, we need to get orphanage of this face
					for (QHVert* v : deleteThis->conflicts)
					{
						if (v != eye)
						{
							// if the vert is NOT eye, just add it as orphan
							g_hull->m_orphans.push_back(v);
						}
						// this face not necessarily has eye as conflicts (imagine the feature is an edge, 
						// point will only be put as conflict point of one of the two faces that share that edge),
						// but if it has, we DO NOT want to put it as conflict point of any face any more,
						// instead, it is just going to be the point added to hull's peripheral, so we keep it in m_eyePair for later reference
					}

					// now that we have cached orphans, we can free this face
					// but things are a little complex here, some cases we need to consider:
					// first, we only want to delete half edges that are not in the horizon list already,
					// since horizon are edges we are about to use to construct new faces.
					// second, we MAY need to also NOT delete the halfedge if it is the TWIN of some edge in horizon, 
					// because the two half edges represent the same geometrical edge, and that edge is still used construct new faces
					// To start with, we get all halfedges of the face about to be deleted
					HalfEdge* it_he = deleteThis->m_entry;
					HalfEdge* prev = it_he->m_prev;
					HalfEdge* next = it_he->m_next;
					// now that we get all three half edges, for each of them, we check if the edge is in horizon list
					// if yes, then we do NOT want to delete this halfedge
					// if no, then we can delete this one
					if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), it_he) == g_hull->m_horizon.end())
						// in other words, delete the entry if we cannot find it as part of horizon
						delete it_he;

					if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), prev) == g_hull->m_horizon.end())
						delete prev;

					if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), next) == g_hull->m_horizon.end())
						delete next;

					
					// we want to unregister the meshes for sure, but could be handled by destructor later

					// we do NOT want to handle freeing memory for conflict points, 
					// as it is either in orphan vector or is stored as eye right now
					// we simple clear the vector
					deleteThis->conflicts.clear();

					// finally, before we delete this face, we need to use find one point on this face, 
					// and set it as anchor to be used when setting normals of new faces.
					// This is applicable because this point is guaranteed to be inside the new hull due to
					// the fact that it is on an obsolete face/
					// Note that we only need one such point, so this assignment should only happen once
					if (Vector3::EqualByTolerance(0.1f, g_hull->m_anchor, Vector3::INVALID))
					{
						// this is first time we need to set the anchor
						// a good candidate is the centroid of the face
						Vector3 interior = deleteThis->GetFaceCentroid();
						g_hull->m_anchor = interior;
					}

					// now we can delete this face, we don't need any data from it anymore
					delete deleteThis;

					// now that we have deleted this face, we do not need to keep searching for it in hull's storage
					// we go on processing next face we want to delete
					// before doing that, also adjust counters and storage of the eyeVisibles because of the face deleted
					std::deque<QHFace*>::iterator it_eye_visibles = eyeVisibles.begin() + idx1;
					eyeVisibles.erase(it_eye_visibles);
					idx1--;						// this is REQUIRED, we ARE going into the rest of this loop later
					break;
				}
			}

			// going to the next face we want to delete
		}

		// visible list of eye should be empty at this moment, we make sure we clear it here
		ASSERT_OR_DIE(eyeVisibles.empty(), "Visible face list of eye (allFaces of hull) should be empty but is not, abort.");
		eyeVisibles.clear();
		std::get<0>(g_hull->m_eyePair) = nullptr;			// note that the face in eye pair should be one of those faces deleted, so we set null here

		// Up to this point, face free logic should come at an end
		// now we need to think about how to add new faces to the hull
		// to start on that, remember that we still have the eye and the horizon list
		// NOTE: HE is on the visible side when pushed, so we can just keep that direction for new faces

		// holder for new faces
		std::vector<QHFace*> new_faces;
		for (HalfEdge* he : g_hull->m_horizon)
		{
			// with eye position, we can form a face given each half edge in the horizon list
			const Vector3& eyePos = eye->vert;
			QHFace* new_face = new QHFace(he, eyePos);

			// remember that for each half edge we have a parent face
			// so we want to make sure each halfedge of the new face actually configures that in the right way
			new_face->SetParentHalfEdge();

			// for each half edge, information set up at this moment looks like the following
			// ----- prev ----- next ----- tail ----- parent ----- twin -----
			// -he1- SET  ----- SET  ----- SET  ----- SET    ----- SET  -----
			// -he2- SET  ----- SET  ----- SET  ----- SET    ----- NOT  -----
			// -he3- SET  ----- SET  ----- SET  ----- SET    ----- NOT  -----

			new_faces.push_back(new_face);
		}

		// fully set all twin relations
		for (std::vector<QHFace*>::size_type i1 = 0; i1 < new_faces.size(); ++i1)
		{
			QHFace* subject = new_faces[i1];

			for (std::vector<QHFace*>::size_type i2 = 0; i2 <new_faces.size(); ++i2)
			{
				QHFace* object = new_faces[i2];
				if (subject != object)
				{
					// for each new face, there is a possibility that it shares an edge with another new face
					// once we find that shared edge, the half edge corresponding to it in that other face is just the twin
					// of the half edge corresponding to this shared edge in this current face we are inspecting
					subject->UpdateSharedEdge(object);		// set twins along the way
				}
			}
		}
		// all data set for HE: tail, prev, next, twin, parent face

		// at this moment we have HE relation of all new faces configured
		// we also have feature id and verts set correctly
		// some other stuff we care absolutely: normal, face mesh, conflicts (from the orphans)
		// the rest of properties we can also add here per needed
		// so we set up the above one by one
		for (std::vector<QHFace*>::size_type i = 0; i < new_faces.size(); ++i)
		{
			QHFace* theNew = new_faces[i];

			// face mesh
			g_hull->CreateFaceMesh(*theNew);

			// normal
			// remember that we stored a anchor that is always valid to compute the outbound normal
			g_hull->GenerateOutboundNorm(g_hull->m_anchor, *theNew);
		}

		// re-assigning orphaned conflict points is a little more complex
		for (QHVert* orphan : g_hull->m_orphans)
			g_hull->AddConflictPointGeneral(orphan, new_faces); 
		// at this point the orphan storage is no longer important, we can just clear it
		g_hull->m_orphans.clear();

		// add faces to hull
		for (QHFace* addedFace : new_faces)
			g_hull->m_faces.push_back(addedFace);

		// there is one subtle op here: we need to set eye back to null as don't use it anymore
		// this will set the tupel too since std::get returns a reference, making the eyePair a <null, null> effectively
		eye = nullptr;

		// at the end we need to reset params for next round of adding point to hull
		// to avoid repeat ops, the variables reset already are:
		// m_verts has been updated; m_faces is updated; m_eyePair back to null for both value;
		// m_allFaces is cleared and its contained faces released;
		// m_orphans is cleared.
		// This leaves params m_visibleFaces, m_horizon, test params, all meshes, m_anchor to be reset
		if (!g_hull->m_visibleFaces.empty())
			g_hull->m_visibleFaces.clear();
		if (!g_hull->m_horizon.empty())
			g_hull->m_horizon.clear();
		g_hull->test_start_he = nullptr;
		g_hull->test_he = nullptr;
		g_hull->test_he_twin = nullptr;
		g_hull->test_otherFace = nullptr;
		for (Mesh* horizonMesh : g_hull->m_horizon_mesh)
			delete horizonMesh;
		g_hull->m_horizon_mesh.clear();
		if (g_hull->m_test_he_mesh != nullptr)
		{
			delete g_hull->m_test_he_mesh;
			g_hull->m_test_he_mesh = nullptr;
		}
		g_hull->m_anchor = Vector3::INVALID;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// horizon list is complete, form s with conflicting point
		// NOTE: since this he is on the visible side when pushed, we keep the direction
		
		/*
		// get reference to new faces
		std::vector<QHFace*> faces;
		QHVert* eye = std::get<1>(g_hull->m_eyePair);

		for (HalfEdge* he : g_hull->m_horizon)
		{
			// with eye position, we can form a face given each half edge
			Vector3 eyePos = eye->vert;
			QHFace* face = new QHFace(he, eyePos);

			// remember that for each half edge we have a parent face
			// so we want to make sure each halfedge of the new face actually configures that in the right way
			face->SetParentHalfEdge();

			// for each half edge, information set up at this moment looks like the following
			// ----- prev ----- next ----- tail ----- parent ----- twin -----
			// -he1- SET  ----- SET  ----- SET  ----- SET    ----- SET  -----
			// -he2- SET  ----- SET  ----- SET  ----- SET    ----- NOT  -----
			// -he3- SET  ----- SET  ----- SET  ----- SET    ----- NOT  -----

			faces.push_back(face);
		}

		// fully set all twin relations
		for (std::vector<QHFace*>::size_type i1 = 0; i1 < faces.size(); ++i1)
		{
			QHFace* subject = faces[i1];

			for (std::vector<QHFace*>::size_type i2 = 0; i2 <faces.size(); ++i2)
			{
				QHFace* object = faces[i2];
				if (subject != object)
				{
					// for each new face, there is a possibility that it shares an edge with another new face
					// once we find that shared edge, the half edge corresponding to it in that other face is just the twin
					// of the half edge corresponding to this shared edge in this current face we are inspecting
					subject->UpdateSharedEdge(object);		// set twins along the way
				}
			}
		}
		// all set for HE: tail, prev, next, twin, parent face

		// delete outdated faces that cannot lie on hull
		std::vector<QHVert*> orphaned;							// need to take the orphans		
		// to get "visible combo" face-edge-face or face-face-face-point combo of a point,
		// remember that we could have cached such visible faces info already, either when finding
		// closest feature in the case of initial hull, or from after adjust this exact info for orphaned points
		// The ponit being: it is safe to assume that visibleFaces contains wanted faces
		std::vector<QHFace*>& eyeVisibles = eye->visibleFaces;
		for (std::vector<QHFace*>::size_type idx1 = 0; idx1 < eyeVisibles.size(); ++idx1)
		{
			QHFace* oldFace = eyeVisibles[idx1];

			for (std::vector<QHFace*>::size_type idx2 = 0; idx2 < g_hull->m_faces.size(); ++idx2)
			{
				// find this old face in hull's face storage
				QHFace* hullFace = g_hull->m_faces[idx2];
				if (oldFace == hullFace)
				{
					// face found in hull's storage 
					// we need to delete it from the storage
					std::vector<QHFace*>::iterator it = g_hull->m_faces.begin() + idx2;
					g_hull->m_faces.erase(it);
					idx2--;					// adjust index

					// before releasing memory of faces, we need to get orphanage of this face
					for (QHVert* v : oldFace->conflicts)
					{
						if (v != eye)
						{
							// if the vert is NOT eye, just add it as orphan
							g_hull->m_orphans.push_back(v);
						}
						// this face not necessarily has eye as conflicts (imagine the feature is an edge, 
						// point will only be put as conflict point of one of the two faces that share that edge),
						// but if it has, we DO NOT want to put it as conflict point of any face any more,
						// instead, it is just going to be the point added to hull's peripheral, so we keep it in m_eyePair for later reference
					}

					// PROBLEM: RELATION OF NEXT, AND PREV HAS BEEN CHANGED IF WE CONSTRUCT NEW FACES FIRST, THAT RELATION IS NOT APPLICABLE HERE

					// now that we have cached orphans, we can free this face
					HalfEdge* it_he = hullFace->m_entry;
					HalfEdge* prev = it_he->m_prev;
					HalfEdge* next = it_he->m_next;
					// assuming triangle face
					
					if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), it_he) == g_hull->m_horizon.end())
						// in other words, delete the entry if we cannot find it as part of horizon
						delete it_he;

					if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), prev) == g_hull->m_horizon.end())
						delete prev;

					if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), next) == g_hull->m_horizon.end())
						delete next;

					delete hullFace;
					break;
					// since we have found the face we want to delete and have deleted it
					// we go to the next face we wish to delete, which is stored in eyeVisibles
				}
			}

			std::vector<QHFace*>::iterator it = eyeVisibles.begin() + idx1;
			eyeVisibles.erase(it);

			idx1--;
		}
		// faces are deleted in both global hull and eye visibles
		// orphans are stored and waiting to be distributed
		// candidate is ready to be added to the vert list of hull

		// add faces and point to hull
		for (QHFace* addedFace : faces)
			g_hull->m_faces.push_back(addedFace);
		g_hull->m_verts.push_back(g_hull->m_candidate);

		// after new faces are added, distributed orphans from abandoned faces
		for (QHVert* orphan : g_hull->m_orphans)
			g_hull->AddConflictPointGeneral(orphan, faces);

		// clean up temporaries for next round of adding points
		std::get<0>(g_hull->m_eyePair) = nullptr;
		std::get<1>(g_hull->m_eyePair) = nullptr;
		if (!g_hull->m_visibleFaces.empty())
			g_hull->m_visibleFaces.clear();
		if (!g_hull->m_allFaces.empty())
			g_hull->m_allFaces.clear();
		if (!g_hull->m_horizon.empty())
			g_hull->m_horizon.clear();
		for (Mesh* tempMesh : g_hull->m_horizon_mesh)
			delete tempMesh;
		g_hull->test_start_he = nullptr;
		g_hull->test_he = nullptr;
		g_hull->test_he_twin = nullptr;
		g_hull->test_otherFace = nullptr;
		if (!g_hull->m_orphans.empty())
			g_hull->m_orphans.clear();
		g_hull->m_candidate = nullptr;
		*/
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE))
	{
		// shoot a ball from camera
		//ShootBallFromCamera();

		WrapAroundTestBox();
		WrapAroundTestBall();
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
	UpdateForceRegistry(deltaTime);			// update force registry
	UpdateGameobjectsCore(deltaTime);		// update GO core
	UpdateBVH();							// update BVH 
	UpdateContactGeneration();				// update contact generation
	UpdateContactResolution(deltaTime);		// contact resolution
}

void Physics3State::UpdateDebugDraw(float deltaTime)
{
	// debug draw for all-at-once resolver
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

	// debug draw for iterative resolver
	TODO("Debug draw for iterative resolver");

	// debug draw for coherent resolver
	for (int i = 0; i < m_coherentResolver->GetCollisionData()->m_contacts.size(); ++i)
	{
		Contact3& contact = m_coherentResolver->GetCollisionData()->m_contacts[i];

		Vector3 point = contact.m_point;
		Vector3 end = point + contact.m_normal * contact.m_penetration;
		DebugRenderLine(0.1f, point, end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
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

	// teleport for wraparound
	m_wraparound->Update();
	m_wraparound_0->Update();
	
	if (m_physBall != nullptr)
	{
		Rigidbody3* rigid_s = static_cast<Rigidbody3*>(m_physBall->GetEntity());
		const Vector3& angVel = rigid_s->GetAngularVelocity();
		const Quaternion& orient = rigid_s->GetQuaternion();
		DebuggerPrintf("angular velocity: %f, %f, %f\n", angVel.x, angVel.y, angVel.z);
		DebuggerPrintf("orientation: %f, %f, %f; real: %f\n", orient.m_imaginary.x, orient.m_imaginary.y, orient.m_imaginary.z, orient.m_real);
	}
}

void Physics3State::UpdateContactGeneration()
{
	if (!m_broadPhase)
	{
		///////////////////////////////////////////// Clear Resolver ////////////////////////////////////////////
		m_iterResolver->ClearRecords();
		m_allResolver->ClearRecords();
		m_coherentResolver->ClearRecords();

		///////////////////////////////////////////// Fill Resolver ////////////////////////////////////////////
		// hard constraints use iterative solver
		//m_rod->FillContact(m_iterResolver->GetCollisionData()->m_contacts);

		UpdateCore();
	}
	else
	{
		TODO("BVH also considers the coherent solver");
		m_iterResolver->ClearRecords();
		m_allResolver->ClearRecords();
		m_bvhContacts.clear();

		// rod constraint is always here
		m_rod->FillContact(m_iterResolver->GetCollisionData()->m_contacts);

		// broad phase, use BVH to generate contacts
		if (m_node != nullptr)
		{
			m_node->GetContacts(m_bvhContacts, 1000);

			for (const BVHContact& bvhc : m_bvhContacts)
			{
				Entity3* e1 = bvhc.m_rb1;
				Entity3* e2 = bvhc.m_rb2;

				CollisionDetector::Entity3VsEntity3(e1, e2, m_allResolver->GetCollisionData());
			}
		}
	}
}

void Physics3State::UpdateCore()
{
	// sphere
	for (uint idx1 = 0; idx1 < m_spheres.size(); ++idx1)
	{
		// sphere vs sphere
		for (uint idx2 = idx1 + 1; idx2 < m_spheres.size(); ++idx2)
		{
			Sphere* s1 = m_spheres[idx1];
			Sphere* s2 = m_spheres[idx2];

			if (s1->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
			{
				if (s2->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereEntity3* se1 = dynamic_cast<SphereEntity3*>(s1->m_physEntity);
					SphereEntity3* se2 = dynamic_cast<SphereEntity3*>(s2->m_physEntity);

					Sphere3 sph1 = se1->GetSpherePrimitive();
					Sphere3 sph2 = se2->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Single(sph1, sph2, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(s1->m_physEntity);
					SphereRB3* srb = dynamic_cast<SphereRB3*>(s2->m_physEntity);

					Sphere3 sph1 = se->GetSpherePrimitive();
					Sphere3 sph2 = srb->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Single(sph1, sph2, m_allResolver->GetCollisionData());
				}
			}
			else
			{
				if (s2->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(s1->m_physEntity);
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(s2->m_physEntity);

					Sphere3 sph1 = srb->GetSpherePrimitive();
					Sphere3 sph2 = se->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Single(sph1, sph2, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereRB3* srb1 = dynamic_cast<SphereRB3*>(s1->m_physEntity);
					SphereRB3* srb2 = dynamic_cast<SphereRB3*>(s2->m_physEntity);

					Sphere3 sph1 = srb1->GetSpherePrimitive();
					Sphere3 sph2 = srb2->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Coherent(sph1, sph2, m_coherentResolver->GetCollisionData());
				}
			}
		}

		// sphere vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Quad* quad = m_quads[idx2];

			if (sphere->m_physEntity->GetEntityBodyID() == BODY_RIGID)
			{
				if (quad->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					QuadEntity3* qe = dynamic_cast<QuadEntity3*>(quad->m_physEntity);

					Sphere3 sph = srb->GetSpherePrimitive();
					Plane pl = qe->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Single(sph, pl, m_allResolver->GetCollisionData());
				}
				else 
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					QuadRB3* qrb = dynamic_cast<QuadRB3*>(quad->m_physEntity);

					Sphere3 sph = srb->GetSpherePrimitive();
					Plane pl = qrb->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Coherent(sph, pl, m_coherentResolver->GetCollisionData());
				}
			}
			else 
			{
				if (quad->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					QuadEntity3* qe = dynamic_cast<QuadEntity3*>(quad->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					Plane pl = qe->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Single(sph, pl, m_allResolver->GetCollisionData());
				}
				else 
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					QuadRB3* qrb = dynamic_cast<QuadRB3*>(quad->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					Plane pl = qrb->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Single(sph, pl, m_allResolver->GetCollisionData());
				}
			}
		}

		// sphere vs obb3
		for (uint idx2 = 0; idx2 < m_boxes.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Box* box = m_boxes[idx2];

			if (sphere->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
			{
				if (box->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(box->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					OBB3 obb = be->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3(obb, sph, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					OBB3 obb = brb->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3(obb, sph, m_allResolver->GetCollisionData());
				}
			}
			else 
			{
				if (box->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(box->m_physEntity);

					Sphere3 sph = srb->GetSpherePrimitive();
					OBB3 obb = be->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3(obb, sph, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereRB3* se = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					OBB3 obb = brb->GetBoxPrimitive();

					//CollisionDetector::OBB3VsSphere3(obb, sph, m_allResolver->GetCollisionData());
				}
			}
		}
	}

	// obb3
	for (uint idx1 = 0; idx1 < m_boxes.size(); ++idx1)
	{
		// obb3 vs obb3: for this we want coherent contacts
		for (uint idx2 = idx1 + 1; idx2 < m_boxes.size(); ++idx2)
		{
			Box* b1 = m_boxes[idx1];
			Box* b2 = m_boxes[idx2];

			if (b1->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
			{
				if (b2->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					BoxEntity3* be1 = dynamic_cast<BoxEntity3*>(b1->m_physEntity);
					BoxEntity3* be2 = dynamic_cast<BoxEntity3*>(b2->m_physEntity);

					OBB3 obb1 = be1->GetBoxPrimitive();
					OBB3 obb2 = be2->GetBoxPrimitive();

					CollisionDetector::OBB3VsOBB3Single(obb1, obb2, m_allResolver->GetCollisionData());
				}
				else
				{
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(b1->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(b2->m_physEntity);

					OBB3 obb1 = be->GetBoxPrimitive();
					OBB3 obb2 = brb->GetBoxPrimitive();

					CollisionDetector::OBB3VsOBB3Single(obb1, obb2, m_allResolver->GetCollisionData());
				}
			}
			else
			{
				if (b2->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					BoxRB3* brb = dynamic_cast<BoxRB3*>(b1->m_physEntity);
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(b2->m_physEntity);

					OBB3 obb1 = brb->GetBoxPrimitive();
					OBB3 obb2 = be->GetBoxPrimitive();

					CollisionDetector::OBB3VsOBB3Single(obb1, obb2, m_allResolver->GetCollisionData());
				}
				else
				{
					BoxRB3* brb1 = dynamic_cast<BoxRB3*>(b1->m_physEntity);
					BoxRB3* brb2 = dynamic_cast<BoxRB3*>(b2->m_physEntity);

					OBB3 obb1 = brb1->GetBoxPrimitive();
					OBB3 obb2 = brb2->GetBoxPrimitive();
					
					CollisionDetector::OBB3VsOBB3Coherent(obb1, obb2, m_coherentResolver->GetCollisionData());
				}
			}
		}

		// obb3 vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Box* box = m_boxes[idx1];
			Quad* plane = m_quads[idx2];

			if (box->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
			{
				if (plane->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(box->m_physEntity);
					QuadEntity3* qe = dynamic_cast<QuadEntity3*>(plane->m_physEntity);

					OBB3 obb = be->GetBoxPrimitive();
					Plane pl = qe->GetPlanePrimitive();

					CollisionDetector::OBB3VsPlane3(obb, pl, m_allResolver->GetCollisionData());
				}
			}
			else
			{
				if (plane->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);
					QuadEntity3* qe = dynamic_cast<QuadEntity3*>(plane->m_physEntity);

					OBB3 obb = brb->GetBoxPrimitive();
					Plane pl = qe->GetPlanePrimitive();

					CollisionDetector::OBB3VsPlane3(obb, pl, m_allResolver->GetCollisionData());
				}
			}
		}
	}

	// plane
	for (uint idx1 = 0; idx1 < m_quads.size(); ++idx1)
	{
		// plane vs plane
		for (uint idx2 = idx1 + 1; idx2 < m_quads.size(); ++idx2)
		{

		}
	}

	TODO("Generate contact for free points later");
}


void Physics3State::UpdateContactResolution(float deltaTime)
{
	m_iterResolver->ResolveContacts(deltaTime);
	m_allResolver->ResolveContacts(deltaTime);
	m_coherentResolver->ResolveContacts(deltaTime);
}

void Physics3State::UpdateBVH()
{
	// update BVH bottom up
	if (m_node != nullptr && m_broadPhase)
		m_node->UpdateNode();
}

void Physics3State::Render(Renderer* renderer)
{
	renderer->SetCamera(m_camera);
	renderer->ClearScreen(Rgba::BLACK);

	m_qh->RenderHull(renderer);
	renderer->DrawModel(m_assimp_0);

	RenderModelSamples(renderer);

	RenderGameobjects(renderer);
	m_forwardPath->RenderScene(m_sceneGraph);

	RenderBVH(renderer);
}

void Physics3State::RenderGameobjects(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
	{
		if (!m_gameObjects[idx]->m_isInForwardPath)
			m_gameObjects[idx]->Render(renderer);
	}
}

void Physics3State::RenderBVH(Renderer* renderer)
{
	// traverse the BVH
	if (m_node != nullptr && m_broadPhase)
		m_node->DrawNode(renderer);
}

void Physics3State::RenderModelSamples(Renderer* renderer)
{
	Shader* shader = renderer->CreateOrGetShader("wireframe_color");
	renderer->UseShader(shader);

	Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());
	glPointSize(10.f);

	renderer->m_objectData.model = Matrix44::IDENTITY;

	renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
	renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
	renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;

	for (int i = 0; i < m_modelPointMeshes.size(); ++i)
	{
		Mesh* mesh = m_modelPointMeshes[i];
		renderer->DrawMesh(mesh);
	}
}

void Physics3State::WrapAroundTestBall()
{
	Vector3 positions[8] = {Vector3(10.f, 120.f, 10.f), Vector3(10.f, 120.f, 20.f), 
		Vector3(20.f, 120.f, 10.f), Vector3(20.f, 120.f, 20.f),
		Vector3(10.f, 130.f, 10.f), Vector3(10.f, 130.f, 20.f),
		Vector3(20.f, 130.f, 10.f), Vector3(20.f, 130.f, 20.f)};
	Vector3 pos = positions[m_wrapPosIterator];
	if (m_physBall == nullptr)
	{
		m_physBall = InitializePhysSphere(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_s = static_cast<Rigidbody3*>(m_physBall->GetEntity());
		rigid_s->SetLinearVelocity(GetRandomVector3() * 20.f);
		rigid_s->SetAwake(true);
		rigid_s->SetCanSleep(true);
		m_wraparound->m_gos.push_back(m_physBall);
	}
	else
	{
		Sphere* s = InitializePhysSphere(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());
		rigid_s->SetLinearVelocity(GetRandomVector3() * 20.f);
		rigid_s->SetAwake(true);
		rigid_s->SetCanSleep(true);
		m_wraparound->m_gos.push_back(s);
	}
	m_wrapPosIterator += 1;
	m_wrapPosIterator %= 8;
}

void Physics3State::WrapAroundTestBox()
{
	Vector3 positions_0[8] = {Vector3(30.f, 120.f, 10.f), Vector3(30.f, 120.f, 15.f),
		Vector3(35.f, 120.f, 10.f), Vector3(35.f, 120.f, 15.f),
		Vector3(30.f, 125.f, 10.f), Vector3(30.f, 125.f, 15.f),
		Vector3(35.f, 125.f, 10.f), Vector3(35.f, 125.f, 15.f)};
	Vector3 pos_0 = positions_0[m_wrapPosIterator_0];
	if (m_physBox == nullptr)
	{
		float obj_rand_x = GetRandomFloatInRange(0.f, 360.f);
		float obj_rand_y = GetRandomFloatInRange(0.f, 360.f);
		float obj_rand_z = GetRandomFloatInRange(0.f, 360.f);
		m_physBox = InitializePhysBox(pos_0, Vector3(obj_rand_x, obj_rand_y, obj_rand_z), Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_b = static_cast<Rigidbody3*>(m_physBox->GetEntity());
		rigid_b->SetLinearVelocity(GetRandomVector3() * 5.f);
		rigid_b->SetAwake(true);
		rigid_b->SetCanSleep(true);
		m_wraparound_0->m_gos.push_back(m_physBox);
	}
	else
	{
		float obj_rand_x = GetRandomFloatInRange(0.f, 360.f);
		float obj_rand_y = GetRandomFloatInRange(0.f, 360.f);
		float obj_rand_z = GetRandomFloatInRange(0.f, 360.f);
		Box* b = InitializePhysBox(pos_0, Vector3(obj_rand_x, obj_rand_y, obj_rand_z), Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_b = static_cast<Rigidbody3*>(b->GetEntity());
		rigid_b->SetLinearVelocity(GetRandomVector3() * 5.f);
		rigid_b->SetAwake(true);
		rigid_b->SetCanSleep(true);
		m_wraparound_0->m_gos.push_back(b);
	}
	m_wrapPosIterator_0 += 1;
	m_wrapPosIterator_0 %= 8;
}

void Physics3State::ShootBallFromCamera()
{
	Vector3 camPos = m_camera->GetWorldPosition();
	Vector3 camForward = m_camera->GetWorldForward().GetNormalized();
	Vector3 pos = camPos + camForward * 2.f;

	Sphere* s = InitializePhysSphere(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());
	m_rigidRegistry->Register(rigid_s, m_gravity);
	rigid_s->SetLinearVelocity(camForward * 10.f);
	rigid_s->SetAwake(true);
	rigid_s->SetCanSleep(true);
}
