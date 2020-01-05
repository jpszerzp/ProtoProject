#pragma once

#include "Game/TheGame.hpp"
#include "Game/Util/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

class TheApp
{
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_slowed = false;

	float m_deltaSeconds;
	float m_accTimer;
	int m_frames;

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

	// module startups
	void TimeStartup();
	void RendererStartup();
	void InputSystemStartup();
	void StateStartup();
	void ProfilerStartup();
	void ConsoleStartup();
	void BlackboardStartup();
};

extern TheApp* g_theApp;