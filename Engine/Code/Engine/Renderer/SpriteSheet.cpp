#include "Engine/Renderer/SpriteSheet.hpp"

SpriteSheet::SpriteSheet()
{
	
}


SpriteSheet::~SpriteSheet()
{

}


SpriteSheet::SpriteSheet(Texture* texture,IntVector2 layout)
	: m_texture(texture), m_layout(layout)
{

}


AABB2 SpriteSheet::GetTexCoordFromLayout(const IntVector2& layout) const
{
	float minX = 1.f * (static_cast<float>(layout.x) / static_cast<float>(m_layout.x));
	float minY = 1.f * (static_cast<float>(layout.y) / static_cast<float>(m_layout.y));

	float maxX = ( 1.f / static_cast<float>(m_layout.x) ) + minX;
	float maxY = ( 1.f / static_cast<float>(m_layout.y) ) + minY;

	Vector2 min = Vector2(minX, minY);
	Vector2 max = Vector2(maxX, maxY);

	AABB2 texBound = AABB2(min, max);
	return texBound;
}


AABB2 SpriteSheet::GetTexCoordFromLayoutReversed(const IntVector2& layout) const
{
	int reversedY = m_layout.y - layout.y - 1;
	IntVector2 reversedLayout = IntVector2(layout.x, reversedY);

	AABB2 texBound = GetTexCoordFromLayout(reversedLayout);
	return texBound;
}