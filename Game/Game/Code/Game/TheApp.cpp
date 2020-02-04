#include "Game/TheApp.hpp"
#include "Game/Scene/GameStateMachine.hpp"
#include "Game/Scene/ProtoState.hpp"
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
	// reverse order to startups
	BlackboardShutdown();
	ConsoleShutdown();
	StateShutdown();
	InputSystemShutdown();
	RendererShutdown();
	TimeShutdown();
}

void TheApp::Update()
{
	g_input->Update();
	ProcessInput();

	game->Update();

	DevConsole* console = DevConsole::GetInstance();
	console->Update(g_input, (float)g_masterClock->GetFPS());
	if (console->GetAppShouldQuit())
	{
		OnQuitRequested();
	}
}

void TheApp::Render()
{
	game->Render(g_renderer);

	DevConsole* console = DevConsole::GetInstance();
	console->Render(g_renderer);
}

void TheApp::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

void TheApp::BeginFrame()
{
	ClockSystemBeginFrame();
	g_renderer->BeginFrame();
	g_input->BeginFrame();
}

void TheApp::EndFrame()
{
	g_input->EndFrame();
	g_renderer->EndFrame();
	//Sleep(1);
	//::SwitchToThread();
}

void TheApp::OnQuitRequested()
{
	m_quitting = true;
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

	// mouse cursor lock
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_M))
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

/*
 * Startups
 */
void TheApp::TimeStartup()
{
	// parent of master clock forced to be null
	g_masterClock = new Clock();
	g_masterClock->SetParent(nullptr);
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
	PrototypeState* ProtoState = new PrototypeState();
	
	// the gamewise state machine
	GameStateMachine* states = new GameStateMachine();

	states->AppendState(ProtoState);

	game = new TheGame();
	game->SetStateMachine(states);
	game->UseDefaultState(ProtoState);
	game->UseGameState(nullptr);
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

/*
 * Shutdowns
 */
void TheApp::TimeShutdown()
{
	delete g_masterClock;
	g_masterClock = nullptr;
}

void TheApp::RendererShutdown()
{
	Renderer::DestroyInstance();
}

void TheApp::InputSystemShutdown()
{
	InputSystem::DestroyInstance();
}

void TheApp::StateShutdown()
{
	delete game;
	game = nullptr;
}

void TheApp::ConsoleShutdown()
{
	DevConsole::DestroyConsole();
}

void TheApp::BlackboardShutdown()
{
	delete g_config_blackboard;
	g_config_blackboard = nullptr;
}