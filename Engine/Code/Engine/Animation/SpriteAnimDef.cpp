#include "Engine/Animation/SpriteAnimDef.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimDef::SpriteAnimDef(const XMLElement& element)
{
	m_name = ParseXmlAttribute(element, "name", "");
	std::string indexes = ParseXmlAttribute(element, "spriteIndexes", "");
	m_fps = ParseXmlAttribute(element, "fps", 0);
	m_frames = SetFromText(indexes.c_str());
	m_numFrames = static_cast<int>(m_frames.size());
}


SpriteAnimDef::~SpriteAnimDef()
{

}