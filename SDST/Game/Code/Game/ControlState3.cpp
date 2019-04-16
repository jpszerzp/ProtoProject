#include "Game/ControlState3.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Physics/3D/RF/CollisionDetector.hpp"


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
	const Vector3& true_center_0 = Vector3::ZERO;			 // com + (wrap_center - com); based on ORIGIN
	CollisionRigidBody* convex_rb_0 = new CollisionRigidBody(mass_0, true_center_0, Vector3::ZERO);
	convex_rb_0->SetAwake(true);
	convex_rb_0->SetSleepable(false);
	m_convex_0->AttachToRigidBody(convex_rb_0);

	const float& mass_1 = m_convex_1->GetInitialMass();
	const Vector3& true_center_1 = Vector3(0.f, 10.f, 0.f);
	CollisionRigidBody* convex_rb_1 = new CollisionRigidBody(mass_1, true_center_1, Vector3::ZERO);
	convex_rb_1->SetAwake(true);
	convex_rb_1->SetSleepable(false);
	m_convex_1->AttachToRigidBody(convex_rb_1);

	// planes
	m_plane_0 = new CollisionPlane(Vector2(20.f), "quad_pcu_20", Vector3(0.f, 1.f, 0.f), 0.f, "wireframe", "Data/Images/white.png");
	m_plane_1 = new CollisionPlane(Vector2(20.f), "quad_pcu_20", Vector3(0.f, 1.f, 0.f), 10.f, "wireframe", "Data/Images/white.png");

	CollisionRigidBody* plane_rb_0 = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3(90.f, 0.f, 0.f));
	plane_rb_0->SetAwake(true);
	plane_rb_0->SetSleepable(false);
	m_plane_0->AttachToRigidBody(plane_rb_0);
	m_controlled_0 = m_plane_0;

	CollisionRigidBody* plane_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 10.f, 0.f), Vector3(90.f, 0.f, 0.f));
	plane_rb_1->SetAwake(true);
	plane_rb_1->SetSleepable(false);
	m_plane_1->AttachToRigidBody(plane_rb_1);
	m_controlled_1 = m_plane_1;

	// spheres
	m_sphere_0 = new CollisionSphere(1.f, "wireframe", "Data/Images/white.png");
	m_sphere_1 = new CollisionSphere(1.f, "wireframe", "Data/Images/white.png");

	CollisionRigidBody* sphere_rb_0 = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3::ZERO);
	sphere_rb_0->SetAwake(true);
	sphere_rb_0->SetSleepable(false);
	m_sphere_0->AttachToRigidBody(sphere_rb_0);

	CollisionRigidBody* sphere_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 10.f, 0.f), Vector3::ZERO);
	sphere_rb_1->SetAwake(true);
	sphere_rb_1->SetSleepable(false);
	m_sphere_1->AttachToRigidBody(sphere_rb_1);

	// boxes
	m_box_0 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");
	m_box_1 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");

	CollisionRigidBody* box_rb_0 = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3::ZERO);
	box_rb_0->SetAwake(true);
	box_rb_0->SetSleepable(false);
	m_box_0->AttachToRigidBody(box_rb_0);

	CollisionRigidBody* box_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 10.f, 0.f), Vector3::ZERO);
	box_rb_1->SetAwake(true);
	box_rb_1->SetSleepable(false);
	m_box_1->AttachToRigidBody(box_rb_1);

	// aabb
	m_aabb_0 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");
	m_aabb_1 = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");

	CollisionRigidBody* aabb_rb_0 = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3::ZERO);
	aabb_rb_0->SetAwake(true);
	aabb_rb_0->SetSleepable(false);
	m_aabb_0->AttachToRigidBody(aabb_rb_0);

	CollisionRigidBody* aabb_rb_1 = new CollisionRigidBody(1.f, Vector3(0.f, 10.f, 0.f), Vector3::ZERO);
	aabb_rb_1->SetAwake(true);
	aabb_rb_1->SetSleepable(false);
	m_aabb_1->AttachToRigidBody(aabb_rb_1);

	// line
	m_line_0 = new CollisionLine(Vector3(-5.f, 0.f, 0.f), Vector3(5.f, 0.f, 0.f), "wireframe", "Data/Images/white.png");
	m_line_1 = new CollisionLine(Vector3(-5.f, 10.f, 0.f), Vector3(5.f, 10.f, 0.f), "wireframe", "Data/Images/white.png");
	// what if there is no rigid bodies for lines?

	// ui
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = height / 70.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - txtHeight);

	std::string title_ui = Stringf("Welcome to control state");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, title_ui, VERT_PCU);
	m_title_ui = t_mesh;
	titleMin -= Vector2(0.f, txtHeight);

	// qh
	Vector3 qh_min = Vector3(-300.f, 0.f, 0.f);
	Vector3 qh_max = Vector3(-200.f, 100.f, 100.f);
	m_qh = new QuickHull(20, qh_min, qh_max);

	m_inspection.push_back(m_cameraInitialPos);
	m_inspection.push_back(Vector3(-250.f, 50.f, -50.f));

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
	//UpdateUI(deltaTime);
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
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(30.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5) && m_cid_0 != CID_AABB && m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(0.f, 30.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_8) && m_cid_0 != CID_AABB && m_cid_0 != CID_LINE)
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(0.f, 0.f, 30.f));
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
	
}

void ControlState3::Render(Renderer* renderer)
{
	// draw UI
	renderer->SetCamera(m_UICamera);
	renderer->ClearScreen(Rgba::BLACK);
	DrawTextCut(m_title_ui);

	// draw group contents
	renderer->SetCamera(m_camera);
	RenderPair(renderer);

	m_qh->RenderHull(renderer);

	m_forwardPath->RenderScene(m_sceneGraph);
}

void ControlState3::RenderPair(Renderer* renderer)
{
	m_controlled_0->Render(renderer);
	m_controlled_1->Render(renderer);
}
