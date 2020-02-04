#include "Game/Scene/ProtoState.hpp"
#include "Engine/Renderer/Window.hpp"

PrototypeState::PrototypeState()
{
	Renderer* renderer = Renderer::GetInstance();

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	if (!m_camera)
	{
		m_camera = new Camera();
		m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_camera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_camera->SetAspect(aspect);

		m_camera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

	if (!m_UICamera)
	{
		m_UICamera = new Camera();
		m_UICamera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_UICamera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_UICamera->SetAspect(aspect);

		m_UICamera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

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
	renderer->ClearScreen(Rgba::BLACK);

	DrawImmediateTitles(m_title_mesh);
}

void PrototypeState::DrawImmediateTitles(Mesh* mesh)
{
	if (mesh != nullptr)
	{
		Renderer* renderer = Renderer::GetInstance();
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->DrawMesh(mesh);
	}
}