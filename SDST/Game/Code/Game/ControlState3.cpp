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

	CollisionConvexObject* cObj = new CollisionConvexObject(*cHull, "wireframe", "Data/Images/white.png");

	const float& mass = cObj->GetInitialMass();
	const Vector3& true_center = Vector3(0.f, 3.f, 0.f);			 // com + (wrap_center - com); based on ORIGIN
	CollisionRigidBody* rb = new CollisionRigidBody(mass, true_center, Vector3::ZERO);
	rb->SetAwake(true);
	rb->SetSleepable(false);

	cObj->AttachToRigidBody(rb);
	m_convex_objs.push_back(cObj);
	m_controlled_0 = cObj;

	CollisionPlane* plane_0 = new CollisionPlane(Vector2(20.f), "quad_pcu_20", Vector3(0.f, 1.f, 0.f), 0.f, "wireframe", "Data/Images/white.png");

	rb = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3(90.f, 0.f, 0.f));
	rb->SetAwake(true);
	rb->SetSleepable(false);

	plane_0->AttachToRigidBody(rb);
	m_planes.push_back(plane_0);
	m_controlled_1 = plane_0;

	// ui
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = height / 70.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - txtHeight);

	std::string title_ui = Stringf("Welcome to control state");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, title_ui, VERT_PCU);
	m_title_ui = t_mesh;
	titleMin -= Vector2(0.f, txtHeight);

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
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, 0.f, 5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, 0.f, -5.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(-5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(5.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, 5.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3(0.f, -5.f, 0.f));
	else
		m_controlled_0->GetRigidBody()->SetLinearVelocity(Vector3::ZERO);

	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_2))
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(30.f, 0.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5))
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(0.f, 30.f, 0.f));
	else if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_8))
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3(0.f, 0.f, 30.f));
	else 
		m_controlled_0->GetRigidBody()->SetAngularVelocity(Vector3::ZERO);

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_TAB))
	{
		switch (m_cid)
		{
		case CID_PLANE:
			if (!m_boxes.empty())
				m_controlled_1 = m_boxes[0];
			else
			{
				CollisionBox* cbox = new CollisionBox(Vector3(.5f), "wireframe", "Data/Images/white.png");

				CollisionRigidBody* rb = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3::ZERO);
				rb->SetAwake(true);
				rb->SetSleepable(false);

				cbox->AttachToRigidBody(rb);

				m_boxes.push_back(cbox);

				m_controlled_1 = cbox;
			}

			m_cid = CID_BOX;
			break;
		case CID_BOX:
			if (!m_spheres.empty())
				m_controlled_1 = m_spheres[0];
			else
			{
				CollisionSphere* csph = new CollisionSphere(1.f, "wireframe", "Data/Images/white.png");

				CollisionRigidBody* rb = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3::ZERO);
				rb->SetAwake(true);
				rb->SetSleepable(false);

				csph->AttachToRigidBody(rb);

				m_spheres.push_back(csph);

				m_controlled_1 = csph;
			}

			m_cid = CID_SPHERE;
			break;
		case CID_SPHERE:
			if (!m_planes.empty())
				m_controlled_1 = m_planes[0];
			else
			{
				CollisionPlane* plane_0 = new CollisionPlane(Vector2(20.f), "quad_pcu_20", Vector3(0.f, 1.f, 0.f), 0.f, "wireframe", "Data/Images/white.png");

				CollisionRigidBody* rb = new CollisionRigidBody(1.f, Vector3::ZERO, Vector3(90.f, 0.f, 0.f));
				rb->SetAwake(true);
				rb->SetSleepable(false);

				plane_0->AttachToRigidBody(rb);
				m_planes.push_back(plane_0);
				m_controlled_1 = plane_0;
			}

			m_cid = CID_PLANE;
			break;
		default:
			break;
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

		DebugRenderLine(.1f, pt, pt + n * pen, 5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);

		count++;
		current = m_keep.m_collision_head + count;
	}
}

/*
void ControlState3::UpdateObjects(float deltaTime)
{
	for (int i = 0; i < m_convex_objs.size(); ++i)
		m_convex_objs[i]->Update(deltaTime);

	for (int i = 0; i < m_planes.size(); ++i)
		m_planes[i]->Update(deltaTime);
}
*/

void ControlState3::UpdatePair(float deltaTime)
{
	m_controlled_0->Update(deltaTime);
	m_controlled_1->Update(deltaTime);
}

void ControlState3::UpdateContacts(float)
{
	m_keep.Reset(MAX_CONTACT_NUM);
	m_keep.m_global_friction = .9f;
	m_keep.m_global_restitution = .1f;
	m_keep.m_tolerance = .1f;

	for (std::vector<CollisionConvexObject*>::size_type idx0 = 0; idx0 < m_convex_objs.size(); ++idx0)
	{
		CollisionConvexObject* c_obj_0 = m_convex_objs[idx0];

		if (m_cid == CID_PLANE)
		{
			// convex vs plane
			for (std::vector<CollisionPlane*>::size_type idx1 = 0; idx1 < m_planes.size(); ++idx1)
			{
				if (!m_keep.AllowMoreCollision())
					return;

				CollisionPlane* pl = m_planes[idx1];

				CollisionSensor::ConvexVsHalfPlane(*c_obj_0, *pl, &m_keep);
			}
		}

		// ...vs box
		if (m_cid == CID_BOX)
		{
			for (std::vector<CollisionBox*>::size_type idx1 = 0; idx1 < m_boxes.size(); ++idx1)
			{
				if (!m_keep.AllowMoreCollision())
					return;

				CollisionBox* bx = m_boxes[idx1];

				CollisionSensor::ConvexVsBox(*c_obj_0, *bx, &m_keep);
			}
		}

		if (m_cid == CID_SPHERE)
		{
			// ... vs sphere
			for (std::vector<CollisionSphere*>::size_type idx1 = 0; idx1 < m_spheres.size(); ++idx1)
			{
				if (!m_keep.AllowMoreCollision())
					return;

				CollisionSphere* sph = m_spheres[idx1];

				CollisionSensor::ConvexVsSphere(*c_obj_0, *sph, &m_keep);
			}
		}
	}
}

void ControlState3::UpdateUI(float)
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

	m_forwardPath->RenderScene(m_sceneGraph);
}

void ControlState3::RenderPair(Renderer* renderer)
{
	m_controlled_0->Render(renderer);
	m_controlled_1->Render(renderer);
}
