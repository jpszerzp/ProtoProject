#include "Engine/Renderer/Sprite.hpp"


Sprite::Sprite()
{

}


Sprite::Sprite(std::string id) : m_name(id)
{
	m_scale = Vector2(1.f, 1.f);
}


Sprite::~Sprite()
{
	
}