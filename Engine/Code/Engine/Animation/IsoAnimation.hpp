#pragma once

#include "Engine/Core/Util/XmlUtilities.hpp"

#include <string>
#include <vector>

enum LoopMode
{
	LOOPMODE_DEFAULT,
	LOOPMODE_CLAMP,
	NUM_LOOPMODE
};

struct AnimationFrameInfo
{
	std::string m_src;
	float m_time;
};

class IsoAnimation
{
private:
	float m_duration;
	std::string m_id;
	LoopMode m_mode;
	std::vector<AnimationFrameInfo> m_frameInfo;

public:
	IsoAnimation();
	~IsoAnimation();

	void PopulateDataWithElement(const XMLElement& ele); 
	void SelectLoopMode(std::string mode);
	void SetDuration();

	float GetDuration() const { return m_duration; }
	std::string GetID() const { return m_id; }
	std::string Evaluate(float timeline);
	LoopMode GetMode() const { return m_mode; }
};