#pragma once

#include "Engine/Core/Util/XmlUtilities.hpp"

#include <string>
#include <vector>

enum AnimMode
{
	ANIM_LOOP,
	ANIM_CLAMP,
	NUM_OF_ANIM_MODE
};


struct FrameInfo
{
	std::string m_src;
	float m_time;
};

class IsoAnim
{
private:
	bool m_complete;
	float m_duration;
	float m_timeline;
	std::string m_id;
	AnimMode m_mode;
	std::vector<FrameInfo> m_frameInfo;
	
public:
	IsoAnim();
	~IsoAnim();

	void PopulateDataWithElement(const XMLElement& ele);
	void SelectAnimMode(std::string mode);

	std::string GetID() const { return m_id; }
	std::string GetIsoSpriteNameWithTimeline();
	void SetDuration();

	void Update(float deltaSeconds);
	void UpdateTimeline(float deltaSeconds);
};