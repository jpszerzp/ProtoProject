#include "Engine/Animation/IsoAnimation.hpp"

IsoAnimation::IsoAnimation()
{
	m_duration = 0.f;
}


IsoAnimation::~IsoAnimation()
{

}


void IsoAnimation::PopulateDataWithElement(const XMLElement& ele)
{
	m_id = ParseXmlAttribute(ele, "id", "");

	std::string animMode = ParseXmlAttribute(ele, "loop", "");
	SelectLoopMode(animMode);

	for(const XMLElement* theEle = ele.FirstChildElement(); theEle != 0; theEle = theEle->NextSiblingElement())
	{
		std::string val = theEle->Value();
		if (val.compare("frame") == 0)
		{
			std::string frameName = ParseXmlAttribute(*theEle, "src", "");
			float frameTime = ParseXmlAttribute(*theEle, "time", 0.f);

			AnimationFrameInfo frameInfo = AnimationFrameInfo();
			frameInfo.m_src = frameName;
			frameInfo.m_time = frameTime;

			m_frameInfo.push_back(frameInfo);
		}
	}

	SetDuration();
}


void IsoAnimation::SelectLoopMode(std::string mode)
{
	if (mode.compare("loop") == 0)
		m_mode = LOOPMODE_DEFAULT;
	else if (mode.compare("clamp") == 0)
		m_mode = LOOPMODE_CLAMP;
}


std::string IsoAnimation::Evaluate(float timeline)
{
	float timeStamp = 0.f;
	std::string res;

	for (std::vector<AnimationFrameInfo>::size_type frameCount = 0; frameCount < m_frameInfo.size(); ++frameCount)
	{
		AnimationFrameInfo frameInfo = m_frameInfo[frameCount];
		timeStamp += frameInfo.m_time;

		if (timeline <= timeStamp)
		{
			res = frameInfo.m_src;
			break;
		}
	}

	return res;
}


void IsoAnimation::SetDuration()
{
	for (std::vector<AnimationFrameInfo>::size_type frameCount = 0; frameCount < m_frameInfo.size(); ++frameCount)
	{
		AnimationFrameInfo frameInfo = m_frameInfo[frameCount];
		m_duration += frameInfo.m_time;
	}
}