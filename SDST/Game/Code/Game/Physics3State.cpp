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
	m_rigidRegistry = new RigidForceRegistry();

	m_gravity = new GravityRigidForceGenerator(Vector3::GRAVITY / 2.f);

	m_wraparound_sphere_only = new WrapAround(Vector3(20.f, 300.f, -10.f), Vector3(40.f, 320.f, 10.f),
		Vector3(25.f, 305.f, -5.f), Vector3(35.f, 305.f, -5.f),
		Vector3(25.f, 305.f, 5.f), Vector3(35.f, 305.f, 5.f),
		Vector3(25.f, 315.f, -5.f), Vector3(35.f, 315.f, -5.f),
		Vector3(25.f, 315.f, 5.f), Vector3(35.f, 315.f, 5.f));
	//sph_holder = new Sphere(1.f, Vector3(30.f, 310.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::GREEN);
	////sph_holder->m_physDriven = true;
	//m_gameObjects.push_back(sph_holder);
	//m_wraparound_sphere_only->m_gos.push_back(sph_holder);

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);

	// inspection spot
	m_inspection.push_back(Vector3(30.f, 310.f, -30.f));
	m_inspection.push_back(Vector3(0.f, 0.f, -7.f));
	m_inspection.push_back(Vector3(-100.f, 50.f, -50.f));
	m_inspection.push_back(Vector3(0.f, 225, -30.f));
	m_inspection.push_back(Vector3(1100.f, 1100.f, 900.f));		
}

Physics3State::~Physics3State()
{
	m_spheres.clear();
	m_cubes.clear();
	m_quads.clear();
	m_points.clear();

	delete m_particleRegistry;
	m_particleRegistry = nullptr;

	delete m_rigidRegistry;
	m_rigidRegistry = nullptr;

	delete m_spring;
	m_spring = nullptr;

	delete m_anchorSpring;
	m_anchorSpring = nullptr;

	delete m_rigidAnchorSpring;
	m_rigidAnchorSpring = nullptr;

	delete m_wraparound_general;
	m_wraparound_general = nullptr;

	delete m_wraparound_verlet;
	m_wraparound_verlet = nullptr;

	delete m_wraparound_continuous;
	m_wraparound_continuous = nullptr;

	delete m_wraparound_box_only;
	m_wraparound_box_only = nullptr;

	delete m_wraparound_sphere_only;
	m_wraparound_sphere_only = nullptr;

	delete m_wraparound_bvh;
	m_wraparound_bvh = nullptr;
}


void Physics3State::PostConstruct()
{
	//m_wraparound_general->m_physState = this;
	m_wraparound_sphere_only->m_physState = this;
	//m_wraparound_box_only->m_physState = this;
	//m_wraparound_verlet->m_physState = this;
	//m_wraparound_continuous->m_physState = this;
	//m_wraparound_bvh->m_physState = this;
}

Sphere* Physics3State::InitializePhysSphere(const float& mass, const Vector3& pos, const Vector3& rot, const Vector3& scale,
	const Rgba& tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme scheme)
{
	Sphere* s = new Sphere(mass, pos, rot, scale, tint, "sphere_pcu", "default", moveStat, bid, false, COMPARE_LESS, CULLMODE_BACK, WIND_COUNTER_CLOCKWISE, scheme);
	//s->m_physDriven = true;
	m_gameObjects.push_back(s);
	m_spheres.push_back(s);

	s->m_physEntity->SetGameobject(s);
	s->m_physEntity->m_body_shape = SHAPE_SPHERE;

	if (scheme == CONTINUOUS)
		m_ccd_spheres.push_back(s);

	return s;
}

Cube* Physics3State::InitializePhysCube(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	// AABB3 - deprecated
	Cube* c = new Cube(pos, rot, scale, tint, "cube_pcu", "default", moveStat, bid);
	//c->m_physDriven = true;
	m_gameObjects.push_back(c);
	m_cubes.push_back(c);
	c->m_physEntity->SetGameobject(c);
	//c->m_physEntity->m_body_shape = SHAPE_BOX;

	return c;
}

Quad* Physics3State::InitializePhysQuad(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme scheme)
{
	Quad* q = new Quad(pos, rot, scale, tint, "quad_pcu", "default", moveStat, bid, false, COMPARE_LESS, CULLMODE_FRONT, WIND_COUNTER_CLOCKWISE, scheme);
	//q->m_physDriven = true;
	m_gameObjects.push_back(q);
	m_quads.push_back(q);

	q->m_physEntity->SetGameobject(q);
	q->m_physEntity->m_body_shape = SHAPE_PLANE;

	if (scheme == CONTINUOUS)
		m_ccd_planes.push_back(q);

	return q;
}

Box* Physics3State::InitializePhysBox(const Vector3& pos, const Vector3& rot, const Vector3& scale,
	const Rgba& tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme)
{
	Box* b = new Box(pos, rot, scale, tint, "cube_pcu", "default", moveStat, bid);
	//b->m_physDriven = true;
	m_gameObjects.push_back(b);
	m_boxes.push_back(b);

	b->m_physEntity->SetGameobject(b);
	b->m_physEntity->m_body_shape = SHAPE_BOX;

	return b;
}


Point* Physics3State::InitializePhysPoint(Vector3 pos, Vector3 rot, float size,
	Rgba tint, eMoveStatus moveStat, eBodyIdentity bid)
{
	// note: points are all entities and non-rigid
	Point* pt = new Point(pos, rot, size, tint, "point_pcu", "default", moveStat, bid);
	//pt->m_physDriven = true;
	m_gameObjects.push_back(pt);
	m_points.push_back(pt);

	pt->m_physEntity->SetGameobject(pt);
	pt->m_physEntity->m_body_shape = SHAPE_POINT;

	return pt;
}

Fireworks* Physics3State::SetupFireworks(float age, Vector3 pos, Vector3 inheritVel, Vector3 maxVel, Vector3 minVel, bool lastRound)
{
	Fireworks* fw = new Fireworks(pos);
	fw->Configure(age, inheritVel, maxVel, minVel, lastRound);
	m_gameObjects.push_back(fw);
	m_points.push_back(fw);

	fw->m_physEntity->SetGameobject(fw);

	m_fw_points.push_back(fw);

	return fw;
}

Ballistics* Physics3State::SetupBallistics(eBallisticsType type, Vector3 pos, bool frozen, Rgba color)
{
	// note: ballistics is point by nature, so non-rigid
	Ballistics* b = new Ballistics(type, pos, frozen, color);
	//b->m_physDriven = true;
	m_gameObjects.push_back(b);
	m_points.push_back(b);

	b->m_physEntity->SetGameobject(b);
	b->m_physEntity->SetNetForcePersistent(true);			// do NOT clear force every frame
	b->m_physEntity->m_body_shape = SHAPE_POINT;

	return b;
}

Spring* Physics3State::SetupSpring(Point* end1, Point* end2, float coef, float rl)
{
	Spring* sp = new Spring(end1, end2, coef, rl);

	// get the entities of points
	Entity3* e1 = end1->m_physEntity;
	Entity3* e2 = end2->m_physEntity;

	e1->m_constrained = true;
	e2->m_constrained = true;

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

	e1->m_constrained = true;
	e2->m_constrained = true;

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
	UpdateInput(deltaTime);				// update input
	//UpdateForceRegistry(deltaTime);		// update force registry
	UpdateGameobjects(deltaTime);		// update gameobjects
	UpdateContacts(deltaTime);
	//UpdateDebug(deltaTime);			
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_B))
		g_broadphase = !g_broadphase;
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

	// clear entities in wraparounds
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_F3))
	{

	}

	// fireworks cleanup - it is the only case where a GO's lifetime would expire
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_I))
	{
		m_fw_points.clear();
		
		// we clear expired fw now
		for (std::vector<Point*>::size_type idx = 0; idx < m_points.size(); ++idx)
		{
			if (m_points[idx] == nullptr)
			{
				std::vector<Point*>::iterator it_pt = m_points.begin() + idx;
				m_points.erase(it_pt);

				idx--;
			}
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0))
	{
		switch (m_genStep)
		{
		case HULL_GEN_FORM_CONFLICTS:
		{
			static int vert_count = 0;
			QHVert* vert = g_hull->GetVert(vert_count);
			bool removed = g_hull->AddConflictPointInitial(vert);

			// if there is nothing removed, we increment the index to go to next point
			// so that next time 0 is pressed, correct point is used
			if (!removed)
				vert_count++;
			// if there is something removed, the index will just match

			// if we have gone thru every point in global candidate list,
			// we are done with adding conflict points, so we should not come to this spot anymore
			if (vert_count == g_hull->GetVertNum())
			{
				//SwapHullStatusMesh("Forming eye");
				m_genStep = HULL_GEN_FORM_EYE;
			}
		}
			break;
		case HULL_GEN_FORM_EYE:
		{
			// get the farthest conflict point
			g_hull->m_eyePair = g_hull->GetFarthestConflictPair();
			QHFace* conflict_face = std::get<0>(g_hull->m_eyePair);
			QHVert* conflict_pt = std::get<1>(g_hull->m_eyePair);

			// initialization of the horizon generation step: we need to push the chosen conflict face as a start
			if (conflict_face != nullptr && conflict_pt != nullptr)
			{
				// i also want to change the color of eye point
				conflict_pt->ChangeColor(Rgba::PURPLE);

				// before iterating on horizon generation, initialize face and HE info here
				g_hull->m_start_he = conflict_face->m_entry;
				g_hull->m_current_he = g_hull->m_start_he;
				g_hull->m_otherFace = g_hull->m_current_he->m_twin->m_parentFace;

				g_hull->m_visitedFaces.push_back(conflict_face);
				g_hull->m_exploredFaces.push_back(conflict_face);

				g_hull->ChangeCurrentHalfEdgeMesh();

				//SwapHullStatusMesh("Horizon start and process");
				m_genStep = HULL_GEN_FORM_HORIZON_START;
			}
		}
			break;
		case HULL_GEN_FORM_HORIZON_START:
		{
			//QHFace* conflict_face = std::get<0>(g_hull->m_eyePair);
			QHVert* conflict_pt = std::get<1>(g_hull->m_eyePair);

			// to start with, we can assume the we have never visited any other faces
			bool visible = g_hull->PointOutBoundFace(conflict_pt->vert, *g_hull->m_otherFace);
			if (visible)
			{
				// step on a new face
				g_hull->m_exploredFaces.push_back(g_hull->m_otherFace);
				g_hull->m_visitedFaces.push_back(g_hull->m_otherFace);

				HalfEdge* expiring = g_hull->m_current_he;

				g_hull->ChangeCurrentHalfEdgeNewFace();
				g_hull->ChangeCurrentHalfEdgeMesh();			// verifies if the new HE has twin

				bool exp_has_twin = expiring->HasTwin();
				expiring->SwapMeshTwinGeneral(exp_has_twin);	// verifies if the outdated HE has twin

				g_hull->ChangeOtherFace();
			}
			else
			{
				// invisible
				HalfEdge* horizon = g_hull->m_current_he;

				g_hull->ChangeCurrentHalfEdgeOldFace();
				g_hull->ChangeCurrentHalfEdgeMesh();			// verifies if the new HE has twin
				
				// still want to verify expiring HE twin validity, 
				// but do not want to draw cyan/megenta in this case
				// Note that horizon IS expiring HE
				horizon->VerifyHorizonTwin();
				horizon->SwapMeshTwinHorizon();		// i got thru last line, i HAVE twin
				g_hull->AddHorizonInfo(horizon);

				g_hull->ChangeOtherFace();
			}

			m_genStep = HULL_GEN_FORM_HORIZON_PROECSS;
		}
			break;
		case HULL_GEN_FORM_HORIZON_PROECSS:
		{
			//QHFace* conflict_face = std::get<0>(g_hull->m_eyePair);
			QHVert* conflict_pt = std::get<1>(g_hull->m_eyePair);

			if (!g_hull->ReachStartHalfEdge())
			{
				bool visible = g_hull->PointOutBoundFace(conflict_pt->vert, *g_hull->m_otherFace);
				if (visible)
				{
					// if the face we have visited
					bool visited = g_hull->HasVisitedFace(g_hull->m_otherFace);
					if (visited)
					{
						// if it is the last face visited
						bool last_visited = g_hull->IsLastVisitedFace(g_hull->m_otherFace);
						if (last_visited)
						{
							g_hull->m_exploredFaces.pop_back();

							HalfEdge* expiring = g_hull->m_current_he;

							g_hull->ChangeCurrentHalfEdgeNewFace();
							g_hull->ChangeCurrentHalfEdgeMesh();		

							bool exp_has_twin = expiring->HasTwin();
							expiring->SwapMeshTwinGeneral(exp_has_twin);

							g_hull->ChangeOtherFace();
						}
						else
						{
							// this is not the face we came from, it is just a normal face we previously visited; skip it
							//HalfEdge* expiring = g_hull->m_current_he;

							g_hull->ChangeCurrentHalfEdgeOldFace();
							g_hull->ChangeCurrentHalfEdgeMesh();			// verifies if the new HE has twin

							g_hull->ChangeOtherFace();
						}
					}
					else
					{
						// this is a new face to visit
						g_hull->m_exploredFaces.push_back(g_hull->m_otherFace);
						g_hull->m_visitedFaces.push_back(g_hull->m_otherFace);

						HalfEdge* expiring = g_hull->m_current_he;

						g_hull->ChangeCurrentHalfEdgeNewFace();
						g_hull->ChangeCurrentHalfEdgeMesh();		

						bool exp_has_twin = expiring->HasTwin();
						expiring->SwapMeshTwinGeneral(exp_has_twin);

						g_hull->ChangeOtherFace();
					}
				}
				else 
				{
					// invisible
					HalfEdge* horizon = g_hull->m_current_he;

					g_hull->ChangeCurrentHalfEdgeOldFace();
					g_hull->ChangeCurrentHalfEdgeMesh();			// verifies if the new HE has twin

					// still want to verify expiring HE twin validity, 
					// but do not want to draw cyan/megenta in this case
					// Note that horizon IS expiring HE
					horizon->VerifyHorizonTwin();
					horizon->SwapMeshTwinHorizon();		// i got thru last line, i HAVE twin
					g_hull->AddHorizonInfo(horizon);

					g_hull->ChangeOtherFace();
				}
			}
			else
			{
				// go to next state
				// in this case we always know where current HE is (cyan or megenta)
				//SwapHullStatusMesh("Delete old faces");
				m_genStep = HULL_GEN_DELETE_OLD_FACES;
			}
		}
			break;
		case HULL_GEN_DELETE_OLD_FACES:
		{
			if (!g_hull->m_visitedFaces.empty())
			{
				QHVert* eye = std::get<1>(g_hull->m_eyePair);

				// remove from visited 
				QHFace* visited_frontier = g_hull->PeekVisitedFrontier();
				g_hull->RemoveVisitedFrontier();

				// remove from list of all faces
				std::vector<QHFace*>::iterator it = std::find(g_hull->m_faces.begin(), g_hull->m_faces.end(), visited_frontier);
				//bool inList = (it != g_hull->m_faces.end());
				g_hull->m_faces.erase(it);

				// delete this face
				// before releasing memory of faces, we need to get orphanage of this face
				for (QHVert* v : visited_frontier->conflicts)
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

				// delete HEs, if we can find it in horizon, we do not delete it
				HalfEdge* it_he = visited_frontier->m_entry;
				HalfEdge* prev = it_he->m_prev;
				HalfEdge* next = it_he->m_next;
				if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), it_he) == g_hull->m_horizon.end())
					// in other words, delete the entry if we cannot find it as part of horizon
					delete it_he;
				if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), prev) == g_hull->m_horizon.end())
					delete prev;
				if (std::find(g_hull->m_horizon.begin(), g_hull->m_horizon.end(), next) == g_hull->m_horizon.end())
					delete next;

				// only set anchor when deleting the first time
				if (g_hull->m_anchor == Vector3::INVALID)
				{
					// a good candidate is the centroid of the face
					Vector3 interior = visited_frontier->GetFaceCentroid();
					g_hull->m_anchor = interior;
				}

				delete visited_frontier;
			}
			else
			{
				// we have deleted some stuff, we want to verify HE relations
				for (QHFace* face : g_hull->m_faces)
				{
					face->VerifyHalfEdgeNext();
					face->VerifyHalfEdgeParent();
					face->VerifyHalfEdgeTwin();
				}

				g_hull->m_render_horizon = true;

				// conflict face is already deleted
				std::get<0>(g_hull->m_eyePair) = nullptr;	

				//SwapHullStatusMesh("Form new faces");
				m_genStep = HULL_GEN_FORM_NEW_FACES;
			}
		}
			break;
		case HULL_GEN_FORM_NEW_FACES:
		{
			if (!g_hull->m_horizon.empty())
			{
				QHVert* eye = std::get<1>(g_hull->m_eyePair);

				HalfEdge* he_restore = g_hull->PeekHorizonFrontier();
				g_hull->RemoveHorizonFrontier();

				std::tuple<Vector3, Vector3, HalfEdge*> he_data = g_hull->m_horizon_infos.front();
				g_hull->m_horizon_infos.pop_front();

				// build new face and verify it 
				//const Vector3& eyePos = eye->vert;
				QHFace* new_face = new QHFace(he_restore, std::get<1>(he_data), eye->vert);
				g_hull->CreateFaceMesh(*new_face);			// face mesh
				g_hull->GenerateOutboundNorm(g_hull->m_anchor, *new_face);		// normal
				g_hull->CreateFaceNormalMesh(*new_face);		// normal mesh and color
				new_face->SetParentHalfEdge();

				new_face->VerifyHalfEdgeNext();
				new_face->VerifyHalfEdgeParent();

				g_hull->AddFace(new_face);
				g_hull->AddNewFace(new_face);
			}
			else
			{
				// build twin relation
				for (std::vector<QHFace*>::size_type i1 = 0; i1 < g_hull->m_newFaces.size(); ++i1)
				{
					QHFace* subject = g_hull->m_newFaces[i1];

					for (std::vector<QHFace*>::size_type i2 = 0; i2 <g_hull->m_newFaces.size(); ++i2)
					{
						QHFace* object = g_hull->m_newFaces[i2];
						if (subject != object)
						{
							// for each new face, there is a possibility that it shares an edge with another new face
							// once we find that shared edge, the half edge corresponding to it in that other face is just the twin
							// of the half edge corresponding to this shared edge in this current face we are inspecting
							subject->UpdateSharedEdge(object);		// set twins along the way
						}
					}
				}

				// verifications
				for (QHFace* face : g_hull->m_faces)
				{
					face->VerifyHalfEdgeNext();
					face->VerifyHalfEdgeParent();
					face->VerifyHalfEdgeTwin();
				}

				//SwapHullStatusMesh("Assign orphans");
				m_genStep = HULL_GEN_ASSIGN_ORPHANS;
			}
		}
			break;
		case HULL_GEN_ASSIGN_ORPHANS:
		{
			if (!g_hull->m_orphans.empty())
			{
				QHVert* orphan = g_hull->m_orphans.front();
				g_hull->m_orphans.pop_front();

				g_hull->AddConflictPointGeneral(orphan, g_hull->m_newFaces);
			}
			else
			{
				g_hull->m_newFaces.clear();

				//SwapHullStatusMesh("Correct Topo errors");
				m_genStep = HULL_GEN_TOPO_ERRORS;
			}
		}
			break;
		case HULL_GEN_TOPO_ERRORS:
		{
			// verify and adjust for topological errors
			//SwapHullStatusMesh("Finish up and reset");
			m_genStep = HULL_GEN_FINISH_RESET;
		}
			break;
		case HULL_GEN_FINISH_RESET:
		{
			QHVert* eye = std::get<1>(g_hull->m_eyePair);
			g_hull->RemovePointGlobal(eye->vert);

			if (!g_hull->m_exploredFaces.empty())
				g_hull->m_exploredFaces.clear();
			
			g_hull->m_start_he = nullptr;
			g_hull->m_current_he = nullptr;
			g_hull->m_otherFace = nullptr;

			g_hull->m_anchor = Vector3::INVALID;

			g_hull->m_render_horizon = false;

			// verify again
			for (QHFace* face : g_hull->m_faces)
			{
				face->VerifyHalfEdgeNext();
				face->VerifyHalfEdgeParent();
				face->VerifyHalfEdgeTwin();
			}

			if (!g_hull->m_conflict_verts.empty())
			{
				// conflict list should be adjusted correctly already; back to the step where we generate eye
				//SwapHullStatusMesh("Forming eye");
				m_genStep = HULL_GEN_FORM_EYE;
			}
			else
			{
				//SwapHullStatusMesh("Hull complete");
				m_genStep = HULL_GEN_COMPLETE;
			}
		}
			break;
		case HULL_GEN_COMPLETE:
		{
			// hull complete, do nothing
		}
			break;
		default:
			break;
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		if (!m_inspection.empty())
		{
			Vector3 pos = m_inspection[m_insepction_count];

			// set camera pos to this inspection position
			m_camera->GetTransform().SetLocalPosition(pos);
			m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);

			int size = (int)m_inspection.size();
			++m_insepction_count;
			m_insepction_count = (m_insepction_count % size);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE))
		WrapAroundTestGeneral(true, true);
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_1))
		WrapAroundTestBox(true, true);			// to general wpa
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_2))
		WrapAroundTestSphere(true, true);		// to general wpa
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_3))
		WrapAroundTestSphere(m_wraparound_sphere_only, false, false);
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_4))
		WrapAroundTestBox(m_wraparound_box_only, false, false);
	// bvh wraparound
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_5))
		WrapAroundTestSphere(m_wraparound_bvh, false, false);
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_6))
		WrapAroundTestSphere(m_wraparound_sphere_only, false, false, true, Vector3(31.8f, 315.f, 0.f), Vector3::ZERO, Vector3::ONE);
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_8))
		WrapAroundTestBox(m_wraparound_box_only, false, false, true, Vector3(60.8f, 315.f, 0.f), Vector3(10.f), Vector3::ONE); // pos - (60.8f, 315.f, 0.f), rot - (-20.f, 0.f, -20.f) gives visual appealing result

	// slow
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_0))
	{
		//// slow down gameobjects in the specified wraparound
		//for (GameObject* go : m_wraparound_general->m_gos)
		//	go->m_physEntity->m_slowed = 0.1f;

		//for (GameObject* go : m_wraparound_sphere_only->m_gos)
		//	go->m_physEntity->m_slowed = 0.1f;

		//for (GameObject* go : m_wraparound_box_only->m_gos)
		//	go->m_physEntity->m_slowed = 0.1f;
	}
	else
	{
		//for (GameObject* go : m_wraparound_general->m_gos)
		//	go->m_physEntity->m_slowed = 1.f; 

		//for (GameObject* go : m_wraparound_sphere_only->m_gos)
		//	go->m_physEntity->m_slowed = 1.f;

		//for (GameObject* go : m_wraparound_box_only->m_gos)
		//	go->m_physEntity->m_slowed = 1.f;
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
	//UpdateFireworksStatus();

	//UpdateHulls(deltaTime);

	UpdateGameobjectsCore(deltaTime);		// update GO core
}

void Physics3State::UpdateDebug(float deltaTime)
{
	UpdateDebugCore(deltaTime);

	UpdateDebugDraw(deltaTime);
}

void Physics3State::UpdateDebugDraw(float deltaTime)
{
	DebugRenderUpdate(deltaTime);

	// debug draw for all-at-once resolver
	// do not need to worry about continuous pairs because they will not use all_resolver
	for (int i = 0; i < m_allResolver->GetCollisionData()->m_contacts.size(); ++i)
	{
		Contact3& contact = m_allResolver->GetCollisionData()->m_contacts[i];

		Vector3 point = contact.m_point;
		Vector3 end = point + contact.m_normal * contact.m_penetration;
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

	// draw a trail after continuous spheres
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

void Physics3State::UpdateDebugCore(float)
{
	const Vector3& euler = sph_holder->m_renderable->m_transform.GetLocalRotation();

	DebuggerPrintf("sph holder euler: %f, %f, %f\n", euler.x, euler.y, euler.z);
}

void Physics3State::UpdateHulls(float)
{
	// update any relevant hulls
	m_qh->UpdateHull();
}

void Physics3State::UpdateWrapArounds()
{
	// WRAPAROUND UPDATE
	//m_wraparound_general->Update();
	m_wraparound_sphere_only->Update();
	//m_wraparound_box_only->Update();
	//m_wraparound_verlet->Update();
	//m_wraparound_continuous->Update();
	//m_wraparound_bvh->Update();re
}

void Physics3State::UpdateFireworksStatus()
{
	// start a new fw seed whenever the structure is empty
	if (m_fw_points.empty())
		SetupFireworks(5.f, Vector3(25.f, 230.f, -5.f), Vector3::ZERO, Vector3(0.f, 4.f, 0.f), Vector3(0.f, 4.f, 0.f), false);
}

void Physics3State::UpdateUI()
{
	//// update UI
	//if (m_bp_stat != nullptr)
	//{
	//	delete m_bp_stat;
	//	m_bp_stat = nullptr;
	//}

	//BitmapFont* font = theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	//std::string bp_status = g_broadphase ? "On" : "Off";
	//std::string bp_title = "Broadphase status: ";
	//float txtHeight = window_height / 50.f;
	//Vector2 titleMin = Vector2(-window_width / 2.f, window_height / 2.f - txtHeight);
	//float bp_title_width = bp_title.size() * (.5f * txtHeight);
	//Vector2 bp_status_min = titleMin + Vector2(bp_title_width, 0.f);
	//m_bp_stat = Mesh::CreateTextImmediate(Rgba::WHITE, bp_status_min, font, txtHeight, .5f, bp_status, VERT_PCU);

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

	std::string time_ui = Stringf("Time: %f", TimingData::GetTimeSeconds());
	m_time_ui = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, time_ui, VERT_PCU);
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
	// continuous clamp time preparation (if there is some collision among continuous objects)
	//UpdateGameobjectContinuous(deltaTime);re

	UpdateGameobjectsDynamics(deltaTime);

	UpdateWrapArounds();
}

void Physics3State::UpdateGameobjectContinuous(float deltaTime)
{
	//// at the start of the frame, take a snapshot of all continuous pairs
	//// and record essential info for ccd test (see comments below)
	//// we start with sphere and plane pairs
	//for (std::vector<Sphere*>::size_type idx_sph = 0; idx_sph < m_ccd_spheres.size(); ++idx_sph)
	//{
	//	// we get the first sphere
	//	Sphere* sph = m_ccd_spheres[idx_sph];
	//	SphereRB3* sph_rb = static_cast<SphereRB3*>(sph->m_physEntity);
	//	const float& radius = sph->GetRadius();
	//	const Vector3& this_center = sph->GetPhysicsCenter();

	//	if (sph_rb->IsFrozen())
	//		break;

	//	// vs (STATIC) plane
	//	for (std::vector<Quad*>::size_type idx_quad = 0; idx_quad < m_ccd_planes.size(); ++idx_quad)
	//	{
	//		TODO("Extend to entity3 as well");
	//		Quad* quad = m_ccd_planes[idx_quad];
	//		QuadRB3* quad_rb = static_cast<QuadRB3*>(quad->m_physEntity);
	//		const Plane& plane = quad_rb->GetPlanePrimitive();

	//		// we need to calculate a sc, which represents a signed distance to from sphere to plane before integration
	//		const float& sc = DistPointToPlaneSigned(this_center, plane);

	//		Vector3 next_center;
	//		Quaternion next_orient;
	//		sph_rb->Simulate(deltaTime, next_center, next_orient);

	//		// with the simulated new center, get the signed dist again
	//		const float& se = DistPointToPlaneSigned(next_center, plane); 

	//		// now we need to adjust simulation time for continuous objects if they are about to tunnel
	//		// remember the radius check, it is by nature based on minkowski method
	//		// the check is sc*se > 0, |sc > r| and |se > r|
	//		if (!(se * sc > 0 && abs(se) > radius && abs(sc) > radius))
	//		{
	//			// in this case we need motion clamp
	//			float t = (sc - radius) / (sc - se);
	//			float simulated_time = deltaTime * t;

	//			// now that we get this t, we want to iterate on it to make sure there would be a intersection using this t
	//			Vector3 predicted_center;
	//			Quaternion predicted_orient;
	//			sph_rb->Simulate(simulated_time, predicted_center, predicted_orient);

	//			float predicted_se = DistPointToPlaneSigned(predicted_center, plane);

	//			while (abs(predicted_se) >= radius)
	//			{
	//				t += 0.01f;
	//				simulated_time = deltaTime * t;
	//				sph_rb->Simulate(simulated_time, predicted_center, predicted_orient);
	//				predicted_se = DistPointToPlaneSigned(predicted_center, plane);
	//			}

	//			// at this time t is reusable since we know it clamps the motion to an extend that there will still be an intersection
	//			sph_rb->m_motionClampTime = t;
	//			sph_rb->m_motionClamp = true;
	//		}
	//	}

	//	// vs sphere
	//	// ...
	//}

	for (std::vector<Sphere*>::size_type idx_sph = 0; idx_sph < m_ccd_spheres.size(); ++idx_sph)
	{
		Sphere* sph = m_ccd_spheres[idx_sph];
		SphereRB3* sph_rb = static_cast<SphereRB3*>(sph->m_physEntity);
		const Sphere3& sph_primitive = sph_rb->GetSpherePrimitive();

		for (std::vector<Quad*>::size_type idx_quad = 0; idx_quad < m_ccd_planes.size(); ++idx_quad)
		{
			Quad* quad = m_ccd_planes[idx_quad];
			QuadRB3* quad_rb = static_cast<QuadRB3*>(quad->m_physEntity);
			const Plane& pl_primitive = quad_rb->GetPlanePrimitive();

			float t;
			Vector3 hit;
			Vector3 rel_v = sph_rb->GetLinearVelocity() - quad_rb->GetLinearVelocity();
			uint res = CollisionDetector::Sphere3VsPlane3Continuous(sph_primitive, pl_primitive, rel_v, t, hit);

			if (res != 0)
			{
				// could be overlapping
				// t cannot < 0
				if (t == 0.f)
				{
					// sphere already overlapping the plane, stop it
					sph_rb->m_frozen = true;
					quad_rb->m_frozen = true;
				}
				else if (t > 0.f && t < deltaTime)
				{
					// sphere will overlap this frame, set position to hit and stop it
					sph->UpdateWithSetPos(hit);
					sph_rb->m_frozen = true;
					quad_rb->m_frozen = true;
				}
				else
				{
					// sphere will overlap next frame, deal with it later
				}
			}
			// otherwise they do not have a chance to overlap
		}
	}
}

void Physics3State::UpdateGameobjectsDynamics(float deltaTime)
{
	// core of update
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
	{
		m_gameObjects[idx]->Update(deltaTime);
		//if (m_gameObjects[idx]->m_dead)
		//{
		//	GameObject* to_be_deleted = m_gameObjects[idx];

		//	std::vector<GameObject*>::iterator it = m_gameObjects.begin() + idx;
		//	m_gameObjects.erase(it);

		//	idx--;

		//	delete to_be_deleted;
		//	to_be_deleted = nullptr;
		//}
	}
}

void Physics3State::UpdateContacts(float deltaTime)
{
	UpdateContactGeneration();				// update contact generation

	UpdateContactResolution(deltaTime);		// contact resolution
}

void Physics3State::UpdateContactGeneration()
{
	//UpdateResolverEnd();

	UpdateContactGenerationCore();
}

void Physics3State::UpdateContactGenerationCore()
{
	if (!g_broadphase)
		UpdateContactGenerationOrdinary();
	//else
	//	UpdateContactGenerationBVH();
}

void Physics3State::UpdateContactGenerationOrdinary()
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

					const Sphere3& sph1 = se1->GetSpherePrimitive();
					const Sphere3& sph2 = se2->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Single(sph1, sph2, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(s1->m_physEntity);
					SphereRB3* srb = dynamic_cast<SphereRB3*>(s2->m_physEntity);

					const Sphere3& sph1 = se->GetSpherePrimitive();
					const Sphere3& sph2 = srb->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Single(sph1, sph2, m_allResolver->GetCollisionData());
				}
			}
			else
			{
				if (s2->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(s1->m_physEntity);
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(s2->m_physEntity);

					const Sphere3& sph1 = srb->GetSpherePrimitive();
					const Sphere3& sph2 = se->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Single(sph1, sph2, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereRB3* srb1 = dynamic_cast<SphereRB3*>(s1->m_physEntity);
					SphereRB3* srb2 = dynamic_cast<SphereRB3*>(s2->m_physEntity);

					const Sphere3& sph1 = srb1->GetSpherePrimitive();
					const Sphere3& sph2 = srb2->GetSpherePrimitive();

					CollisionDetector::Sphere3VsSphere3Coherent(sph1, sph2, m_coherentResolver->GetCollisionData());
				}
			}
		}

		// sphere vs plane
		for (uint idx2 = 0; idx2 < m_quads.size(); ++idx2)
		{
			Sphere* sphere = m_spheres[idx1];
			Quad* quad = m_quads[idx2];

			if (sphere->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
			{
				if (quad->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					QuadEntity3* qe = dynamic_cast<QuadEntity3*>(quad->m_physEntity);

					const Sphere3& sph = se->GetSpherePrimitive();
					const Plane& pl = qe->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Single(sph, pl, m_allResolver->GetCollisionData());
				}
				else 
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					QuadRB3* qrb = dynamic_cast<QuadRB3*>(quad->m_physEntity);

					const Sphere3& sph = se->GetSpherePrimitive();
					const Plane& pl = qrb->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Single(sph, pl, m_allResolver->GetCollisionData());
				}
			}
			else 
			{
				if (quad->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					QuadEntity3* qe = dynamic_cast<QuadEntity3*>(quad->m_physEntity);

					const Sphere3& sph = srb->GetSpherePrimitive();
					const Plane& pl = qe->GetPlanePrimitive();

					CollisionDetector::Sphere3VsPlane3Single(sph, pl, m_allResolver->GetCollisionData());
				}
				else 
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					QuadRB3* qrb = dynamic_cast<QuadRB3*>(quad->m_physEntity);

					const Sphere3& sph = srb->GetSpherePrimitive();
					const Plane& pl = qrb->GetPlanePrimitive();

					// if for any reason objects are frozen, we do not bother to generate contacts to resolve with
					if (srb->m_frozen && qrb->m_frozen)
						continue;		// to the next plane

					CollisionDetector::Sphere3VsPlane3Coherent(sph, pl, m_coherentResolver->GetCollisionData());
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

					const Sphere3& sph = se->GetSpherePrimitive();
					const OBB3& obb = be->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3Single(obb, sph, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereEntity3* se = dynamic_cast<SphereEntity3*>(sphere->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);

					const Sphere3& sph = se->GetSpherePrimitive();
					const OBB3& obb = brb->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3Single(obb, sph, m_allResolver->GetCollisionData());
				}
			}
			else 
			{
				if (box->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					SphereRB3* srb = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(box->m_physEntity);

					const Sphere3& sph = srb->GetSpherePrimitive();
					const OBB3& obb = be->GetBoxPrimitive();

					CollisionDetector::OBB3VsSphere3Single(obb, sph, m_allResolver->GetCollisionData());
				}
				else
				{
					SphereRB3* se = dynamic_cast<SphereRB3*>(sphere->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(box->m_physEntity);

					const Sphere3& sph = se->GetSpherePrimitive();
					const OBB3& obb = brb->GetBoxPrimitive();

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

					const OBB3& obb1 = be1->GetBoxPrimitive();
					const OBB3& obb2 = be2->GetBoxPrimitive();

					CollisionDetector::OBB3VsOBB3Single(obb1, obb2, m_allResolver->GetCollisionData());
				}
				else
				{
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(b1->m_physEntity);
					BoxRB3* brb = dynamic_cast<BoxRB3*>(b2->m_physEntity);

					const OBB3& obb1 = be->GetBoxPrimitive();
					const OBB3& obb2 = brb->GetBoxPrimitive();

					CollisionDetector::OBB3VsOBB3Single(obb1, obb2, m_allResolver->GetCollisionData());
				}
			}
			else
			{
				if (b2->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					BoxRB3* brb = dynamic_cast<BoxRB3*>(b1->m_physEntity);
					BoxEntity3* be = dynamic_cast<BoxEntity3*>(b2->m_physEntity);

					const OBB3& obb1 = brb->GetBoxPrimitive();
					const OBB3& obb2 = be->GetBoxPrimitive();

					CollisionDetector::OBB3VsOBB3Single(obb1, obb2, m_allResolver->GetCollisionData());
				}
				else
				{
					BoxRB3* brb1 = dynamic_cast<BoxRB3*>(b1->m_physEntity);
					BoxRB3* brb2 = dynamic_cast<BoxRB3*>(b2->m_physEntity);

					const OBB3& obb1 = brb1->GetBoxPrimitive();
					const OBB3& obb2 = brb2->GetBoxPrimitive();
					
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
					BoxEntity3* be = static_cast<BoxEntity3*>(box->m_physEntity);
					QuadEntity3* qe = static_cast<QuadEntity3*>(plane->m_physEntity);

					const OBB3& obb = be->GetBoxPrimitive();
					const Plane& pl = qe->GetPlanePrimitive();

					CollisionDetector::OBB3VsPlane3Single(obb, pl, m_allResolver->GetCollisionData());
				}
				else 
				{
					BoxEntity3* be = static_cast<BoxEntity3*>(box->m_physEntity);
					QuadRB3* rbq = static_cast<QuadRB3*>(plane->m_physEntity);

					const OBB3& obb = be->GetBoxPrimitive();
					const Plane& pl = rbq->GetPlanePrimitive();

					CollisionDetector::OBB3VsPlane3Single(obb, pl, m_allResolver->GetCollisionData());
				}
			}
			else
			{
				if (plane->m_physEntity->GetEntityBodyID() == BODY_PARTICLE)
				{
					BoxRB3* brb = static_cast<BoxRB3*>(box->m_physEntity);
					QuadEntity3* qe = static_cast<QuadEntity3*>(plane->m_physEntity);

					const OBB3& obb = brb->GetBoxPrimitive();
					const Plane& pl = qe->GetPlanePrimitive();

					CollisionDetector::OBB3VsPlane3Single(obb, pl, m_allResolver->GetCollisionData());
				}
				else 
				{
					BoxRB3* brb = static_cast<BoxRB3*>(box->m_physEntity);
					QuadRB3* rbq = static_cast<QuadRB3*>(plane->m_physEntity);

					const OBB3& obb = brb->GetBoxPrimitive();
					const Plane& pl = rbq->GetPlanePrimitive();

					CollisionDetector::OBB3VsPlane3Coherent(obb, pl, m_coherentResolver->GetCollisionData());
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
}


void Physics3State::UpdateContactGenerationBVH()
{
	// again, we assume here only wraparound_bvh uses bvh
	m_wraparound_bvh->UpdateBVHContactGeneration();
}

void Physics3State::UpdateContactResolution(float)
{
	//m_iterResolver->ResolveContacts(deltaTime);
	//m_allResolver->ResolveContacts(deltaTime);
	//m_coherentResolver->ResolveContacts(deltaTime);
}

void Physics3State::UpdateResolverEnd()
{
	m_allResolver->ClearRecords();
	m_coherentResolver->ClearRecords();

	// if a wraparound uses bvh, clear its bvhContact record
	// here we assume only wraparound_bvh uses bvh, for convenience
	// just to be very safe, clear bvh records every frame
	m_wraparound_bvh->ClearBVHRecords();
}

void Physics3State::Render(Renderer* renderer)
{
	renderer->SetCamera(m_UICamera);
	renderer->ClearScreen(Rgba::BLACK);

	RenderUI(renderer);

	renderer->SetCamera(m_camera);

	//RenderHulls(renderer);

	//RenderAssimpModels(renderer);

	RenderGameobjects(renderer);
	
	//RenderForwardPath(renderer);

	RenderWrapArounds(renderer);
}

void Physics3State::RenderGameobjects(Renderer* renderer)
{
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
	{
		if (!m_gameObjects[idx]->m_isInForwardPath)
			m_gameObjects[idx]->Render(renderer);
	}
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

void Physics3State::RenderHulls(Renderer* renderer)
{
	// render all qhs
	g_hull->RenderHull(renderer);
	m_qh->RenderHull(renderer);
}

void Physics3State::RenderWrapArounds(Renderer* renderer)
{
	//m_wraparound_general->Render(renderer);
	m_wraparound_sphere_only->Render(renderer);
	//m_wraparound_box_only->Render(renderer);
	//m_wraparound_verlet->Render(renderer);
	//m_wraparound_continuous->Render(renderer);
	//m_wraparound_bvh->Render(renderer);
}

void Physics3State::RenderForwardPath(Renderer*)
{
	m_forwardPath->RenderScene(m_sceneGraph);
}

void Physics3State::RenderAssimpModels(Renderer* renderer)
{
	// render all assimp-imported models
	// make sure the model is not drifted
	renderer->m_objectData.model = Matrix44::IDENTITY;
	renderer->DrawModel(m_assimp_0);
	RenderModelSamples(renderer);				// sample points for model
}

void Physics3State::RenderUI(Renderer*)
{
	//DrawTextCut(m_bp_title);
	//DrawTextCut(m_bp_stat);

	DrawTextCut(m_time_ui);
}

static int wraparound_toggle = 0;
void Physics3State::WrapAroundTestGeneral(bool give_ang_vel, bool register_g)
{
	if ((wraparound_toggle % 2) == 0)
		WrapAroundTestSphere(give_ang_vel, register_g);
	else
		WrapAroundTestBox(give_ang_vel, register_g);

	wraparound_toggle++;
}


void Physics3State::WrapAroundTestSphere(bool give_ang_vel, bool register_g)
{
	Vector3 pos = m_wraparound_general->m_positions[m_wraparound_general->m_pos_idx];

	// even, spawn ball
	Sphere* s = InitializePhysSphere(1.f, pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());
	if (register_g)
		m_rigidRegistry->Register(rigid_s, m_gravity);
	rigid_s->SetLinearVelocity(GetRandomVector3() * 5.f);
	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rigid_s->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}
	rigid_s->SetAwake(true);
	rigid_s->SetCanSleep(true);
	m_wraparound_general->m_gos.push_back(s);

	m_wraparound_general->m_pos_idx += 1;
	m_wraparound_general->m_pos_idx %= 8;
}

void Physics3State::WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, bool register_g)
{
	Vector3 pos = wpa->m_positions[wpa->m_pos_idx];

	// even, spawn ball
	Sphere* s = InitializePhysSphere(1.f, pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());
	if (register_g)
		m_rigidRegistry->Register(rigid_s, m_gravity);
	rigid_s->SetLinearVelocity(GetRandomVector3() * 5.f);
	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rigid_s->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}
	rigid_s->SetAwake(true);
	rigid_s->SetCanSleep(true);
	wpa->m_gos.push_back(s);

	wpa->m_pos_idx += 1;
	wpa->m_pos_idx %= 8;
}

void Physics3State::WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale)
{
	Sphere* s = InitializePhysSphere(1.f, position, rot, scale, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_s = static_cast<Rigidbody3*>(s->GetEntity());

	if(register_g)
		m_rigidRegistry->Register(rigid_s, m_gravity);
	
	if (give_lin_vel)
		rigid_s->SetLinearVelocity(GetRandomVector3() * 5.f);

	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rigid_s->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}

	rigid_s->SetAwake(true);
	rigid_s->SetCanSleep(true);
	wpa->m_gos.push_back(s);
}

void Physics3State::WrapAroundTestBox(bool give_ang_vel, bool register_g)
{
	Vector3 pos = m_wraparound_general->m_positions[m_wraparound_general->m_pos_idx];

	// spawn box
	Box* b = InitializePhysBox(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_b = static_cast<Rigidbody3*>(b->GetEntity());
	if (register_g)
		m_rigidRegistry->Register(rigid_b, m_gravity);
	rigid_b->SetLinearVelocity(GetRandomVector3() * 5.f);
	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rigid_b->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}
	rigid_b->SetAwake(true);
	rigid_b->SetCanSleep(true);
	m_wraparound_general->m_gos.push_back(b);

	m_wraparound_general->m_pos_idx += 1;
	m_wraparound_general->m_pos_idx %= 8;
}

void Physics3State::WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool register_g)
{
	Vector3 pos = wpa->m_positions[wpa->m_pos_idx];

	// spawn box
	Box* b = InitializePhysBox(pos, Vector3::ZERO, Vector3::ONE, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_b = static_cast<Rigidbody3*>(b->GetEntity());
	if (register_g)
		m_rigidRegistry->Register(rigid_b, m_gravity);
	rigid_b->SetLinearVelocity(GetRandomVector3() * 5.f);
	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rigid_b->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}
	rigid_b->SetAwake(true);
	rigid_b->SetCanSleep(true);
	wpa->m_gos.push_back(b);

	wpa->m_pos_idx += 1;
	wpa->m_pos_idx %= 8;
}

void Physics3State::WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale)
{
	Box* b = InitializePhysBox(position, rot, scale, Rgba::RED, MOVE_DYNAMIC, BODY_RIGID);
	Rigidbody3* rigid_b = static_cast<Rigidbody3*>(b->GetEntity());

	if(register_g)
		m_rigidRegistry->Register(rigid_b, m_gravity);

	if (give_lin_vel)
		rigid_b->SetLinearVelocity(GetRandomVector3() * 5.f);

	if (give_ang_vel)
	{
		float ang_v_x = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_y = GetRandomFloatInRange(-5.f, 5.f);
		float ang_v_z = GetRandomFloatInRange(-5.f, 5.f);
		rigid_b->SetAngularVelocity(Vector3(ang_v_x, ang_v_y, ang_v_z));
	}

	rigid_b->SetAwake(true);
	rigid_b->SetCanSleep(true);
	wpa->m_gos.push_back(b);
}
