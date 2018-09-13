#include "Engine/Renderer/BitmapFont.hpp"

BitmapFont::BitmapFont(const std::string& fontName, const SpriteSheet& glyphSheet, float baseAspect)
	: m_name(fontName), m_spriteSheet(glyphSheet), m_baseAspect(baseAspect)
{

}


AABB2 BitmapFont::GetUVsForGlyph( int glyphUnicode ) const 
{
	int v = glyphUnicode / 16;
	int flippedV = (16 - 1) - v;				// we know texture is flipped in Y
	int u = glyphUnicode - (16 * v);			// we know X is not flipped
	IntVector2 uv = IntVector2(u, flippedV);	// we use flipped v, but same u

	AABB2 bounds = m_spriteSheet.GetTexCoordFromLayout(uv);
	return bounds;
}


float BitmapFont::GetStringWidth( const std::string& asciiText, float cellHeight, float aspectScale )
{
	float cellWidth = cellHeight * (m_baseAspect * aspectScale);
	float strWidth = cellWidth * asciiText.length();

	return strWidth;
}
