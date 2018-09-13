#include "Engine/Animation/SpriteAnim.hpp"


SpriteAnim::SpriteAnim(SpriteAnimDef* def, AnimPlaymode mode) : m_def(def), m_mode(mode)
{
	m_secondsPerFrame = 1.f / (m_def->GetFPS());
	m_perFrameCountdown = m_secondsPerFrame;

	m_isPlaying = true;
	m_isFinished = false;
	m_isPaused = false;

	m_currFrameIndex = 0;
	m_startFrame = m_def->GetFrame(m_currFrameIndex);
	m_currFrame = m_startFrame;
}


SpriteAnim::~SpriteAnim()
{
	//delete m_def;
	//m_def = nullptr;
}

void SpriteAnim::Update(float deltaSeconds)
{
	if (m_isPlaying)
	{
		m_perFrameCountdown -= deltaSeconds;

		switch (m_mode)
		{
		case ANIM_MODE_PLAY_TO_END:
			if (m_perFrameCountdown < 0.f)
			{
				m_currFrameIndex = m_currFrameIndex + 1;
				m_currFrame = m_def->GetFrame(m_currFrameIndex);
				m_perFrameCountdown = m_secondsPerFrame;
			}

			if (m_currFrameIndex == (m_def->GetFrameNum() - 1))
			{
				m_isFinished = true;
				m_isPlaying = false;
			}
			break;
		case ANIM_MODE_LOOPING:
			if (m_perFrameCountdown < 0.f)
			{
				m_currFrameIndex = (m_currFrameIndex + 1) % (m_def->GetFrameNum());
				m_currFrame = m_def->GetFrame(m_currFrameIndex);
				m_perFrameCountdown = m_secondsPerFrame;
			}
			break;
		default:
			break;
		}
	}
}


void SpriteAnim::ResetAnim()
{
	m_isPlaying = true;
	m_isFinished = false;
	m_isPaused = false;

	m_currFrameIndex = 0;
	m_currFrame = m_startFrame;
}