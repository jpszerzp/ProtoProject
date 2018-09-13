#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"

class SpriteSheet
{
	IntVector2 m_layout;
	Texture* m_texture;

public:
	SpriteSheet();
	SpriteSheet(Texture* texture, IntVector2 layout);
	~SpriteSheet();

	AABB2 GetTexCoordFromLayout(const IntVector2& layout) const;
	AABB2 GetTexCoordFromLayoutReversed(const IntVector2& layout) const;

	//setters
	void SetLayout(IntVector2 value) { m_layout = value; }

	// getters
	Texture* GetTexture() { return m_texture; }
	IntVector2 GetLayout() { return m_layout; }
};