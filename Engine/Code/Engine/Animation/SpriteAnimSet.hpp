#pragma once

#include "Engine/Core/Util/XmlUtilities.hpp"
#include "Engine/Animation/SpriteAnim.hpp"
#include "Engine/Animation/SpriteAnimSetDef.hpp"

#include <vector>

class SpriteAnimSet
{
private:
	SpriteAnim* m_currentAnim;									
	std::map<std::string, SpriteAnim*>	m_namedAnims;			

	SpriteAnimSetDef* m_animSetDef;

public:
	SpriteAnimSet(SpriteAnimSetDef* def, std::string currAnimName);
	~SpriteAnimSet();

	// getters
	SpriteAnimSetDef* GetDef() const { return m_animSetDef; }
	SpriteAnim* GetNamedAnim(const std::string name) const { return m_namedAnims.find(name)->second; }
	SpriteAnim* GetCurrentAnim() const { return m_currentAnim; }
	AABB2 GetCurrentTexCoord() const;

	// setters
	void SetCurrentAnim(SpriteAnim* anim) { m_currentAnim = anim; }

	void Update(float deltaSeconds);
};