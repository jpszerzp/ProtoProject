#include "Game/StateMachine.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

StateMachine::StateMachine()
{

}

StateMachine::~StateMachine()
{
	for each (GameState* state in m_states)
	{
		delete state;
		state = nullptr;
	}

	m_states.clear();
}

void StateMachine::Update(float deltaTime)
{
	UpdateInput();
	UpdateState(deltaTime);
}

void StateMachine::UpdateInput()
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

void StateMachine::UpdateState(float deltaTime)
{
	m_currentState->Update(deltaTime);
}

void StateMachine::Render(Renderer* renderer)
{
	m_currentState->Render(renderer);
}

void StateMachine::SetStateIndex(int idx)
{
	m_stateIdx = idx;
}

void StateMachine::AppendState(GameState* state)
{
	m_states.push_back(state);
}

void StateMachine::SetCurrentState(GameState* state)
{
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

void StateMachine::SetDefaultState(GameState* state)
{
	m_defaultState = state;
}

