#include "Engine/Core/Primitive/Quad.hpp"

Quad::Quad()
{

}

Quad::~Quad()
{
	delete m_renderable;
	m_renderable = nullptr;
}

void Quad::Update(float deltaTime)
{

}

void Quad::Render(Renderer* renderer)
{

}