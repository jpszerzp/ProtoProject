#pragma once

#include "Game/TheGame.hpp"
#include "Game/Util/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

class TheApp
{
private:
	bool m_quitting = false;

	TheGame* game;

public:
	TheApp();
	~TheApp();

	void RunFrame();
	void BeginFrame();
	void EndFrame();

	void Update();
	void UpdateTime();
	void ProcessInput();
	void OnQuitRequested();

	void Render();

	// module startups
	void TimeStartup();
	void RendererStartup();
	void InputSystemStartup();
	void ConsoleStartup();
	void BlackboardStartup();

	/*
	 * Function that inserts all game states into the state machine
	 */
	void StateStartup();

	// module shutdowns
	void TimeShutdown();
	void RendererShutdown();
	void InputSystemShutdown();
	void StateShutdown();
	void ConsoleShutdown();
	void BlackboardShutdown();

	bool IsQuitting() { return m_quitting; }
	void SetIsQuitting(bool value) { m_quitting = value; }
};

extern TheApp* g_theApp;