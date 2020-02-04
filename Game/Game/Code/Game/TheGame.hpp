#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/IsoSprite.hpp"
#include "Game/Scene/GameState.hpp"
#include "Game/Scene/GameStateMachine.hpp"

class TheGame
{
private:
	//float m_fps;
	//float m_dt;

	GameStateMachine* m_states;
	Clock* m_clock;
	Mesh* m_fps_mesh = nullptr;

public:
	TheGame();
	~TheGame();

	void UseGameState(GameState* state);
	void UseDefaultState();
	void UseDefaultState(GameState* defaultState);

	void InitializeBitMapFont();
	void InitializeTextures();
	void InitializeClock();

	void Update();
	void UpdateTime();
	
	void PassTimeToRenderer();

	void Render(Renderer* renderer);

	// setters
	void SetStateMachine(GameStateMachine* states) {m_states = states;}

	// getters
	GameStateMachine*	GetStateMachine() { return m_states; }
	//float GetDt() const;
};