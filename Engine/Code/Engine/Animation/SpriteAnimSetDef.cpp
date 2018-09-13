#include "Engine/Animation/SpriteAnimSetDef.hpp"


SpriteAnimSetDef::SpriteAnimSetDef(const XMLElement& element)
{
	std::string image = ParseXmlAttribute(element, "spriteSheet", "");
	IntVector2 layout = ParseXmlAttribute(element, "spriteLayout", IntVector2(0, 0));
	Texture* texture = new Texture("Data/Images/" + image);
	m_sheet = new SpriteSheet(texture, layout);

	for(const XMLElement* ele = element.FirstChildElement(); ele != 0; ele = ele->NextSiblingElement())
	{
		SpriteAnimDef* anim = new SpriteAnimDef(*ele);
		m_animDefs.emplace(anim->GetName(), anim);
	}
}


SpriteAnimSetDef::~SpriteAnimSetDef()
{
	delete m_sheet;
	m_sheet = nullptr;

	for(std::map<std::string, SpriteAnimDef*>::iterator itr = m_animDefs.begin(); itr != m_animDefs.end(); itr++)
	{
		delete (itr->second);
		itr->second = nullptr;
	}
	m_animDefs.clear();
}


IntVector2 SpriteAnimSetDef::GetIntTexCoordFromIndex(int index) const
{
	int x = index % (m_sheet->GetLayout().x);
	int y = index / (m_sheet->GetLayout().y);
	return IntVector2(x, y);
}