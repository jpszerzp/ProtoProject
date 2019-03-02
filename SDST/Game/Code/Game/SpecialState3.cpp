/*
#include "Game/SpecialState3.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Physics/3D/RF/CollisionDetector.hpp"

SpecialState3::SpecialState3()
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

	// ui
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	float txtHeight = height / 70.f;
	Vector2 titleMin = Vector2(-width/ 2.f, height / 2.f - txtHeight);

	std::string title_ui = Stringf("Welcome to special setup state");
	Mesh* t_mesh = Mesh::CreateTextImmediate(Rgba::WHITE, titleMin, font, txtHeight, .5f, title_ui, VERT_PCU);
	m_title_ui = t_mesh;
	titleMin -= Vector2(0.f, txtHeight);

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);
}

SpecialState3::~SpecialState3()
{

}

void SpecialState3::Update(float deltaTime)
{
	UpdateKeyboard(deltaTime);
	UpdateMouse(deltaTime);
	UpdateObjects(deltaTime);
	UpdateContacts(deltaTime);
	UpdateDebugDraw(deltaTime);
	UpdateUI(deltaTime);
}

void SpecialState3::UpdateMouse(float deltaTime)
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

void SpecialState3::UpdateKeyboard(float deltaTime)
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

void SpecialState3::UpdateObjects(float deltaTime)
{

}

void SpecialState3::UpdateContacts(float deltaTime)
{
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

void SpecialState3::UpdateDebugDraw(float deltaTime)
{
	DebugRenderUpdate(deltaTime);
}
*/