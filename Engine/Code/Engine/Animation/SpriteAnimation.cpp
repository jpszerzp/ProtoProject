#include "Engine/Animation/SpriteAnimation.hpp"


SpriteAnimation::SpriteAnimation()
{

}


SpriteAnimation::~SpriteAnimation()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}


SpriteAnimation::SpriteAnimation(SpriteSheet* sheet, int startIndex, float durationSeconds, PlayMode playMode) : m_spriteSheet(sheet), m_startTexIndex(startIndex), 
m_durationSeconds(durationSeconds), m_playMode(playMode)
{
	m_currentTexIndex = m_startTexIndex;
	m_numFrames = m_spriteSheet->GetLayout().x * m_spriteSheet->GetLayout().y;
	m_secondsElapsed = 0.f;
	m_secondsPerFrame = m_durationSeconds / m_numFrames;
	m_perFrameCountdown = m_secondsPerFrame;
	m_isFinished = false;
	m_isPlaying = true;
}


AABB2 SpriteAnimation::GetCurrentTexCoord() const
{
	AABB2 texCoord = m_spriteSheet->GetTexCoordFromLayout(GetIntTexCoordFromIndex(m_currentTexIndex));
	return texCoord;
}


IntVector2 SpriteAnimation::GetIntTexCoordFromIndex(int index) const
{
	int x = index % (m_spriteSheet->GetLayout().x);
	int y = index / (m_spriteSheet->GetLayout().y);
	return IntVector2(x, y);
}


void SpriteAnimation::Update(float deltaSeconds)
{
	if (m_isPlaying)
	{
		m_perFrameCountdown -= deltaSeconds;

		switch (m_playMode)
		{
		case SPRITE_ANIM_MODE_PLAY_TO_END:
			if (m_perFrameCountdown < 0.f)
			{
				m_currentTexIndex = m_currentTexIndex + 1;
				m_perFrameCountdown = m_secondsPerFrame;
			}

			if (m_currentTexIndex == (m_numFrames - 1))
			{
				m_isFinished = true;
				m_isPlaying = false;
			}
			break;
		case SPRITE_ANIM_MODE_LOOPING:
			if (m_perFrameCountdown < 0.f)
			{
				m_currentTexIndex = (m_currentTexIndex + 1) % m_numFrames;
				m_perFrameCountdown = m_secondsPerFrame;
			}
			break;
		default:
			break;
		}
	}
}


void SpriteAnimation::Pause()
{
	if (m_isPlaying)
	{
		m_isPlaying = false;
	}
}


void SpriteAnimation::Reset()
{
	m_currentTexIndex = m_startTexIndex;
}


void SpriteAnimation::Resume()
{
	if (!m_isPlaying)
	{
		m_isPlaying = true;
	}
}