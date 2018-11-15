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
	
	InitializePhysQuad(Vector3(0.f, 20.f, 0.f), Vector3(90.f, 0.f, 0.f), Vector3(2000.f, 2000.f, 1.f), Rgba::GREEN, MOVE_STATIC, BODY_RIGID);

	m_iterResolver = new ContactResolver(2);
	m_allResolver = new ContactResolver();
	m_coherentResolver = new ContactResolver(RESOLVE_COHERENT);

	// quick hull
	Vector3 qhMin = Vector3(-100.f, -50.f, 0.f);
	Vector3 qhMax = Vector3(-50.f, 0.f, 50.f);
	m_qh = new QuickHull(6, qhMin, qhMax);
	g_hull = m_qh;

	// wrap around field
	Vector3 wraparoundMin = Vector3(28.f, 118.f, 8.f);
	Vector3 wraparoundMax = Vector3(37.f, 127.f, 17.f);
	m_wraparound_0 = new WrapAround(wraparoundMin, wraparoundMax);
	wraparoundMin = Vector3(1000.f, 1000.f, 1000.f);
	wraparoundMax = Vector3(1200.f, 1200.f, 1200.f);
	m_wraparound_1 = new WrapAround(wraparoundMin, wraparoundMax);

	m_quad_ccd_test = InitializePhysQuad(Vector3(1150.f, 1100.f, 1100.f), Vector3(0.f, 90.f, 0.f), Vector3(200.f, 200.f, 1.f), Rgba::WHITE, MOVE_STATIC, BODY_RIGID, CONTINUOUS);
	m_ball_ccd_test = InitializePhysSphere(Vector3(1050.f, 1100.f, 1100.f), Vector3::ZERO, Vector3(0.5f, 0.5f, 0.5f), Rgba::CYAN, MOVE_DYNAMIC, BODY_RIGID);			// this is for comparison without ccd
	Rigidbody3* rigid_s = static_cast<Rigidbody3*>(m_ball_ccd_test->GetEntity());
	rigid_s->SetLinearVelocity(Vector3(500.f, 0.f, 0.f));	
	rigid_s->SetAwake(true);
	rigid_s->SetCanSleep(false);
	rigid_s->SetFrozen(true);			// freeze at the start
	m_wraparound_1->m_gos.push_back(m_ball_ccd_test);
	m_ball_ccd_test_0 = InitializePhysSphere(Vector3(1050.f, 1050.f, 1050.f), Vector3::ZERO, Vector3(0.5f, 0.5f, 0.5f), Rgba::CYAN, MOVE_DYNAMIC, BODY_RIGID, CONTINUOUS);
	Rigidbody3* rigid_s_0 = static_cast<Rigidbody3*>(m_ball_ccd_test_0->GetEntity());
	rigid_s_0->SetLinearVelocity(Vector3(500.f, -1.f, 0.f));	
	rigid_s_0->SetAwake(true);
	rigid_s_0->SetCanSleep(false);
	rigid_s_0->SetFrozen(true);			// freeze at the start
	m_wraparound_1->m_gos.push_back(m_ball_ccd_test_0);
	m_inspection.push_back(Vector3(1100.f, 1100.f, 900.f));			// add a inspection point for this control group

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

	delete m_wraparound_0;
	m_wraparound_0 = nullptr;

	delete m_wraparound_1;
	m_wraparound_1 = nullptr;
}


Sphere* Physics3State::InitializePhysSphere(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme scheme)
{
	Sphere* s = new Sphere(pos, rot, scale, tint, "sphere_pcu", "default", moveStat, bid, false, COMPARE_LESS, CULLMODE_BACK, WIND_COUNTER_CLOCKWISE, scheme);
	s->m_physDriven = true;
	m_gameObjects.push_back(s);
	m_spheres.push_back(s);
	s->m_physEntity->SetGameobject(s);

	if (scheme == CONTINUOUS)
		m_ccd_spheres.push_back(s);

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
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme scheme)
{
	Quad* q = new Quad(pos, rot, scale, tint, "quad_pcu", "default", moveStat, bid, false, COMPARE_LESS, CULLMODE_FRONT, WIND_COUNTER_CLOCKWISE, CONTINUOUS);
	q->m_physDriven = true;
	m_gameObjects.push_back(q);
	m_quads.push_back(q);
	q->m_physEntity->SetGameobject(q);

	if (scheme == CONTINUOUS)
		m_ccd_planes.push_back(q);

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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		Vector3 pos = m_inspection[m_insepction_count];

		// set camera pos to this inspection position
		m_camera->GetTransform().SetLocalPosition(pos);

		int size = m_inspection.size();
		++m_insepction_count;
		m_insepction_count = (m_insepction_count % size);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE))
	{
		// shoot a ball from camera
		//ShootBallFromCamera();

		//WrapAroundTestBox();

		//WrapAroundTestBall();

		WrapAroundTestBallBox();
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
	// do not need to worry about continuous pairs because they will not use all_resolver
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

		// do not draw continuous pairs as the simulated time is not accurate
		if (!(contact.m_e1->IsContinuous() && contact.m_e2->IsContinuous()))
		{
			Vector3 point = contact.m_point;
			Vector3 end = point + contact.m_normal * contact.m_penetration;
			DebugRenderLine(0.1f, point, end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
		}
	}

	// draw a trail after spheres
	for (int i = 0; i < m_spheres.size(); ++i)
	{
		Sphere* sph = m_spheres[i];
		SphereRB3* rb = static_cast<SphereRB3*>(sph->m_physEntity);

		if (rb != nullptr)
		{
			const Vector3& start = sph->GetPhysicsCenter();
			const Vector3& vel_norm = rb->GetLinearVelocity().GetNormalized();
			const Vector3& end = start + vel_norm * 0.1f;
			DebugRenderLine(.1f, start, end, 10.f, Rgba::MEGENTA, Rgba::MEGENTA, DEBUG_RENDER_USE_DEPTH);
		}
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
	// at the start of the frame, take a snapshot of all continuous pairs
	// and record essential info for ccd test (see comments below)
	// we start with sphere and plane pairs
	for (std::vector<Sphere*>::size_type idx_sph = 0; idx_sph < m_ccd_spheres.size(); ++idx_sph)
	{
		// we get the first sphere
		Sphere* sph = m_ccd_spheres[idx_sph];
		SphereRB3* sph_rb = static_cast<SphereRB3*>(sph->m_physEntity);
		const float& radius = sph->GetRadius();
		const Vector3& this_center = sph->GetPhysicsCenter();

		if (sph_rb->IsFrozen())
			break;

		// vs (STATIC) plane
		for (std::vector<Quad*>::size_type idx_quad = 0; idx_quad < m_ccd_planes.size(); ++idx_quad)
		{
			TODO("Extend to entity3 as well");
			Quad* quad = m_ccd_planes[idx_quad];
			QuadRB3* quad_rb = static_cast<QuadRB3*>(quad->m_physEntity);
			const Plane& plane = quad_rb->GetPlanePrimitive();

			// we need to calculate a sc, which represents a signed distance to from sphere to plane before integration
			const float& sc = DistPointToPlaneSigned(this_center, plane);

			//const Vector3& linear_acc = sph_rb->GetLinearAcceleration();
			//const Vector3& ang_acc = sph_rb->m_inverseInertiaTensorWorld * sph_rb->m_torqueAcc;
			//const Vector3& linear_vel = sph_rb->GetLinearVelocity();
			//const Vector3& ang_vel = sph_rb->GetAngularVelocity();

			Vector3 next_center;
			Quaternion next_orient;
			sph_rb->Simulate(deltaTime, next_center, next_orient);

			// with the simulated new center, get the signed dist again
			const float& se = DistPointToPlaneSigned(next_center, plane); 

			// now we need to adjust simulation time for continuous objects if they are about to tunnel
			// remember the radius check, it is by nature based on minkowski method
			// the check is sc*se > 0, |sc > r| and |se > r|
			if (!(se * sc > 0 && abs(se) > radius && abs(sc) > radius))
			{
				// in this case we need motion clamp
				float t = (sc - radius) / (sc - se);
				float simulated_time = deltaTime * t;
				
				// now that we get this t, we want to iterate on it to make sure there would be a intersection using this t
				Vector3 predicted_center;
				Quaternion predicted_orient;
				sph_rb->Simulate(simulated_time, predicted_center, predicted_orient);

				float predicted_se = DistPointToPlaneSigned(predicted_center, plane);

				while (abs(predicted_se) >= radius)
				{
					t += 0.01f;
					simulated_time = deltaTime * t;
					sph_rb->Simulate(simulated_time, predicted_center, predicted_orient);
					predicted_se = DistPointToPlaneSigned(predicted_center, plane);
				}

				// at this time t is reusable since we know it clamps the motion to an extend that there will still be an intersection
				sph_rb->m_motionClampTime = t;
				sph_rb->m_motionClamp = true;
			}
		}

		// vs sphere
		// ...
	}

	// plane pairs (excluding spheres since that sort of pair has been processed)
	// ...

	////////////////////////////////////////////////////////////////////////////////////////////////
	// actually committing the integration

	// core of update
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
	////////////////////////////////////////////////////////////////////////////////////////////////

	// teleport for wraparound, meaning that it only updates objects in corner boundary cases
	//m_wraparound->Update();
	m_wraparound_0->Update();
	m_wraparound_1->Update();
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

					CollisionDetector::OBB3VsSphere3Single(obb, sph, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					OBB3 obb = brb->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3Single(obb, sph, m_allResolver->GetCollisionData());
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

					CollisionDetector::OBB3VsSphere3Single(obb, sph, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereRB3* se = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);

					Sphere3 sph = se->GetSpherePrimitive();
					OBB3 obb = brb->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3Coherent(obb, sph, m_coherentResolver->GetCollisionData());
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

	RenderModelSamples(renderer);				// sample points for model

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
	Vector3 pos = positions[m_wrapPosIterator_0];
	if (m_physBall == nullptr)
	{
		//m_physBall = InitializePhysSphere(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_s = static_cast<Rigidbody3*>(m_physBall->GetEntity());
		rigid_s->SetLinearVelocity(GetRandomVector3() * 20.f);
		rigid_s->SetAwake(true);
		rigid_s->SetCanSleep(true);
		m_wraparound_0->m_gos.push_back(m_physBall);
	}
	else
	{
		Sphere* s = InitializePhysSphere(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());
		rigid_s->SetLinearVelocity(GetRandomVector3() * 20.f);
		rigid_s->SetAwake(true);
		rigid_s->SetCanSleep(true);
		m_wraparound_0->m_gos.push_back(s);
	}
	m_wrapPosIterator_0 += 1;
	m_wrapPosIterator_0 %= 8;
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

void Physics3State::WrapAroundTestBallBox()
{
	Vector3 positions_0[8] = {Vector3(30.f, 120.f, 10.f), Vector3(30.f, 120.f, 15.f),
		Vector3(35.f, 120.f, 10.f), Vector3(35.f, 120.f, 15.f),
		Vector3(30.f, 125.f, 10.f), Vector3(30.f, 125.f, 15.f),
		Vector3(35.f, 125.f, 10.f), Vector3(35.f, 125.f, 15.f)};
	Vector3 pos_0 = positions_0[m_wrapPosIterator_0];

	if ((m_wrapPosIterator_0 % 2) == 0)
	{
		// even, spawn ball
		Sphere* s = InitializePhysSphere(pos_0, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
		Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());
		rigid_s->SetLinearVelocity(GetRandomVector3() * 5.f);
		rigid_s->SetAwake(true);
		rigid_s->SetCanSleep(true);
		m_wraparound_0->m_gos.push_back(s);
	}
	else
	{
		// odd, spawn box
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
