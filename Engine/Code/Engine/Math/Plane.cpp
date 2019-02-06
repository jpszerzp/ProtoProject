#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Entity3.hpp"

void Plane::Translate(Vector3 translation)
{
	// anchor and offset update
	Vector3 anchor = m_entity->GetEntityCenter();
	m_offset = DotProduct(anchor, m_normal.GetNormalized());
}

//bool Plane::operator<(const Plane& pl) const
//{
//	bool offset_smaller = m_offset < pl.m_offset;
//	bool normal_smaller_x = (m_normal.x < pl.m_normal.x);
//	bool normal_smaller_y = (m_normal.y < pl.m_normal.y);
//	bool normal_smaller_z = (m_normal.z < pl.m_normal.z);
//
//	if (offset_smaller)
//		return true;
//
//	if (normal_smaller_x)
//		return true;
//
//	if (normal_smaller_y)
//		return true;
//
//	if (normal_smaller_z)
//		return true;
//}
//
