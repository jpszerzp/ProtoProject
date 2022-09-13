#include "Game/TheGame.hpp"
#include "Game/Util/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Util/AssetUtils.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

TheGame::TheGame()
{
	InitializeClock();
	InitializeBitMapFont();
	InitializeTextures();

	DebugRenderStartup();

	Renderer* renderer = Renderer::GetInstance();
	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	Vector2 titleTextMin = Vector2(-width / 2.f, height / 2.f - 64.f);
	float titleHeight = height / 50.f;
	Rgba titleTextTint = Rgba::WHITE;
	BitmapFont* font = renderer->CreateOrGetBitmapFont(GetAbsFontPath().c_str());
	std::string fps_str = Stringf("FPS:%f", m_clock->GetFPS());
	m_fps_mesh = Mesh::CreateTextImmediate(titleTextTint, titleTextMin, font, titleHeight, 1.f, fps_str, VERT_PCU);
}

TheGame::~TheGame()
{
	DebugRenderShutdown();

	delete m_states;
	m_states = nullptr;

	delete m_fps_mesh;
	m_fps_mesh = nullptr;
}

void TheGame::UseGameState(GameState* state)
{
	m_states->SetCurrentState(state);
}

void TheGame::UseDefaultState()
{
	// set the default state to be the first state in the vector
	GameState* defaultState = m_states->GetState(0);

	// apply that default state, adjust index correctly;
	m_states->SetDefaultState(defaultState);
	m_states->SetStateIndex(0);
}

void TheGame::UseDefaultState(GameState* defaultState)
{
	m_states->SetDefaultState(defaultState);
}

void TheGame::InitializeBitMapFont()
{
	g_renderer->CreateOrGetBitmapFont(GetAbsFontPath().c_str());
}

void TheGame::InitializeTextures()
{

}

void TheGame::InitializeClock()
{
	// clock hierarchy
	m_clock = new Clock();
	m_clock->SetParent(g_masterClock);
	g_masterClock->AddChild(m_clock);
}

void TheGame::Update()
{
	UpdateTime();
	
	m_states->Update(m_clock->frame.seconds);

	if (m_fps_mesh)
	{
		delete m_fps_mesh;
		Renderer* renderer = Renderer::GetInstance();
		Window* window = Window::GetInstance();
		float width = window->GetWindowWidth();
		float height = window->GetWindowHeight();
		Vector2 titleTextMin = Vector2(-width / 2.f, height / 2.f - 64.f);
		float titleHeight = height / 50.f;
		Rgba titleTextTint = Rgba::WHITE;
		BitmapFont* font = renderer->CreateOrGetBitmapFont(GetAbsFontPath().c_str());
		std::string fps_str = Stringf("FPS:%f", m_clock->GetFPS());
		m_fps_mesh = Mesh::CreateTextImmediate(titleTextTint, titleTextMin, font, titleHeight, 1.f, fps_str, VERT_PCU);
	}
}

void TheGame::UpdateTime()
{
	PassTimeToRenderer();
	//m_dt = m_clock->frame.seconds;
}

void TheGame::PassTimeToRenderer()
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->m_timeData.gameTime = m_clock->total.seconds;
}

void TheGame::Render(Renderer* renderer)
{
	m_states->Render(renderer);

	if (m_fps_mesh != nullptr)
	{
		//Renderer* renderer = Renderer::GetInstance();
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture(GetAbsFontPath());
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->DrawMesh(m_fps_mesh);
	}
}