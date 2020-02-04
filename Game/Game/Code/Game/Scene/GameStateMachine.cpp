#include "Game/Scene/GameStateMachine.hpp"
#include "Game/Util/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

GameStateMachine::GameStateMachine()
{

}

GameStateMachine::~GameStateMachine()
{
	for each (GameState* state in m_states)
	{
		delete state;
		state = nullptr;
	}

	m_states.clear();
}

void GameStateMachine::Update(float deltaTime)
{
	UpdateInput();
	UpdateState(deltaTime);
}

void GameStateMachine::UpdateInput()
{
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_COMMA) && !DevConsoleIsOpen())
	{
		m_stateIdx++;
		if (m_stateIdx > (m_states.size() - 1U))
		{
			m_stateIdx = 0;
		}

		GameState* state = m_states[m_stateIdx];
		SetCurrentState(state);
	}

	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_OEM_PERIOD) && !DevConsoleIsOpen())
	{
		m_stateIdx--;
		if (m_stateIdx < 0)
		{
			m_stateIdx = (int)(m_states.size() - 1U);
		}

		GameState* state = m_states[m_stateIdx];
		SetCurrentState(state);
	}
}

void GameStateMachine::UpdateState(float deltaTime)
{
	m_currentState->Update(deltaTime);
}

void GameStateMachine::Render(Renderer* renderer)
{
	m_currentState->Render(renderer);
}

void GameStateMachine::SetStateIndex(int idx)
{
	m_stateIdx = idx;
}

void GameStateMachine::AppendState(GameState* state)
{
	m_states.push_back(state);
}

void GameStateMachine::SetCurrentState(GameState* state)
{
	// if the input state is nullptr, meaning that user does not know which state to use
	// just use the default state as the current state
	if (state == nullptr)
	{
		if (m_defaultState != nullptr)
		{
			m_currentState = m_defaultState;
			return;
		}
	}

	m_currentState = state;
	// state idx updated already
}

void GameStateMachine::SetDefaultState(GameState* state)
{
	m_defaultState = state;
}

