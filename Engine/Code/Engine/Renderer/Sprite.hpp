#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"

class Sprite
{
private:
	IntVector2 m_uv;
	Vector2 m_scale;
	AABB2 m_offset;
	std::string m_name;
	Texture* m_texture;

public:
	Sprite();
	Sprite(std::string id);
	~Sprite();

	void SetUV(IntVector2 uv) { m_uv = uv; }
	void SetScale(Vector2 scale) { m_scale = scale; }
	void SetTexture(Texture* tex) {m_texture = tex;}
	void SetOffset(AABB2 offset) {m_offset = offset;}

	Texture* GetTexture() const { return m_texture; }
	IntVector2 GetUV() const { return m_uv; }
	Vector2 GetScale() const { return m_scale; }
	AABB2 GetOffset() const { return m_offset; }
	std::string GetSpriteName() const { return m_name; }
};