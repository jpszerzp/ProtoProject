#include "Engine/Animation/SpriteAnimSet.hpp"
#include "Engine/Renderer/Renderer.hpp"


SpriteAnimSet::SpriteAnimSet(SpriteAnimSetDef* def, std::string currAnimName)
{
	m_animSetDef = def;
	m_currentAnim = new SpriteAnim(m_animSetDef->GetAnimDef(currAnimName), AnimPlaymode::ANIM_MODE_LOOPING);
	
	std::map<std::string, SpriteAnimDef*>& mapTemp = m_animSetDef->GetAnimDefsMap();
	for (std::map<std::string, SpriteAnimDef*>::iterator it = mapTemp.begin(); it != mapTemp.end(); ++it)
	{
		SpriteAnim* theAnim = new SpriteAnim(it->second, AnimPlaymode::ANIM_MODE_LOOPING);
		m_namedAnims.emplace(it->first, theAnim);
	}
}


SpriteAnimSet::~SpriteAnimSet()
{
	delete m_currentAnim;
	m_currentAnim = nullptr;
	
	// delete pointers in map
	for(std::map<std::string, SpriteAnim*>::iterator itr = m_namedAnims.begin(); itr != m_namedAnims.end(); itr++)
	{
		delete (itr->second);
		itr->second = nullptr;
	}
	m_namedAnims.clear();
}


void SpriteAnimSet::Update(float deltaSeconds)
{
	m_currentAnim->Update(deltaSeconds);
}


AABB2 SpriteAnimSet::GetCurrentTexCoord() const
{
	int frame = m_currentAnim->GetCurrentFrame();
	IntVector2 spritePos = m_animSetDef->GetIntTexCoordFromIndex(frame);
	AABB2 texCoord = (m_animSetDef->GetSheet())->GetTexCoordFromLayout(spritePos);
	return texCoord;
}