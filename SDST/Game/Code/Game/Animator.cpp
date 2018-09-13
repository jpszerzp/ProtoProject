#include "Game/Animator.hpp"
#include "Game/TheGame.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Util/StringUtils.hpp"


Animator::Animator()
{
	m_current_animation_complete = false;
	m_time_into_animation = 0.f;
	m_current_animation = nullptr;
	m_default_sprite = nullptr;
}


Animator::~Animator()
{

}


void Animator::Play(std::string name, LoopMode)
{
	//m_current_animation = g_theGame->GetIsoAnimation(name);
	m_time_into_animation = 0.0f;
	m_current_animation_complete = false;
}


IsoSprite* Animator::GetCurrentIsoSprite()
{
	if (m_current_animation != nullptr) {
		std::string isoSpriteName = m_current_animation->Evaluate(m_time_into_animation);
		//return g_theGame->GetIsoSprite(isoSpriteName);
		return nullptr;
	} else {
		return m_default_sprite; 
	}
}


void Animator::Update(float deltaTime)
{
	if (!m_current_animation_complete)
		m_time_into_animation += deltaTime;

	if (m_time_into_animation >= m_current_animation->GetDuration())
	{
		if (m_current_animation->GetMode() == LoopMode::LOOPMODE_DEFAULT)
		{
			m_time_into_animation = 0.f;
		}
		else if (m_current_animation->GetMode() == LoopMode::LOOPMODE_CLAMP)
		{
			m_time_into_animation = 0.f;
			m_current_animation_complete = true;
		}
	}
}