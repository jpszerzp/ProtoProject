#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"

Vector3 AABB3::GetDimensions() const
{
	float x = m_max.x - m_min.x;
	float y = m_max.y - m_min.y;
	float z = m_max.z - m_min.z;

	return Vector3(x, y, z);
}

Vector3 AABB3::GetHalfDimensions() const
{
	return GetDimensions() / 2.f;
}

Vector3 AABB3::GetCenter() const
{
	return (m_max + m_min) / 2.f;
}

void AABB3::GetVertices(Vector3* out_vertices) const
{
	//static Vector3 vertices[8];

	Vector3 center = GetCenter();
	Vector3 halfdim = GetDimensions() / 2.f;
	float halfwidth = halfdim.x;
	float halfheight = halfdim.y;
	float halfdepth = halfdim.z;

	out_vertices[0] = center + (-halfdim);
	out_vertices[1] = center + Vector3(-halfwidth, -halfheight, halfdepth);
	out_vertices[2] = center + Vector3(-halfwidth, halfheight, -halfdepth);
	out_vertices[3] = center + Vector3(-halfwidth, halfheight, halfdepth);
	out_vertices[4] = center + Vector3(halfwidth, -halfheight, -halfdepth);
	out_vertices[5] = center + Vector3(halfwidth, -halfheight, halfdepth);
	out_vertices[6] = center + Vector3(halfwidth, halfheight, -halfdepth);
	out_vertices[7] = center + halfdim;
}

void AABB3::GetEdges(Vector3*) const
{

}

void AABB3::Translate(Vector3 translation)
{
	m_min += translation;
	m_max += translation;
}

void AABB3::SetCenter(const Vector3& center)
{
	Vector3 halfDim = (m_max - m_min) / 2.f;

	m_max = center + halfDim;
	m_min = center - halfDim;
}

/*
 * Project half dimension of aabb3 to the axis
 * @param axis: axis to project onto
 * @return absolute value of the projection
 */
float AABB3::ProjectJHalfDimToAxisAbs(const Vector3& axis) const
{
	Vector3 halfExtent = (m_max - m_min) / 2.f;

	Vector3 normalizedAxis = axis.GetNormalized();

	float projection = abs(DotProduct(halfExtent, normalizedAxis));

	return projection;
}

/*
 * See if the two aabb3 overlap on the given axis. If they overlap, record the amount.
 * @param box1: first aabb3, box2: second abb3, axis: axis to test overlap, overlap: reference to amount of overlap
 * @return whether or not the two boxes overlap on the axis
 */
bool OverlapOnAxis(const AABB3& box1, const AABB3& box2, const Vector3& axis, float& overlap)
{
	// get projection of both boxes onto the axis
	float unsignedBox1Projection = box1.ProjectJHalfDimToAxisAbs(axis);
	float unsignedBox2Projection = box2.ProjectJHalfDimToAxisAbs(axis);

	// compute center displacement/distance, store it
	Vector3 centerDisp = box1.GetCenter() - box2.GetCenter();
	float distance = abs(DotProduct(centerDisp, axis.GetNormalized()));
	float signedOverlap = distance - (unsignedBox1Projection + unsignedBox2Projection);
	overlap = abs(signedOverlap);

	return (signedOverlap < 0);
}

/*
 * See if the two aabb3 overlap on the given axis
 * @param box1: first aabb3, box2: second abb3, axis: axis to test overlap
 * @return whether or not the two boxes overlap on the axis
 */
bool OverlapOnAxis(const AABB3& box1, const AABB3& box2, const Vector3& axis)
{
	// get projection of both boxes onto the axis
	float unsignedBox1Projection = box1.ProjectJHalfDimToAxisAbs(axis);
	float unsignedBox2Projection = box2.ProjectJHalfDimToAxisAbs(axis);

	// center displacement/distance
	Vector3 centerDisp = box1.GetCenter() - box2.GetCenter();
	float distance = abs(DotProduct(centerDisp, axis.GetNormalized()));

	// check for overlap on this axis
	return (distance < (unsignedBox1Projection + unsignedBox2Projection));
}


bool CollideAABB3(const AABB3& box1, const AABB3& box2)
{
	// Exit with no intersection if separated along an axis
	if (box1.m_max.x < box2.m_min.x || box1.m_min.x > box2.m_max.x) return false;
	if (box1.m_max.y < box2.m_min.y || box1.m_min.y > box2.m_max.y) return false;
	if (box1.m_max.z < box2.m_min.z || box1.m_min.z > box2.m_max.z) return false;

	// Overlapping on all axes means AABBs are intersecting
	return true;
}

Vector3 GetRandomLocationWithin(const AABB3& bound)
{
	float x = GetRandomFloatInRange(bound.m_min.x, bound.m_max.x);
	float y = GetRandomFloatInRange(bound.m_min.y, bound.m_max.y);
	float z = GetRandomFloatInRange(bound.m_min.z, bound.m_max.z);

	return Vector3(x, y, z);
}
