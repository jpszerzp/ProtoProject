#include "Game/TheApp.hpp"
#include "Game/Scene/StateMachine.hpp"
#include "Game/Scene/AIPrototype2D.hpp"
#include "Game/Test/TransformTest.hpp"
#include "Game/Test/MathTest.hpp"
#include "Game/Test/DelegateTest.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/Command.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

TheApp::TheApp()
{
	TimeStartup();
	RendererStartup();
	InputSystemStartup();
	StateStartup();
	ConsoleStartup();
	BlackboardStartup();
}

TheApp::~TheApp()
{
	delete g_config_blackboard;
	g_config_blackboard = nullptr;

	DevConsole::DestroyConsole();

	delete g_theGame;
	g_theGame = nullptr;

	InputSystem::DestroyInstance();

	Renderer::DestroyInstance();

	delete g_masterClock;
	g_masterClock = nullptr;
}

void TheApp::SetInstantFPS()
{
	float theFps = 1.f / m_deltaSeconds;
	g_theGame->SetFPS(theFps);
}

void TheApp::SetDelayedFPS()
{
	m_accTimer += m_deltaSeconds;
	m_frames++;

	if (m_accTimer >= 1.f)
	{
		float theFps = static_cast<float>(m_frames) / m_accTimer;
		g_theGame->SetFPS(theFps);				
		m_accTimer = 0.f;
		m_frames = 0;
	}
}

void TheApp::Update()
{
	UpdateTime();

	g_input->Update();
	ProcessInput();

	g_theGame->Update();

	DevConsole* console = DevConsole::GetInstance();
	console->Update(g_input, m_deltaSeconds);
	if (console->GetAppShouldQuit())
	{
		OnQuitRequested();
	}
}

void TheApp::UpdateTime()
{
	m_deltaSeconds = g_masterClock->frame.seconds;

	g_theGame->SetDeltaTime(m_deltaSeconds);
}

void TheApp::Render()
{
	g_theGame->Render(g_renderer);

	DevConsole* console = DevConsole::GetInstance();
	console->Render(g_renderer);
}

void TheApp::RunFrame()
{
	ClockSystemBeginFrame();

	g_renderer->BeginFrame();
	g_input->BeginFrame();

	Update();
	Render();

	g_input->EndFrame();
	g_renderer->EndFrame();

	//Sleep(1);
	//::SwitchToThread();
}

void TheApp::OnQuitRequested()
{
	m_isQuitting = true;
}

void TheApp::ProcessInput()
{
	// exit of dev console
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_ESC))
	{
		if (!DevConsoleIsOpen())
		{
			OnQuitRequested();
		}
	}

	// dev console
	else if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_DOT_WAVE))
	{
		if (!DevConsoleIsOpen())
		{
			DevConsole* console = DevConsole::GetInstance();
			console->Open();

			g_input->MouseLockCursor(false);
		}
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_M) /*&& !IsProfilerOn()*/)
	{
		if (g_input->m_mouseLock)
		{
			g_input->MouseShowCursor(true);
			g_input->MouseLockCursor(false);

			g_input->m_mouseLock = false;
		}
		else
		{
			g_input->MouseShowCursor(false);
			g_input->MouseLockCursor(true);

			g_input->m_mouseLock = true;
		}
	}
}

void TheApp::TimeStartup()
{
	// master clock forced to be null
	g_masterClock = new Clock();
	g_masterClock->SetParent(nullptr);
	m_accTimer = 0.f;
	m_frames = 0;
}

void TheApp::RendererStartup()
{
	g_renderer = Renderer::GetInstance();
}

void TheApp::InputSystemStartup()
{
	g_input = InputSystem::GetInstance();
	g_input->MouseShowCursor(false);
	g_input->MouseLockCursor(true);
}

void TheApp::StateStartup()
{
	AIPrototypeState* AIPropState = new AIPrototypeState();
	
	// the game-wise state machine
	StateMachine* states = new StateMachine();

	states->AppendState(AIPropState);

	g_theGame = new TheGame();
	g_theGame->SetStateMachine(states);
	g_theGame->UseDefaultState(AIPropState);
	g_theGame->UseGameState(nullptr);
}

void TheApp::ConsoleStartup()
{
	DevConsole* theConsole = DevConsole::GetInstance();
	theConsole->RegisterConsoleHandler();
	theConsole->SetFont(g_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png"));
	theConsole->ConfigureMeshes();
	CommandStartup();
}

void TheApp::BlackboardStartup()
{
	XMLDocument gameConfigDoc;
	gameConfigDoc.LoadFile("Data/GameConfig.xml");
	g_config_blackboard = new Blackboard();
	g_config_blackboard->PopulateFromXmlElementAttributes(*(gameConfigDoc.FirstChildElement()));
}