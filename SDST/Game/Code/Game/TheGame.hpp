#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/IsoSprite.hpp"
#include "Engine/Animation/IsoAnim.hpp"
#include "Engine/Animation/IsoAnimation.hpp"
#include "Engine/Net/RemoteCommandService.hpp"
#include "Game/ProtoState.hpp"
#include "Game/PachinkoState.hpp"
#include "Game/GameState.hpp"
#include "Game/StateMachine.hpp"

enum eGameMode
{
	GALLERY,
	MAIN,
	ENCOUNTER,
	NON,
	NUM_OF_MODE
};

class TheGame
{
private:
	float m_fps;
	float m_deltaTime;
	bool m_developerMode;

	StateMachine* m_states;

public:
	Clock* m_clock;

public:
	TheGame();
	~TheGame();

	void UseGameState(GameState* state);
	void UseDefaultState();
	void UseDefaultState(GameState* defaultState);

	void InitializeGeneralGameData();
	void InitializeBitMapFont();
	void InitializeTextures();
	void InitializeClock();

	void Update();
	void UpdateTime();
	
	void PassTimeToRenderer();

	void Render(Renderer* renderer);

	// setters
	void SetFPS(float fps) { m_fps = fps; }
	void SetDeveloperMode (bool value) { m_developerMode = value; }
	void SetStateMachine(StateMachine* states) {m_states = states;}

	// getters
	float			GetFPS() const { return m_fps; }
	bool			GetDeveloperMode() const { return m_developerMode; }
	Clock*			GetClock() { return m_clock; }
	StateMachine*	GetStateMachine() { return m_states; }
};