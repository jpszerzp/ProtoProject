#include "Game/Scene/ProtoState.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Primitive/Box.hpp"

PrototypeState::PrototypeState()
{
	Renderer* renderer = Renderer::GetInstance();

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	m_cameraInitialPos = Vector3(0.f, 0.f, -7.f);
	m_cameraRotationSpd = 50.f;

	if (!m_camera)
	{
		m_camera = new Camera(m_cameraInitialPos);
		m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_camera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_camera->SetAspect(aspect);
		m_camera->SetFOV(45.f);

		m_camera->SetProjectionPerspective(m_camera->GetFOV(), aspect, 1.f, 1000.f);
	}

	if (!m_UICamera)
	{
		m_UICamera = new Camera();
		m_UICamera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_UICamera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_UICamera->SetAspect(aspect);

		m_UICamera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

	Vector3 pos = Vector3::ZERO;
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Rgba tint = Rgba::WHITE;
	eMoveStatus stat = MOVE_KINEMATIC;
	eBodyIdentity bid = BODY_RIGID;
	bool multipass = false;
	eDepthCompare compare = COMPARE_LESS;
	eCullMode cull = CULLMODE_BACK;
	eWindOrder wind = WIND_COUNTER_CLOCKWISE;
	Box* box = new Box(pos, rot, scale, tint, "cube_pcu", "default", stat, bid, multipass, compare, cull, wind);
	m_gameObjects.push_back(box);

	Vector2 titleTextMin = Vector2(-width / 2.f, height / 2.f - 32.f);
	float titleHeight = height / 50.f;
	Rgba titleTextTint = Rgba::WHITE;
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	std::string title = "Welcome to ProtoState!";
	m_title_mesh = Mesh::CreateTextImmediate(titleTextTint, titleTextMin, font, titleHeight, 1.f, title, VERT_PCU);
}

PrototypeState::~PrototypeState()
{
	delete m_title_mesh;
	m_title_mesh = nullptr;
}

void PrototypeState::Update(float dt)
{
	UpdateKeyboard(dt);
	UpdateMouse(dt);
}

void PrototypeState::UpdateMouse(float dt)
{

}

void PrototypeState::UpdateKeyboard(float dt)
{

}

void PrototypeState::Render(Renderer* renderer)
{
	renderer->SetCamera(m_camera);
	// let's go for the gameobject path instead of the forward rendering path for now...
	for (std::vector<GameObject*>::size_type idx = 0; idx < m_gameObjects.size(); ++idx)
	{
		m_gameObjects[idx]->Render(renderer);
	}

	renderer->SetCamera(m_UICamera);
	renderer->ClearScreen(Rgba::BLACK);

	if (m_title_mesh != nullptr)
	{
		Renderer* renderer = Renderer::GetInstance();
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->DrawMesh(m_title_mesh);
	}
}