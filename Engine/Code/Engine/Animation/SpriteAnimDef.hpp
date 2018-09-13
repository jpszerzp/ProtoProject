#pragma once

#include "Engine/Core/Util/XmlUtilities.hpp"

#include <vector>

class SpriteAnimDef
{
private:
	std::string m_name;
	std::vector<int> m_frames;

	int m_fps;
	int m_numFrames;

public:
	SpriteAnimDef(const XMLElement& element);
	~SpriteAnimDef();

	std::string GetName() const { return m_name; }
	int GetFrame(int index) const { return m_frames[index]; }
	int GetFrameNum() const { return m_numFrames; }
	int GetFPS() const { return m_fps; }
};