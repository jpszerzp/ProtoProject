#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Net/Socket.hpp"


TheGame::TheGame()
{
	InitializeClock();
	InitializeGeneralGameData();
	InitializeBitMapFont();
	InitializeTextures();

	DebugRenderStartup();
	//ProfilerSystemInitialize();

	DevConsole::GetInstance()->ConfigureMeshes();
}


TheGame::~TheGame()
{
	DebugRenderShutdown();
	//ProfilerSystemShutDown();

	delete m_states;
	m_states = nullptr;
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

void TheGame::InitializeGeneralGameData()
{
	m_fps = 0.f;
	m_developerMode = false;
}


void TheGame::InitializeBitMapFont()
{
	g_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
}


void TheGame::InitializeTextures()
{

}


void TheGame::InitializeClock()
{
	m_clock = new Clock();
	
	// set hierarchy
	m_clock->SetParent(g_masterClock);
	g_masterClock->AddChild(m_clock);

	m_deltaTime = 0.f;
}


void TheGame::Update()
{
	PROFILE_LOG_SCOPED_FUNCTION();

	UpdateTime();
	
	m_states->Update(m_deltaTime);

	PassTimeToRenderer();

	// Update rcs
	g_rcs->Update(m_deltaTime);

	// udpate udp test
	UDPTest::GetInstance()->Update();
}


void TheGame::UpdateTime()
{
	m_deltaTime = m_clock->frame.seconds;
}


void TheGame::PassTimeToRenderer()
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->m_timeData.gameTime = m_clock->total.seconds;
}

void TheGame::Render(Renderer* renderer)
{
	m_states->Render(renderer);
}