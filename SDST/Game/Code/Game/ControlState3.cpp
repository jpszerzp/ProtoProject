#include "Game/ControlState3.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Physics/3D/RF/CollisionDetector.hpp"
#include "Engine/Physics/3D/GJK3.hpp"
#include "Engine/Physics/3D/EPA3.hpp"
#include "Engine/Physics/3D/GJK3Simplex.hpp"

static QuickHull* gjk_hull = nullptr;
static Vector3 net_disp;

// status
static eGJKStatus gjk_stat = GJK_FIND_SUPP_INITIAL;
static float gjk_closest_dist;

// support point
static Vector3 gjk_supp;
static Mesh* gjk_supp_mesh;

// support direction
static Line3 gjk_supp_dir;
static Mesh* gjk_supp_dir_mesh;

// gjk simplex
static std::set<Vector3> gjk_simplex;
static eGJKSimplex gjk_simplex_stat= GJK_SIMPLEX_NONE;
static Mesh* gjk_simplex_mesh =nullptr;

// gjk normal base
static Mesh* gjk_normal_base_mesh = nullptr;
static Vector3 gjk_normal_base;
static Vector3 gjk_last_normal_base;

// epa stat
static eEPAStat epa_stat = EPA_DELETE_GJK_REF;

// epa helper
static Mesh* epa_face_centroid;
static float epa_threshold = 1.f;	// hardcoded threshold, could be adjusted
static float epa_close_dist;

// epa simplex
static sEPASimplex* epa_simplex;
static sEPAFace* epa_close_face;

// epa support data
static Mesh* epa_support_anchor;
static Mesh* epa_support_line;
static Mesh* epa_support_pt;
static Vector3 epa_support_pt_pos;
static Vector3 epa_last_support_pt_pos = Vector3::ZERO;		// might be wrong in the corner case where the origin lies ON one of the verts of hull
static Vector3 epa_support_anchor_pos;

ControlState3::ControlState3()
{
	Renderer* renderer = Renderer::GetInstance();
	m_sceneGraph = new RenderSceneGraph();
	m_forwardPath = new ForwardRenderPath();
	m_forwardPath->m_renderer = renderer;

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	m_cameraInitialPos = Vector3(0.f, 5.f, -7.f);
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

	// origin
	m_origin = Mesh::CreatePointImmediate(VERT_PCU, Vector3::ZERO, Rgba::CYAN);

	// collision keep
	m_keep.m_collision_head = m_storage;

	// convex
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

	m_convex_0 = new CollisionConvexObject(*cHull, "wireframe", "Data/Images/white.png");
	m_convex_1 = new CollisionConvexObject(*cHull, "wireframe", "Data/Images/white.png");

	const float& mass_0 = m_convex_0->GetInitialMass();
	const Vector3& true_center_0 = Vector3(0.f, 200.f, 0.f);			 // com + (wrap_center - com); based on ORIGIN
	CollisionRigidBody* convex_rb_0 = new CollisionRigidBody(mass_0, true_center_0, Vector3::ZERO);
	convex_rb_0->SetAwake(true);
	convex_rb_0->SetSleepable(false);
	m_convex_0->AttachToRigidBody(convex_rb_0);

	const float& mass_1 = m_convex_1->GetInitialMass();
	const Vector3& true_center_1 = Vector3(0.f, 210.f, 0.f);
	CollisionRigidBody* convex_rb_1 = new CollisionRigidBody(mass_1, true_center_1, Vector3::ZERO);
	convex_rb_1->SetAwake(true);
	convex_rb_1->SetSleepable(false);
	m_convex_1->AttachToRigidBody(convex_rb_1);

	// planes
	m_plane_0 = new CollisionPlane(Vector2(20.f), "quad_pcu_20", Vector3(0.f, 1.f, 0.f), 200.f, "wireframe", "Data/Images/white.png");
	m_plane_1 = new CollisionPlane(Vector2(20.f), "quad_pcu_20", Vector3(0.f, 1.f, 0.f), 210.f, "wireframe", "Data/Images/white.png");

	CollisionRigidBody* plane_rb_0 = new CollisionRigidBody(1.f, Vector3(0.f, 200.f, 0.f), Vector3(90.f, 0.f, 0.f));
	plane_rb_0->SetAwake(true);
	plane_rb_0->SetSleepable(false);
	m_plane_0->AttachToRigidBody(plane_rb_0);
	m_controlled_0 = m_plane_0;

	CollisionRigidBody* plane_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 210.f, 0.f), Vector3(90.f, 0.f, 0.f));
	plane_rb_1->SetAwake(true);
	plane_rb_1->SetSleepable(false);
	m_plane_1->AttachToRigidBody(plane_rb_1);
	m_controlled_1 = m_plane_1;

	// spheres
	m_sphere_0 = new CollisionSphere(1.f, "wireframe", "Data/Images/white.png");
	m_sphere_1 = new CollisionSphere(1.f, "wireframe", "Data/Images/white.png");

	CollisionRigidBody* sphere_rb_0 = new CollisionRigidBody(1.f, Vector3(0.f, 200.f, 0.f), Vector3::ZERO);
	sphere_rb_0->SetAwake(true);
	sphere_rb_0->SetSleepable(false);
	m_sphere_0->AttachToRigidBody(sphere_rb_0);

	CollisionRigidBody* sphere_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 210.f, 0.f), Vector3::ZERO);
	sphere_rb_1->SetAwake(true);
	sphere_rb_1->SetSleepable(false);
	m_sphere_1->AttachToRigidBody(sphere_rb_1);

	// boxes
	m_box_0 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");
	m_box_1 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");

	CollisionRigidBody* box_rb_0 = new CollisionRigidBody(1.f, Vector3(0.f, 200.f, 0.f), Vector3::ZERO);
	box_rb_0->SetAwake(true);
	box_rb_0->SetSleepable(false);
	m_box_0->AttachToRigidBody(box_rb_0);

	CollisionRigidBody* box_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 210.f, 0.f), Vector3::ZERO);
	box_rb_1->SetAwake(true);
	box_rb_1->SetSleepable(false);
	m_box_1->AttachToRigidBody(box_rb_1);

	// aabb
	m_aabb_0 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");
	m_aabb_1 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");

	CollisionRigidBody* aabb_rb_0 = new CollisionRigidBody(1.f, Vector3(0.f, 200.f, 0.f), Vector3::ZERO);
	aabb_rb_0->SetAwake(true);
	aabb_rb_0->SetSleepable(false);
	m_aabb_0->AttachToRigidBody(aabb_rb_0);

	CollisionRigidBody* aabb_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 210.f, 0.f), Vector3::ZERO);
	aabb_rb_1->SetAwake(true);
	aabb_rb_1->SetSleepable(false);
	m_aabb_1->AttachToRigidBody(aabb_rb_1);

	// line
	m_line_0 = new CollisionLine(Vector3(-5.f, 200.f, 0.f), Vector3(5.f, 200.f, 0.f), "wireframe", "Data/Images/white.png");
	m_line_1 = new CollisionLine(Vector3(-5.f, 210.f, 0.f), Vector3(5.f, 210.f, 0.f), "wireframe", "Data/Images/white.png");
	// what if there is no rigid bodies for lines?

	// ui
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	m_text_height = height / 50.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - m_text_height);

	std::string title_ui = Stringf("Welcome to control state");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, m_text_height, .5f, title_ui, VERT_PCU);
	m_title_ui = t_mesh;
	titleMin -= Vector2(0.f, m_text_height);
	m_start_min = titleMin;

	// qh
	Vector3 qh_min = Vector3(-300.f, 0.f, 0.f);
	Vector3 qh_max = Vector3(-200.f, 100.f, 100.f);
	m_qh = new QuickHull(20, qh_min, qh_max);

	m_inspection.push_back(Vector3(0.f, 0.f, -80.f));
	m_inspection.push_back(Vector3(0.f, 200.f, -20.f));
	m_inspection.push_back(Vector3(-250.f, 50.f, -50.f));

	// hulls - point def
	Vector3 pos = Vector3(-35.f, 0.f, 0.f);
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Vector3 hull_ext = Vector3(15.f);
	Rgba tint = Rgba::WHITE;
	eMoveStatus stat = MOVE_KINEMATIC;
	eBodyIdentity bid = BODY_RIGID;
	bool multipass = false;
	eDepthCompare compare = COMPARE_LESS;
	eCullMode cull = CULLMODE_BACK;
	eWindOrder wind = WIND_COUNTER_CLOCKWISE;
	m_hobj_0 = new HullObject(pos, hull_ext, rot, scale, tint, "white", stat, bid, 5, "", multipass, compare, cull, wind);
	pos = Vector3(35.f, 0.f, 0.f);
	rot = Vector3(45.f, 0.f, 0.f);
	m_hobj_1 = new HullObject(pos, hull_ext, rot, scale, tint, "white", stat, bid, 5, "", multipass, compare, cull, wind);
	m_hobj_0->SetHullObject();
	m_hobj_1->SetHullObject();

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

ControlState3::~ControlState3()
{

}

void ControlState3::Update(float deltaTime)
{
	UpdateKeyboard(deltaTime);
	UpdateMouse(deltaTime);
	UpdatePair(deltaTime);
	UpdateContacts(deltaTime);
	UpdateDebugDraw(deltaTime);
	UpdateUI(deltaTime);
}

void ControlState3::UpdateMouse(float deltaTime)
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

void ControlState3::UpdateKeyboard(float deltaTime)
{
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Z))
	{
		// Reset camera position, euler and scale 
		m_camera->GetTransform().SetLocalPosition(m_cameraInitialPos);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}

	// Apply camera movement
	float leftRight = 0.f;
	float forwardBack = 0.f;
	float upDown = 0.f; 

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_A))
		leftRight = -20.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_D))
		leftRight = 20.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_W))
		forwardBack = 20.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_S))
		forwardBack = -20.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_Q))
		upDown = 20.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_E))
		upDown = -20.f;

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_I))
	{
		if (m_cid_0 != CID_LINE)
			m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
		else
		{
			CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
			line_0->SetStart(line_0->GetStart() + Vector3(0.f, 0.f, .1f));
		}
	}
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
	{
		if (m_cid_0 != CID_LINE)
			m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
		else
		{
			CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
			line_0->SetStart(line_0->GetStart() + Vector3(0.f, 0.f, -.1f));
		}
	}
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
	{
		if (m_cid_0 != CID_LINE)
			m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
		else
		{
			CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
			line_0->SetStart(line_0->GetStart() + Vector3(-.1f, 0.f, 0.f));
		}
	}
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
	{
		if (m_cid_0 != CID_LINE)
			m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
		else
		{
			CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
			line_0->SetStart(line_0->GetStart() + Vector3(.1f, 0.f, 0.f));
		}
	}
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
	{
		if (m_cid_0 != CID_LINE)
			m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
		else
		{
			CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
			line_0->SetStart(line_0->GetStart() + Vector3(0.f, .1f, 0.f));
		}
	}
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
	{
		if (m_cid_0 != CID_LINE)
			m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
		else
		{
			CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
			line_0->SetStart(line_0->GetStart() + Vector3(0.f, -.1f, 0.f));
		}
	}
	else if (m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_2) && m_cid_0 != CID_AABB && m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5) && m_cid_0 != CID_AABB && m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_8) && m_cid_0 != CID_AABB && m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(0.f, 0.f, 5.f));
	else if (m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);

	// control 0
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_SHIFT))
	{
		switch (m_cid_0)
		{
		case CID_PLANE:
			m_controlled_0 = m_sphere_0;
			m_cid_0 = CID_SPHERE;
			break;
		case CID_BOX:
			m_controlled_0 = m_plane_0;
			m_cid_0 = CID_PLANE;
			break;
		case CID_SPHERE:
			m_controlled_0 = m_convex_0;
			m_cid_0 = CID_CONVEX;
			break;
		case CID_CONVEX:
			m_controlled_0 = m_aabb_0;
			m_cid_0 = CID_AABB;
			break;
		case CID_AABB:
			m_controlled_0 = m_line_0;
			m_cid_0 = CID_LINE;
			break;
		case CID_LINE:
			m_controlled_0 = m_box_0;
			m_cid_0 = CID_BOX;
			break;
		default:
			break;
		}
	}

	// control 1 
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_CAPITAL))
	{
		switch (m_cid_1)
		{
		case CID_PLANE:
			m_controlled_1 = m_sphere_1;
			m_cid_1 = CID_SPHERE;
			break;
		case CID_BOX:
			m_controlled_1 = m_plane_1;
			m_cid_1 = CID_PLANE;
			break;
		case CID_SPHERE:
			m_controlled_1 = m_convex_1;
			m_cid_1 = CID_CONVEX;
			break;
		case CID_CONVEX:
			m_controlled_1 = m_aabb_1;
			m_cid_1 = CID_AABB;
			break;
		case CID_AABB:
			m_controlled_1 = m_line_1;
			m_cid_1 = CID_LINE;
			break;
		case CID_LINE:
			m_controlled_1 = m_box_1;
			m_cid_1 = CID_BOX;
			break;
		default:
			break;
		}
	}

	// hull 0 for gjk
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
	{
		Vector3 pos = m_hobj_0->m_renderable->m_transform.GetLocalPosition();
		pos += Vector3(0.f, 0.f, 1.f);
		m_hobj_0->m_renderable->m_transform.SetLocalPosition(pos);
		net_disp = Vector3(0.f, 0.f, 1.f);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
	{
		Vector3 pos = m_hobj_0->m_renderable->m_transform.GetLocalPosition();
		pos -= Vector3(0.f, 0.f, 1.f);
		m_hobj_0->m_renderable->m_transform.SetLocalPosition(pos);
		net_disp = -Vector3(0.f, 0.f, 1.f);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
	{
		Vector3 pos = m_hobj_0->m_renderable->m_transform.GetLocalPosition();
		pos -= Vector3(1.f, 0.f, 0.f);
		m_hobj_0->m_renderable->m_transform.SetLocalPosition(pos);
		net_disp = -Vector3(1.f, 0.f, 0.f);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
	{
		Vector3 pos = m_hobj_0->m_renderable->m_transform.GetLocalPosition();
		pos += Vector3(1.f, 0.f, 0.f);
		m_hobj_0->m_renderable->m_transform.SetLocalPosition(pos);
		net_disp = Vector3(1.f, 0.f, 0.f);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
	{
		Vector3 pos = m_hobj_0->m_renderable->m_transform.GetLocalPosition();
		pos += Vector3(0.f, 1.f, 0.f);
		m_hobj_0->m_renderable->m_transform.SetLocalPosition(pos);
		net_disp = Vector3(0.f, 1.f, 0.f);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
	{
		Vector3 pos = m_hobj_0->m_renderable->m_transform.GetLocalPosition();
		pos -= Vector3(0.f, 1.f, 0.f);
		m_hobj_0->m_renderable->m_transform.SetLocalPosition(pos);
		net_disp = -Vector3(0.f, 1.f, 0.f);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_1))
	{
		Vector3 rot = m_hobj_0->m_renderable->m_transform.GetLocalRotation();
		rot += Vector3(1.f, 0.f, 0.f);
		m_hobj_0->m_renderable->m_transform.SetLocalRotation(rot);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
	{
		Vector3 rot = m_hobj_0->m_renderable->m_transform.GetLocalRotation();
		rot += Vector3(0.f, 1.f, 0.f);
		m_hobj_0->m_renderable->m_transform.SetLocalRotation(rot);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_7))
	{
		Vector3 rot = m_hobj_0->m_renderable->m_transform.GetLocalRotation();
		rot += Vector3(0.f, 0.f, 1.f);
		m_hobj_0->m_renderable->m_transform.SetLocalRotation(rot);
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0))
	{
		ManualGenQH(m_qh);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3))
	{
		if (!gjk_hull)
		{
			QuickHull* qh_0 = m_hobj_0->GetHullPrimitive();
			QuickHull* qh_1 = m_hobj_1->GetHullPrimitive();

			gjk_hull = QuickHull::GenerateMinkowskiHull(qh_0, qh_1);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_6))
	{
		if (gjk_hull)
		{
			switch (gjk_stat)
			{
			case GJK_FIND_SUPP_INITIAL:
			{
				gjk_supp = gjk_hull->GetRandomPt();
				if (gjk_supp_mesh != nullptr)
				{
					delete gjk_supp_mesh;
					gjk_supp_mesh= nullptr;
				}
				gjk_supp_mesh = Mesh::CreatePointImmediate(VERT_PCU, gjk_supp, Rgba::RED);

				// update simplex
				gjk_simplex.insert(gjk_supp);
				gjk_simplex_stat = GJK_UpdateSimplex(gjk_simplex);
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				gjk_simplex_mesh = GJK_CreateSimplexMesh(gjk_simplex, gjk_simplex_stat);

				gjk_stat = GJK_FIND_DIRECTION_INITIAL;
			}
			break;
			case GJK_FIND_DIRECTION_INITIAL:
			{
				// given the supp point, find the direction to start with iterations
				gjk_supp_dir = Line3(gjk_supp, Vector3::ZERO);
				if (gjk_supp_dir_mesh != nullptr)
				{
					delete gjk_supp_dir_mesh;
					gjk_supp_dir_mesh = nullptr;
				}
				gjk_supp_dir_mesh = Mesh::CreateLineImmediate(VERT_PCU, gjk_supp,Vector3::ZERO, Rgba::CYAN);

				gjk_stat = GJK_FIND_SUPP;
			}
			break;
			case GJK_FIND_SUPP:
			{
				gjk_supp = GJK_FindSupp(gjk_hull, gjk_supp_dir);
				if (gjk_supp_mesh != nullptr)
				{
					delete gjk_supp_mesh;
					gjk_supp_mesh = nullptr;
				}
				gjk_supp_mesh = Mesh::CreatePointImmediate(VERT_PCU, gjk_supp, Rgba::RED);

				// update simplex
				std::vector<Vector3> gjk_simplex_snapshot = ConvertToVectorFromSet(gjk_simplex);
				gjk_simplex.insert(gjk_supp);
				gjk_simplex_stat = GJK_UpdateSimplex(gjk_simplex);
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				gjk_simplex_mesh = GJK_CreateSimplexMesh(gjk_simplex, gjk_simplex_stat);

				// TODO: when the simplex is a tetrahedron, we check if the origin is in it; if yes we abort and the two hulls intersect
				// there may be a better early out check on this using an IsPointContainedHull(Vector3::ZERO)
				if (gjk_simplex.size() == 4)
				{
					// Note: this also means that when gjk ends the simplex is mostly a tetrahedron? (not a triangle or line)
					// when it is triangle it is most when the origin is outside the mksi_hull
					GJK3SimplexTetra tetra = GJK3SimplexTetra(gjk_simplex);
					if (tetra.IsPointInTetra(Vector3::ZERO))
					{
						gjk_closest_dist = -INFINITY;
						gjk_stat = GJK_COMPLETE;
					}
					else
						gjk_stat = GJK_UPDATE_MIN_NORMAL;
				}
				else
					gjk_stat = GJK_UPDATE_MIN_NORMAL;
			}
			break;
			case GJK_UPDATE_MIN_NORMAL:
			{
				// find the min normal of current simplex
				float dist;
				gjk_normal_base= GJK_FindMinNormalBase(gjk_simplex, gjk_simplex_stat, dist);

				// there is a chance where finding the normal base would reduce the simplex
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				gjk_simplex_mesh = GJK_CreateSimplexMesh(gjk_simplex, gjk_simplex_stat);

				if (gjk_normal_base_mesh != nullptr)
				{
					delete gjk_normal_base_mesh;
					gjk_normal_base_mesh = nullptr;
				}
				gjk_normal_base_mesh = Mesh::CreatePointImmediate(VERT_PCU, gjk_normal_base, Rgba::YELLOW);

				// if the normal base remains the same, gjk ends
				if (gjk_normal_base == gjk_last_normal_base)
				{
					gjk_closest_dist = dist;
					gjk_stat = GJK_COMPLETE;
				}
				else
				{
					gjk_last_normal_base = gjk_normal_base;
					gjk_stat = GJK_FIND_DIRECTION;
				}
			}
			break;
			case GJK_FIND_DIRECTION:
			{
				gjk_supp_dir = Line3(gjk_normal_base, Vector3::ZERO);
				if (gjk_supp_dir_mesh != nullptr)
				{
					delete gjk_supp_dir_mesh;
					gjk_supp_dir_mesh= nullptr;
				}
				gjk_supp_dir_mesh = Mesh::CreateLineImmediate(VERT_PCU, gjk_normal_base,Vector3::ZERO, Rgba::CYAN);

				gjk_stat = GJK_FIND_SUPP;
			}
			break;
			case GJK_COMPLETE:
			{
				// do nothing...
			}
			break;
			default:
				break;
			}
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_9))
	{
		if (gjk_hull != nullptr && gjk_stat == GJK_COMPLETE)
		{
			switch (epa_stat)
			{
			case EPA_DELETE_GJK_REF:
			{
				if (gjk_supp_mesh != nullptr)
				{
					delete gjk_supp_mesh;
					gjk_supp_mesh = nullptr;
				}
				if (gjk_supp_dir_mesh != nullptr)
				{
					delete gjk_supp_dir_mesh;
					gjk_supp_dir_mesh = nullptr;
				}
				if (gjk_simplex_mesh != nullptr)
				{
					delete gjk_simplex_mesh;
					gjk_simplex_mesh = nullptr;
				}
				if (gjk_normal_base_mesh != nullptr)
				{
					delete gjk_normal_base_mesh;
					gjk_normal_base_mesh = nullptr;
				}

				epa_stat = EPA_CREATE_SIMPLEX;
			}
			break;
			case EPA_CREATE_SIMPLEX:
			{
				epa_simplex = new sEPASimplex(gjk_simplex);

				epa_stat = EPA_FIND_FACE;
			}
			break;
			case EPA_FIND_FACE:
			{
				epa_close_face = epa_simplex->SelectClosestFaceToOrigin();
				epa_support_anchor_pos = ProjectPointToPlane(Vector3::ZERO, epa_close_face->m_verts[0], epa_close_face->m_verts[1], epa_close_face->m_verts[2], epa_close_dist);

				Vector3 centroid = (epa_close_face->m_verts[0] + epa_close_face->m_verts[1] + epa_close_face->m_verts[2]) / 3.f;
				if (epa_face_centroid != nullptr)
				{
					delete epa_face_centroid;
					epa_face_centroid = nullptr;
				}
				epa_face_centroid = Mesh::CreatePointImmediate(VERT_PCU, centroid, Rgba::GREEN);

				if (epa_support_anchor != nullptr)
				{
					delete epa_support_anchor;
					epa_support_anchor = nullptr;
				}
				epa_support_anchor = Mesh::CreatePointImmediate(VERT_PCU, epa_support_anchor_pos, Rgba::BLUE);

				if (epa_support_line != nullptr)
				{
					delete epa_support_line;
					epa_support_line = nullptr;
				}
				epa_support_line = Mesh::CreateLineImmediate(VERT_PCU, Vector3::ZERO, epa_support_anchor_pos, Rgba::CYAN);

				// finally, the support point should be on the hull
				Line3 epa_support_dir = Line3(Vector3::ZERO, epa_support_anchor_pos);
				epa_support_pt_pos = EPA_FindSupp(gjk_hull, epa_support_dir);

				// if the support point remains the same, we've reached end
				if (epa_support_pt_pos != epa_last_support_pt_pos)
					epa_last_support_pt_pos = epa_support_pt_pos;
				else
					epa_stat = EPA_COMPLETE;

				if (epa_stat != EPA_COMPLETE)
				{
					// IMPORTANT: got the newest support point, will abort if dist from this point to origin
					// MINUS the threshold is smaller than the distance from closest face to origin
					if (((epa_support_pt_pos - Vector3::ZERO).GetLength() - epa_threshold) < epa_close_dist)
						epa_stat = EPA_COMPLETE;
					else 
					{
						if (epa_support_pt != nullptr)
						{
							delete epa_support_pt;
							epa_support_pt = nullptr;
						}
						epa_support_pt = Mesh::CreatePointImmediate(VERT_PCU, epa_support_pt_pos, Rgba::RED);

						epa_stat = EPA_DELETE_VISIBLE;
					}
				}
			}
			break;
			case EPA_DELETE_VISIBLE:
			{
				bool deleted = epa_simplex->DeleteVisibleFacesForPt(epa_support_pt_pos);

				// delete old debug draws
				if (epa_support_anchor != nullptr)
				{
					delete epa_support_anchor;
					epa_support_anchor = nullptr;
				}
				if (epa_support_line != nullptr)
				{
					delete epa_support_line;
					epa_support_line = nullptr;
				}
				if (epa_face_centroid != nullptr)
				{
					delete epa_face_centroid;
					epa_face_centroid = nullptr;
				}

				epa_stat = EPA_FORM_NEW_FACE;
			}
			break;
			case EPA_FORM_NEW_FACE:
			{
				// generate new faces between the supp point and edges with 1 ref count
				std::vector<sEPAEdgeRef*> new_edges;
				for (sEPAEdgeRef* edge : epa_simplex->m_edge_refs)
				{
					if (edge->ref_count == 1)
						epa_simplex->FormNewFace(epa_support_pt_pos, edge, epa_support_anchor_pos, new_edges);
				}
				epa_simplex->AppendNewEdges(new_edges);
				new_edges.clear();

				epa_stat = EPA_FIND_FACE;
			}
			break;
			case EPA_COMPLETE:
			{
				// in the step of completion, release all unnecessary heap memory
				if (epa_face_centroid != nullptr)
				{
					delete epa_face_centroid;
					epa_face_centroid = nullptr;
				}

				if (epa_support_anchor != nullptr)
				{
					delete epa_support_anchor;
					epa_support_anchor = nullptr;
				}

				if (epa_support_line != nullptr)
				{
					delete epa_support_line;
					epa_support_line = nullptr;
				}

				if (epa_support_pt != nullptr)
				{
					delete epa_support_pt;
					epa_support_pt = nullptr;
				}

				epa_stat = EPA_POST_COMPLETE;
			}
			break;
			case EPA_POST_COMPLETE:
			{
				// debug draw
				const Vector3& n = epa_close_face->m_normal;
				DebugRenderLine(.1f, epa_support_pt_pos, epa_support_pt_pos + n * epa_close_dist, 5.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
			}
			break;
			default:
				break;
			}
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

void ControlState3::UpdateDebugDraw(float deltaTime)
{
	DebugRenderUpdate(deltaTime);

	uint count = 0;
	Collision* current = m_keep.m_collision_head;
	while (count != m_keep.m_collision_count)
	{
		const Vector3& n = current->GetNormal();
		const Vector3& pt = current->GetPos();
		const float& pen = current->GetPenetration();
		Vector3 pt_end = pt + n * pen;

		DebugRenderLine(.1f, pt, pt_end, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
		DebugRenderLine(.1f, pt_end, pt_end + n * pen, 5.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);

		count++;
		current = m_keep.m_collision_head + count;
	}
}

void ControlState3::UpdatePair(float deltaTime)
{
	m_controlled_0->Update(deltaTime);
	m_controlled_1->Update(deltaTime);

	// gjk demo
	m_hobj_0->Update(deltaTime);
	m_hobj_1->Update(deltaTime);
	if (gjk_hull != nullptr)
	{
		gjk_hull->m_ref_pos += net_disp;

		for (std::vector<Vector3>::size_type idx = 0; idx < gjk_hull->m_vertices.size(); ++idx)
			gjk_hull->m_vertices[idx] += net_disp;

		net_disp = Vector3::ZERO;
	}
}

void ControlState3::UpdateContacts(float deltaTime)
{
	m_keep.Reset(MAX_CONTACT_NUM);
	m_keep.m_global_friction = .9f;
	m_keep.m_global_restitution = .1f;
	m_keep.m_tolerance = .1f;

	if (!m_keep.AllowMoreCollision())
		return;

	if (m_cid_0 == CID_PLANE && m_cid_1 == CID_PLANE)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_PLANE && m_cid_1 == CID_BOX)
		CollisionSensor::BoxVsHalfPlane(*m_box_1, *m_plane_0, &m_keep);
	else if (m_cid_0 == CID_PLANE && m_cid_1 == CID_SPHERE)
		CollisionSensor::SphereVsPlane(*m_sphere_1, *m_plane_0, &m_keep);
	else if (m_cid_0 == CID_PLANE && m_cid_1 == CID_CONVEX)
		CollisionSensor::ConvexVsHalfPlane(*m_convex_1, *m_plane_0, &m_keep);
	else if (m_cid_0 == CID_PLANE && m_cid_1 == CID_AABB)
	{
		// nothing yet...
	}
	else if (m_cid_0 == CID_PLANE && m_cid_1 == CID_LINE)
	{
		// todo
	}
	else if (m_cid_0 == CID_BOX && m_cid_1 == CID_PLANE)
		CollisionSensor::BoxVsHalfPlane(*m_box_0, *m_plane_1, &m_keep);
	else if (m_cid_0 == CID_BOX && m_cid_1 == CID_SPHERE)
		CollisionSensor::BoxVsSphere(*m_box_0, *m_sphere_1, &m_keep);
	else if (m_cid_0 == CID_BOX && m_cid_1 == CID_BOX)
		CollisionSensor::BoxVsBox(*m_box_0, *m_box_1, &m_keep);
	else if (m_cid_0 == CID_BOX && m_cid_1 == CID_CONVEX)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_BOX && m_cid_1 == CID_AABB)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_BOX && m_cid_1 == CID_LINE)
	{
		// todo
	}
	else if (m_cid_0 == CID_SPHERE && m_cid_1 == CID_PLANE)
		CollisionSensor::SphereVsPlane(*m_sphere_0, *m_plane_1, &m_keep);
	else if (m_cid_0 == CID_SPHERE && m_cid_1 == CID_SPHERE)
		CollisionSensor::SphereVsSphere(*m_sphere_0, *m_sphere_1, &m_keep);
	else if (m_cid_0 == CID_SPHERE && m_cid_1 == CID_BOX)
		CollisionSensor::BoxVsSphere(*m_box_1, *m_sphere_0, &m_keep);
	else if (m_cid_0 == CID_SPHERE && m_cid_1 == CID_CONVEX)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_SPHERE && m_cid_1 == CID_AABB)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_SPHERE && m_cid_1 == CID_LINE)
	{
		// todo
	}
	else if (m_cid_0 == CID_CONVEX && m_cid_1 == CID_PLANE)
		CollisionSensor::ConvexVsHalfPlane(*m_convex_0, *m_plane_1, &m_keep);
	else if (m_cid_0 == CID_CONVEX && m_cid_1 == CID_BOX)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_CONVEX && m_cid_1 == CID_SPHERE)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_CONVEX && m_cid_1 == CID_CONVEX)
	{
		// do nothing...
	}
	else if (m_cid_0 == CID_CONVEX && m_cid_1 == CID_AABB)
	{
		// nothing yet...
	}
	else if (m_cid_0 == CID_CONVEX && m_cid_1 == CID_LINE)
	{
		// todo
	}
	else if (m_cid_0 == CID_AABB && m_cid_1 == CID_AABB)
	{
		// nothing yet...
	}
	else if (m_cid_0 == CID_AABB && m_cid_1 == CID_BOX)
	{
		// nothing yet...
	}
	else if (m_cid_0 == CID_AABB && m_cid_1 == CID_CONVEX)
	{
		// nothing yet...
	}
	else if (m_cid_0 == CID_AABB && m_cid_1 == CID_SPHERE)
	{
		// nothing yet..
	}
	else if (m_cid_0 == CID_AABB && m_cid_1 == CID_PLANE)
	{
		// nothing yet...
	}
	else if (m_cid_0 == CID_AABB && m_cid_1 == CID_LINE)
	{
		// nothing...
	}
	else if (m_cid_0 == CID_LINE && m_cid_1 == CID_LINE)
	{
		CollisionLine* line_0 = static_cast<CollisionLine*>(m_controlled_0);
		CollisionLine* line_1 = static_cast<CollisionLine*>(m_controlled_1);
		
		LineSegment3 seg_0 = LineSegment3(line_0->GetStart(), line_0->GetEnd());
		LineSegment3 seg_1 = LineSegment3(line_1->GetStart(), line_1->GetEnd());

		Vector3 close_0, close_1;
		float t0, t1;
		float close_sqr = LineSegment3::ClosestPointsSegments(seg_0, seg_1, t0, t1, close_0, close_1);
		DebugRenderLine(.1f, close_0, close_1, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
	}
	else if (m_cid_0 == CID_LINE && m_cid_1 == CID_SPHERE)
	{
		// todo...
	}
	else if (m_cid_0 == CID_LINE && m_cid_1 == CID_AABB)
	{
		// nothing...
    }
	else if (m_cid_0 == CID_LINE && m_cid_1 == CID_BOX)
	{
		// todo...
	}
	else if (m_cid_0 == CID_LINE && m_cid_1 == CID_PLANE)
	{
		// todo...
	}
	else if (m_cid_0 == CID_LINE && m_cid_1 == CID_CONVEX)
	{
		// todo...
	}
}

void ControlState3::UpdateUI(float deltaTime)
{
	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	DeleteVector(m_dynamic_ui);

	Vector2 min = m_start_min;

	// gjk debug
	if (gjk_stat == GJK_COMPLETE)
	{
		std::string dist_str = Stringf("Dist (-inf if point inside hull): %f", gjk_closest_dist);
		Mesh* m = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_text_height, .5f, dist_str, VERT_PCU);
		m_dynamic_ui.push_back(m);
		min -= Vector2(0.f, m_text_height);
	}

	// epa debug
	if (epa_stat >= EPA_DELETE_VISIBLE)
	{
		std::string face_str = std::to_string(epa_simplex->m_unordered_faces.size());
		face_str += " - number of face of EPA simplex";
		Mesh* m = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_text_height, .5f, face_str, VERT_PCU);
		m_dynamic_ui.push_back(m);
		min -= Vector2(0.f, m_text_height);
	}

	if (epa_stat == EPA_POST_COMPLETE)
	{
		std::string complete_str = "EPA Complete!";
		Mesh* m = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_text_height, .5f, complete_str, VERT_PCU);
		m_dynamic_ui.push_back(m);
		min -= Vector2(0.f, m_text_height);

		// normal is the negativity of triangle normal here, think about the inducing hull
		Vector3 n = epa_close_face->m_normal;
		n.Normalize();
		std::string normal_str = Stringf("The normal is: (%f, %f, %f)", n.x, n.y, n.z);
		m = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_text_height, .5f, normal_str, VERT_PCU);
		m_dynamic_ui.push_back(m);
		min -= Vector2(0.f, m_text_height);

		std::string pen_str = Stringf("The penetration depth is: %f", epa_close_dist);
		m = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_text_height, .5f, pen_str, VERT_PCU);
		m_dynamic_ui.push_back(m);
		min -= Vector2(0.f, m_text_height);

		std::string pt_str = Stringf("point: %f, %f, %f", epa_support_pt_pos);
		m = Mesh::CreateTextImmediate(Rgba::WHITE, min, font, m_text_height, .5f, pt_str, VERT_PCU);
		m_dynamic_ui.push_back(m);
		min -= Vector2(0.f, m_text_height);
	}
}

void ControlState3::Render(Renderer* renderer)
{
	// draw UI
	renderer->SetCamera(m_UICamera);
	renderer->ClearScreen(Rgba::BLACK);

	// title
	DrawTextCut(m_title_ui);

	// dynamic ui
	DrawTexts(m_dynamic_ui);

	// draw group contents
	renderer->SetCamera(m_camera);

	// control pair
	RenderPair(renderer);

	// quick hull demo
	m_qh->RenderHull(renderer);

	// gjk demo
	m_hobj_0->Render(renderer);
	m_hobj_1->Render(renderer);
	if (gjk_hull != nullptr)
		gjk_hull->RenderHull(renderer);
	if (gjk_supp_mesh != nullptr)
		DrawPoint(gjk_supp_mesh);
	if (gjk_supp_dir_mesh != nullptr)
		DrawLine(gjk_supp_dir_mesh);
	if (gjk_simplex_mesh != nullptr)
		GJK_DrawSimplex(gjk_simplex_mesh, gjk_simplex_stat);
	if (gjk_normal_base_mesh != nullptr)
		DrawPoint(gjk_normal_base_mesh, 20.f);

	// epa demo
	if (epa_simplex != nullptr)
		epa_simplex->Draw(renderer);
	if (epa_support_anchor != nullptr)
		DrawPoint(epa_support_anchor);
	if (epa_support_line != nullptr)
		DrawLine(epa_support_line);
	if (epa_support_pt != nullptr)
		DrawPoint(epa_support_pt);
	if (epa_face_centroid != nullptr)
		DrawPoint(epa_face_centroid);

	// origin
	DrawPoint(m_origin);

	m_forwardPath->RenderScene(m_sceneGraph);
}

void ControlState3::RenderPair(Renderer* renderer)
{
	m_controlled_0->Render(renderer);
	m_controlled_1->Render(renderer);
}