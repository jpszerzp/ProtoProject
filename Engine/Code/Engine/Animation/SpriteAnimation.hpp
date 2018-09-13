#pragma once

#include "Engine/Renderer/SpriteSheet.hpp"


enum PlayMode 
{
	SPRITE_ANIM_MODE_PLAY_TO_END,	// Play from time=0 to durationSeconds, then finish
	SPRITE_ANIM_MODE_LOOPING,		// Play from time=0 to end then repeat (never finish)
	//SPRITE_ANIM_MODE_PINGPONG, 	// optional, play forwards, backwards, forwards...
	NUM_SPRITE_ANIM_MODES
};


class SpriteAnimation
{
private:
	SpriteSheet* m_spriteSheet;
	int m_startTexIndex;
	int m_currentTexIndex;

	int m_numFrames;
	float m_durationSeconds;
	float m_secondsElapsed;
	float m_perFrameCountdown;
	float m_secondsPerFrame;
	float m_fractionElapsed;

	PlayMode m_playMode;
	bool m_isFinished;
	bool m_isPlaying;

public:
	SpriteAnimation();
	~SpriteAnimation();
	SpriteAnimation(SpriteSheet* sheet, int startIndex, float durationSeconds ,PlayMode playMode);

	// getters
	SpriteSheet* GetSpriteSheet() { return m_spriteSheet; }
	Texture* GetTexture() const { return m_spriteSheet->GetTexture(); }
	AABB2 GetCurrentTexCoord() const;
	IntVector2 GetIntTexCoordFromIndex(int index) const;
	float GetDurationSeconds() const { return m_durationSeconds; }
	float GetSecondsElapsed() const { return m_secondsElapsed; }
	float GetSecondsRemaining() const { return (m_durationSeconds - fmod(m_secondsElapsed, m_durationSeconds)); }
	float GetFractionElapsed() const { return m_fractionElapsed; }
	bool IsFinished() const { return m_isFinished; }
	bool IsPlaying() const { return m_isPlaying; }

	// setters
	void SetSpriteSheet(SpriteSheet* value) { m_spriteSheet = value; }
	void SetFinished(bool value) { m_isFinished = value; }
	void SetSecondsElapsed(float value) { m_secondsElapsed = value; }
	void SetFractionElapsed(float value) { m_fractionElapsed = value; }

	void Update(float deltaSeconds);
	void Pause();
	void Resume();
	void Reset();
};