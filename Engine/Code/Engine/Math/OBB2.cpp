#include "Engine/Math/OBB2.hpp"

Vector2 OBB2::GenerateLeftNormal() const
{
	Vector2 leftNormal = Vector2::MakeDirectionAtDegrees(m_rotation + 180.f);
	return leftNormal.GetNormalized();
}

Vector2 OBB2::GenerateRightNormal() const
{
	Vector2 rightNormal = Vector2::MakeDirectionAtDegrees(m_rotation);
	return rightNormal.GetNormalized();
}

Vector2 OBB2::GenerateUpNormal() const
{
	Vector2 upNormal = Vector2::MakeDirectionAtDegrees(m_rotation + 90.f);
	return upNormal.GetNormalized();
}

Vector2 OBB2::GenerateDownNormal() const
{
	Vector2 downNormal = Vector2::MakeDirectionAtDegrees(m_rotation + 270.f);
	return downNormal.GetNormalized();
}

Vector2 OBB2::GenerateBL() const
{
	Vector2 leftNorm = GenerateLeftNormal();
	Vector2 left = leftNorm * (m_scale.x / 2.f);

	Vector2 botNorm = GenerateDownNormal();
	Vector2 down = botNorm * (m_scale.y / 2.f);

	return (m_center + left + down);
}

Vector2 OBB2::GenerateBR() const
{
	Vector2 rightNorm = GenerateRightNormal();
	Vector2 right = rightNorm * (m_scale.x / 2.f);

	Vector2 botNorm = GenerateDownNormal();
	Vector2 down = botNorm * (m_scale.y / 2.f);

	return (m_center + right + down);
}

Vector2 OBB2::GenerateTL() const
{
	Vector2 leftNorm = GenerateLeftNormal();
	Vector2 left = leftNorm * (m_scale.x / 2.f);

	Vector2 topNorm = GenerateUpNormal();
	Vector2 up = topNorm * (m_scale.y / 2.f);

	return (m_center + left + up);
}

Vector2 OBB2::GenerateTR() const
{
	Vector2 rightNorm = GenerateRightNormal();
	Vector2 right = rightNorm * (m_scale.x / 2.f);

	Vector2 topNorm = GenerateUpNormal();
	Vector2 up = topNorm * (m_scale.y / 2.f);

	return (m_center + right + up);
}

AABB2 OBB2::GenerateAABB() const
{
	Vector2 leftNormal = GenerateLeftNormal();
	Vector2 upNormal = GenerateUpNormal();
	Vector2 rightNormal = GenerateRightNormal();
	Vector2 downNormal = GenerateDownNormal();

	Vector2 LN = leftNormal.RotateDegree2D(-m_rotation).GetNormalized();
	Vector2 UN = upNormal.RotateDegree2D(-m_rotation).GetNormalized();
	Vector2 RN = rightNormal.RotateDegree2D(-m_rotation).GetNormalized();
	Vector2 DN = downNormal.RotateDegree2D(-m_rotation).GetNormalized();

	Vector2 min = m_center + LN * (m_scale.x / 2.f) + DN * (m_scale.y / 2.f);
	Vector2 max = m_center + RN * (m_scale.x / 2.f) + UN * (m_scale.y / 2.f);

	return AABB2(min, max);
}

bool OBB2::IsPointInside(const Vector2& point) const
{
	// get four corners
	Vector2 bl = GenerateBL();
	Vector2 br = GenerateBR();
	Vector2 tl = GenerateTL();
	Vector2 tr = GenerateTR();

	float distanceToLeft = point.GetDistanceToLineSegment(bl, tl);
	float distanceToRight = point.GetDistanceToLineSegment(tr, br);
	if ((distanceToLeft + distanceToRight) > m_scale.x)
	{
		return false;
	}

	float distanceToUp = point.GetDistanceToLineSegment(tl, tr);
	float distanceToDown = point.GetDistanceToLineSegment(bl, br);
	if ((distanceToDown + distanceToUp) > m_scale.y)
	{
		return false;
	}

	return true;
}

OBB2::OBB2()
{

}

OBB2::OBB2(Vector2 center, float rotation, Vector2 scale)
{
	m_center = center;
	m_rotation = rotation;
	m_scale = scale;
}

OBB2::~OBB2()
{

}