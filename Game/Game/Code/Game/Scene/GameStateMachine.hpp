#pragma once

#include "Game/Scene/GameState.hpp"

class GameStateMachine
{
private:
	int m_stateIdx = 0;
	GameState* m_currentState = nullptr;
	GameState* m_defaultState = nullptr;
	std::vector<GameState*> m_states;

public:
	GameStateMachine();
	~GameStateMachine();

	void Update(float deltaTime);
	void UpdateInput();
	void UpdateState(float deltaTime);

	void Render(Renderer* renderer);

	void SetStateIndex(int idx);
	void SetCurrentState(GameState* state);
	void SetDefaultState(GameState* state);
	void AppendState(GameState* state);

	GameState* GetState(int idx) { return m_states[idx]; }
	GameState* GetCurrentState() { return m_currentState; }
};