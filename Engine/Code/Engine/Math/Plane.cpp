#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Entity3.hpp"

Plane::Plane(Vector3 normal, float offset)
	: m_normal(normal), m_offset(offset)
{
	// guarantee this normal is normalized
	m_normal.Normalize();
}

void Plane::Translate(Vector3 translation)
{
	// anchor and offset update
	Vector3 anchor = m_entity->GetEntityCenter();
	m_offset = DotProduct(anchor, m_normal.GetNormalized());
}
