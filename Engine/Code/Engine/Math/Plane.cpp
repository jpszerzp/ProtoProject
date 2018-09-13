#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Entity3.hpp"

void Plane::Translate(Vector3 translation)
{
	// anchor and offset update
	Vector3 anchor = m_entity->GetEntityCenter();
	m_offset = DotProduct(anchor, m_normal.GetNormalized());
}

