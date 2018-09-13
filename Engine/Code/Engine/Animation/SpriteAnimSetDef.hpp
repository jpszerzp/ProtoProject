#pragma once

#include "Engine/Core/Util/XmlUtilities.hpp"
#include "Engine/Animation/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include <map>

class SpriteAnimSetDef
{
private:
	std::map<std::string, SpriteAnimDef*> m_animDefs;
	SpriteSheet* m_sheet;

public:
	SpriteAnimSetDef(const XMLElement& element);
	~SpriteAnimSetDef();

	SpriteAnimDef* GetAnimDef(std::string key) const { return m_animDefs.find(key)->second; }
	SpriteSheet* GetSheet() const { return m_sheet; }
	IntVector2 GetIntTexCoordFromIndex(int index) const;
	std::map<std::string, SpriteAnimDef*>& GetAnimDefsMap() { return m_animDefs; }
	size_t GetMapSize() const { return m_animDefs.size(); }
};