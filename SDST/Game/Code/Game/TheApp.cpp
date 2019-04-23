#include "Game/TheApp.hpp"
#include "Game/StateMachine.hpp"
#include "Game/Physics3State.hpp"
#include "Game/Collision3State.hpp"
#include "Game/ControlState3.hpp"
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

std::vector<PxVec3> gContactPositions;
std::vector<PxVec3> gContactImpulses;

class ContactReportCallback: public PxSimulationEventCallback
{
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)	{ PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count)					{ PX_UNUSED(pairs); PX_UNUSED(count); }
	void onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) 
	{
		PX_UNUSED((pairHeader));
		std::vector<PxContactPairPoint> contactPoints;

		for(PxU32 i=0;i<nbPairs;i++)
		{
			PxU32 contactCount = pairs[i].contactCount;
			if(contactCount)
			{
				contactPoints.resize(contactCount);
				pairs[i].extractContacts(&contactPoints[0], contactCount);

				for(PxU32 j=0;j<contactCount;j++)
				{
					gContactPositions.push_back(contactPoints[j].position);
					gContactImpulses.push_back(contactPoints[j].impulse);
				}
			}
		}
	}
};

ContactReportCallback gContactReportCallback;

PhysAllocator gAllocator;
PhysErrorCallback gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene		= NULL;

PxMaterial*				gMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxReal stackZ = 10.0f;

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
	PhysxStartup();
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

	PhysxShutdown();
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

	// If it turns out problematic to put profiler update inside run_frame of app
	// use game's process_input for input detection instead.
	// Disable profiler update/update_input accordingly.
	Profiler* profiler = Profiler::GetInstance();
	profiler->Update();
}


void TheApp::UpdateTime()
{
	//m_deltaSeconds = g_masterClock->frame.seconds;

	PhysTimeSystem& time = PhysTimeSystem::GetTimeSystem();
	m_deltaSeconds = time.GetTimeDurationSeconds();			// ms to s
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
	
	// profiler render
	Profiler* profiler = Profiler::GetInstance();
	profiler->Render(g_renderer);
}


void TheApp::RunFrame()
{
	//ClockSystemBeginFrame();
	PhysTimeSystem& time = PhysTimeSystem::GetTimeSystem();
	time.UpdateTime();

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
	g_physState = phys3;
	phys3->PostConstruct();
	
	ControlState3* control = new ControlState3();
	StateMachine* states = new StateMachine();

	states->AppendState(phys3);
	states->AppendState(control);

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
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

void TheApp::PhysxShutdown()
{
	PX_UNUSED(true);
	gScene->release();
	gDispatcher->release();
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();

	gFoundation->release();
}

void TheApp::SpawnPhysxStack(const Vector3& origin, uint sideLength, uint stackHeight)
{
	// plane
	PxRigidStatic* pl = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, -342), *gMaterial);
	gScene->addActor(*pl);

	// interface with my API
	PhysXObject* pl_obj = new PhysXObject(pl);
	m_physx_objs.push_back(pl_obj);
	m_physx_stack.push_back(pl_obj);

	PxVec3 stack_offset = PxVec3(0.f);
	PxVec3 stack_origin = PxVec3(origin.x, origin.y, origin.z);
	PxTransform pxt = PxTransform(stack_origin);
	PxReal half_ext = .5f;

	PxShape* shape = gPhysics->createShape(PxBoxGeometry(half_ext, half_ext, half_ext), *gMaterial);

	for (PxU32 k = 0; k < stackHeight; ++k)
	{
		for (PxU32 i = 0; i < sideLength; ++i)
		{
			// * 2.f is why we use half_ext when setting up local transform
			PxReal stack_x = stack_offset.x + i * 2.f;			
			for (uint j = 0; j < sideLength; ++j)
			{
				PxReal stack_z = stack_offset.z + j * 2.f;
				PxVec3 offset = PxVec3(stack_x, stack_offset.y, stack_z);
				PxTransform local_t(offset * half_ext);

				PxRigidDynamic* body = gPhysics->createRigidDynamic(pxt.transform(local_t));

				body->attachShape(*shape);
				PxRigidBodyExt::updateMassAndInertia(*body, 10.f);

				gScene->addActor(*body);

				PhysXObject* phys_obj = new PhysXObject(body);
				m_physx_objs.push_back(phys_obj);
				m_physx_stack.push_back(phys_obj);
			}
		}

		sideLength--;
		stack_origin = PxVec3(stack_origin.x + .5f, stack_origin.y + 1.f, stack_origin.z + .5f);
		pxt = PxTransform(stack_origin);
	}

	shape->release();
}

void TheApp::PhysxUpdate(float dt)
{
	gContactPositions.clear();
	gContactImpulses.clear();

	// in sample code, this is forced to be 60 fps...
	//deltaTime = 1.f / 60.f;
	gScene->simulate(dt);
	gScene->fetchResults(true);
	DebuggerPrintf("%d contact reports\n", PxU32(gContactPositions.size()));

	PhysxUpdateDelete();
}

void TheApp::PhysxUpdateDelete()
{
	// stack vector has already been emptied, objs are already marked as deleted
	// delete those obj prescence in general vector
	// remove those actors in physx scene
	for (int i = 0; i < m_physx_objs.size(); ++i)
	{
		PhysXObject* phys_obj = m_physx_objs[i];
		if (phys_obj->ShouldDelete())
		{
			// obj vector
			std::vector<PhysXObject*>::iterator it = m_physx_objs.begin() + i;
			m_physx_objs.erase(it);
			// we do not know which vector of physx obj is affected, so we check all
			// still check those in stacks just to be safe
			for (int j = 0; j < m_physx_stack.size(); ++j)
			{
				if (m_physx_stack[j] == phys_obj)
				{
					std::vector<PhysXObject*>::iterator it_del = m_physx_stack.begin() + j;
					m_physx_stack.erase(it_del);
					j--;
				}
			}
			// check those in wraparounds
			g_physState->m_wraparound_demo_0->RemovePhysXObj(phys_obj);
			g_physState->m_wraparound_demo_1->RemovePhysXObj(phys_obj);

			// todo: check those in plane wraparound after they are actually added to it...
			//m_wraparound_plane...

			// check corner case place holder
			if (m_corner_case_3 == phys_obj)
				m_corner_case_3 = nullptr;
			else if (m_corner_case_4 == phys_obj)
				m_corner_case_4 = nullptr;

			// scene actor
			PxRigidActor* ra = phys_obj->GetRigidActor();
			gScene->removeActor(*ra);
			ra->release();

			delete phys_obj;
			i = i - 1;
		}
	}
}

void TheApp::PhysxRender(Renderer* rdr)
{
	for (int i = 0; i < m_physx_objs.size(); ++i)
		m_physx_objs[i]->RenderActor(rdr);
}

PhysXObject* TheApp::SpawnPhysxBox(const Vector3& pos)
{
	PxVec3 pxp = PxVec3(pos.x, pos.y, pos.z);
	PxTransform pxt = PxTransform(pxp);
	PxReal half_ext = .5f;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(half_ext, half_ext, half_ext), *gMaterial);

	// offset
	PxVec3 offset = PxVec3(0.f, 0.f, 0.f);
	PxTransform local = PxTransform(offset);

	// dynamic rigidbody
	PxRigidDynamic* body = gPhysics->createRigidDynamic(pxt.transform(local));
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 1.f);
	gScene->addActor(*body);

	// encapsulation
	PhysXObject* px_obj = new PhysXObject(body);
	m_physx_objs.push_back(px_obj);

	// shape release
	shape->release();
	return px_obj;
}
