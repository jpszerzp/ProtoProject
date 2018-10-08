#include "Engine/Math/Sphere3.hpp"

void Sphere3::Translate(Vector3 translation)
{
	m_center += translation;
}

void Sphere3::SetCenter(const Vector3& center)
{
	m_center = center;
}

