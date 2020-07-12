#include "Engine/Core/Primitive/Point.hpp"

Point::Point()
{

}


Point::~Point()
{
	delete m_renderable;
	m_renderable = nullptr;
}


void Point::Update(float deltaTime)
{

}


void Point::Render(Renderer* renderer)
{

}

