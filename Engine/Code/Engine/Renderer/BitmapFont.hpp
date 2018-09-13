#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include <string>

class BitmapFont
{
	friend class Renderer;

public:
	AABB2 GetUVsForGlyph( int glyphUnicode ) const; // pass ‘A’ or 65 for A, etc.
	float GetGlyphAspect( /*int glyphUnicode*/ ) const { return m_baseAspect; } // will change later
	float GetStringWidth( const std::string& asciiText, float cellHeight, float aspectScale );
	SpriteSheet GetSpriteSheet () const { return m_spriteSheet; }

private:
	explicit BitmapFont( const std::string& fontName, const SpriteSheet& glyphSheet, float baseAspect ); // private, can only called by Renderer (friend class)

private:
	const std::string& m_name;
	const SpriteSheet& m_spriteSheet; // used internally; assumed to be a 16x16 glyph sheet
	float m_baseAspect = 1.0f; // used as the base aspect ratio for all glyphs
};