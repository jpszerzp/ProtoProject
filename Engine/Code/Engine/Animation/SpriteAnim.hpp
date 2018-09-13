#pragma once

#include "Engine/Core/Util/XmlUtilities.hpp"
#include "Engine/Animation/SpriteAnimDef.hpp"

#include <string.h>

enum AnimPlaymode
{
	ANIM_MODE_PLAY_TO_END,	// Play from time=0 to durationSeconds, then finish
	ANIM_MODE_LOOPING,		// Play from time=0 to end then repeat (never finish)
	//ANIM_MODE_PINGPONG, 	// optional, play forwards, backwards, forwards...
	ANIM_NUM_SPRITE_ANIM_MODES
};


class SpriteAnim
{
private:
	float m_duration;
	float m_secondsElapsed;
	float m_secondsPerFrame;
	float m_perFrameCountdown;

	int m_startFrame;
	int m_currFrame;
	int m_currFrameIndex;

	bool m_isPaused;
	bool m_isFinished;
	bool m_isPlaying;

	SpriteAnimDef* m_def;
	AnimPlaymode m_mode;

public:
	SpriteAnim(SpriteAnimDef* def, AnimPlaymode mode);
	~SpriteAnim();

	void Update(float deltaSeconds);

	void ResetAnim();

	void SetMode(AnimPlaymode mode) { m_mode = mode; }

	int GetCurrentFrame() const { return m_currFrame; }
	bool IsFinished() const { return m_isFinished; }
	SpriteAnimDef* GetDef() const { return m_def; }
};