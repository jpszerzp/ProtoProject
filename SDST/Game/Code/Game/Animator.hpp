#pragma once

#include "Engine/Animation/IsoAnimation.hpp"
#include "Engine/Renderer/IsoSprite.hpp"

class Animator 
{
public:

	Animator();
	~Animator();

	void Play(std::string name, LoopMode loop_override = LOOPMODE_DEFAULT);
	void Update(float deltaTime);

	bool		IsCurrentAnimationComplete() const { return m_current_animation_complete; }
	float		GetTimeIntoAnim() const { return m_time_into_animation; }
	IsoSprite*	GetCurrentIsoSprite();

public:
	bool m_current_animation_complete;
	float m_time_into_animation; 
	IsoAnimation* m_current_animation; 
	IsoSprite* m_default_sprite; 
};
