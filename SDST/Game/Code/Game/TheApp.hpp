#pragma once

#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Net/Socket.hpp"
//#include "Engine/Physics/3D/PHYSX/PhysErrorCallback.hpp"
//#include "Engine/Physics/3D/PHYSX/PhysAllocator.hpp"
//#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"

class TheApp
{
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_slowed = false;

	float m_deltaSeconds;
	float m_accTimer;
	int m_frames;

public:
	//std::vector<PhysXObject*> m_physx_objs;
	//std::vector<PhysXObject*> m_physx_stack;

	//PhysXObject* m_corner_case_3 = nullptr;
	//PhysXObject* m_corner_case_4 = nullptr;

public:
	TheApp();
	~TheApp();

	bool IsQuitting() { return m_isQuitting; }
	bool IsPaused() { return m_isPaused; }
	bool IsSlowed() { return m_slowed; }

	void SetPaused (bool value) { m_isPaused = value; }
	void SetSlowed (bool value) { m_slowed = value; }
	void SetIsQuitting(bool value) { m_isQuitting = value; }
	void SetDeltaSeconds(float value) { m_deltaSeconds = value; }
	void SetInstantFPS();
	void SetDelayedFPS();
	
	void RunFrame();

	void Update();
	void UpdateTime();
	void ProcessInput();
	void OnQuitRequested();

	void Render();

	void PlayAudio(std::string clipName);

	// module startups
	void TimeStartup();
	void RendererStartup();
	void InputSystemStartup();
	void AudioSystemStartup();
	void NetStartup();
	void StateStartup();
	void ProfilerStartup();
	void ConsoleStartup();
	void BlackboardStartup();

	// physx
	//void PhysxStartup();
	//void PhysxShutdown();
	//void SpawnPhysxStack(const Vector3& origin, uint sideLength, uint stackHeight);
	//void PhysxUpdate(float dt);
	//void PhysxUpdateDelete();
	//void PhysxRender(Renderer* rdr);
	//PhysXObject* SpawnPhysxBox(const Vector3& pos);
	//PhysXObject* SpawnPhysxBox(const Vector3& pos, float stat_friction, float dyn_friction, float rest);
};

extern TheApp* g_theApp;