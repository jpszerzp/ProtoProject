
#include "Engine/Core/Primitive/Box.hpp"

Box::Box()
{

}


Box::~Box()
{
	delete m_renderable;
	m_renderable = nullptr;
}


void Box::Update(float deltaTime)
{

}