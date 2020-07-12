#include "Engine/Core/Primitive/Sphere.hpp"

Sphere::Sphere()
{

}

Sphere::~Sphere()
{
	delete m_renderable;
	m_renderable = nullptr;
}

float Sphere::GetRadius() const
{
	return 0.f;
}

void Sphere::Update(float)
{

}

void Sphere::Render(Renderer* renderer)
{

}