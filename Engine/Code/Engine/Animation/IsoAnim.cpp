#include "Engine/Animation/IsoAnim.hpp"


IsoAnim::IsoAnim()
{
	m_duration = 0.f;
	m_complete = false;
}


IsoAnim::~IsoAnim()
{

}


void IsoAnim::PopulateDataWithElement(const XMLElement& ele)
{
	m_id = ParseXmlAttribute(ele, "id", "");
	
	std::string animMode = ParseXmlAttribute(ele, "loop", "");
	SelectAnimMode(animMode);

	for(const XMLElement* theEle = ele.FirstChildElement(); theEle != 0; theEle = theEle->NextSiblingElement())
	{
		std::string val = theEle->Value();
		if (val.compare("frame") == 0)
		{
			std::string frameName = ParseXmlAttribute(*theEle, "src", "");
			float frameTime = ParseXmlAttribute(*theEle, "time", 0.f);

			FrameInfo frameInfo = FrameInfo();
			frameInfo.m_src = frameName;
			frameInfo.m_time = frameTime;

			m_frameInfo.push_back(frameInfo);
		}
	}

	SetDuration();
}


void IsoAnim::SelectAnimMode(std::string mode)
{
	if (mode.compare("loop") == 0)
		m_mode = ANIM_LOOP;
	else if (mode.compare("clamp") == 0)
		m_mode = ANIM_CLAMP;
}


std::string IsoAnim::GetIsoSpriteNameWithTimeline()
{
	float timeStamp = 0.f;
	std::string res;

	for (std::vector<FrameInfo>::size_type frameCount = 0; frameCount < m_frameInfo.size(); ++frameCount)
	{
		FrameInfo frameInfo = m_frameInfo[frameCount];
		timeStamp += frameInfo.m_time;

		if (m_timeline <= timeStamp)
		{
			res = frameInfo.m_src;
			break;
		}
	}

	return res;
}


void IsoAnim::Update(float deltaSeconds)
{
	UpdateTimeline(deltaSeconds);
}


void IsoAnim::SetDuration()
{
	for (std::vector<FrameInfo>::size_type frameCount = 0; frameCount < m_frameInfo.size(); ++frameCount)
	{
		FrameInfo frameInfo = m_frameInfo[frameCount];
		m_duration += frameInfo.m_time;
	}
}


void IsoAnim::UpdateTimeline(float deltaSeconds)
{
	m_timeline += deltaSeconds;
	
	if (m_timeline >= m_duration)
	{
		if (m_mode == ANIM_LOOP)
		{
			m_timeline = 0.f;
		}
		else if (m_mode == ANIM_CLAMP)
		{
			m_timeline = 0.f;
			m_complete = true;
		}
	}
}