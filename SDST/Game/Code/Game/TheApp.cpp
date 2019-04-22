#include "Game/TheApp.hpp"
#include "Game/StateMachine.hpp"
#include "Game/Physics3State.hpp"
#include "Game/Collision3State.hpp"
#include "Game/TransformTest.hpp"
#include "Game/MathTest.hpp"
#include "Game/DelegateTest.hpp"
#include "Game/NetTest.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/Command.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Net/Net.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Physics/3D/RF/PhysTime.hpp"

TheApp::TheApp()
{
	TimeStartup();
	RendererStartup();
	InputSystemStartup();
	AudioSystemStartup();
	NetStartup();
	StateStartup();
	ProfilerStartup();
	ConsoleStartup();
	BlackboardStartup();
	//PhysxStartup();
}


TheApp::~TheApp()
{
	delete g_gameConfigBlackboard;
	g_gameConfigBlackboard = nullptr;

	DevConsole::DestroyConsole();

	Profiler::DestroyInstance();

	delete g_theGame;
	g_theGame = nullptr;

	// destroy rcs
	RCS::DestroyInstance();
	Net::Shutdown();

	AudioSystem::DestroyInstance();

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

	//// If it turns out problematic to put profiler update inside run_frame of app
	//// use game's process_input for input detection instead.
	//// Disable profiler update/update_input accordingly.
	//Profiler* profiler = Profiler::GetInstance();
	//profiler->Update();
}


void TheApp::UpdateTime()
{
	//m_deltaSeconds = g_masterClock->frame.seconds;

	m_deltaSeconds = TimingData::GetTimeDurationSeconds();		// ms to s
	if (m_deltaSeconds <= 0.0f) 
		return;
	else if (m_deltaSeconds > 0.05f)
		m_deltaSeconds = 0.05f;

	g_theGame->SetDeltaTime(m_deltaSeconds);
}


void TheApp::Render()
{
	g_theGame->Render(g_renderer);

	DevConsole* console = DevConsole::GetInstance();
	console->Render(g_renderer);
	
	//// profiler render
	//Profiler* profiler = Profiler::GetInstance();
	//profiler->Render(g_renderer);
}


void TheApp::RunFrame()
{
	//ClockSystemBeginFrame();
	TimingData::Get().UpdateTime();

	g_renderer->BeginFrame();
	g_input->BeginFrame();
	g_audio->BeginFrame();

	Update();
	Render();

	g_audio->EndFrame();
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

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_7) && !DevConsoleIsOpen())
	{
#if defined(PROFILE_ENABLED) 
		Profiler* profiler = Profiler::GetInstance();

		profiler->m_on = !profiler->m_on;
#endif
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_1))
	{
		MathTest::RunMathTest();
		TransformTest::RunTransformTest();
		NetTest::RunNetTest();
		RunDelegateTest();
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_M) && !IsProfilerOn())
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


void TheApp::PlayAudio(std::string clipName)
{
	SoundID testSound = g_audio->CreateOrGetSound( clipName );
	g_audio->PlaySound( testSound );
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

void TheApp::AudioSystemStartup()
{
	g_audio = AudioSystem::GetInstance();
}

void TheApp::NetStartup()
{
	// set up net work
	Net::Startup();

	// set instance of rcs
	g_rcs = RCS::GetInstance();
	g_rcs->Startup();
}

void TheApp::StateStartup()
{
	Physics3State* phys3 = new Physics3State();
	Collision3State* collision = new Collision3State();
	StateMachine* states = new StateMachine();
	phys3->PostConstruct();
	states->AppendState(phys3);
	states->AppendState(collision);
	g_theGame = new TheGame();
	g_theGame->SetStateMachine(states);
	g_theGame->UseDefaultState();			// set default state as state at index 0 
	g_theGame->UseGameState(nullptr);
}

void TheApp::ProfilerStartup()
{
	// set up profiler
	Profiler::GetInstance();
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
	g_gameConfigBlackboard = new Blackboard();
	g_gameConfigBlackboard->PopulateFromXmlElementAttributes(*(gameConfigDoc.FirstChildElement()));
}


void TheApp::PhysxStartup()
{
	//// create physx foundation
	//m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback);
	//if (!m_foundation)
	//	ASSERT_OR_DIE(false, "PxCreateFoundation failed!");

	//// create top-level physx object
	//bool recordMemoryAllocations = true;

	/*
	// optional pvd instance
	m_pvd = PxCreatePvd(*m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	*/

	//m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), recordMemoryAllocations, m_pvd);
	//if (!m_physics)
	//	ASSERT_OR_DIE(false, "PxCreatePhysics failed!");
}
