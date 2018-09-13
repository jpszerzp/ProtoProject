#include "Engine/Physics/2D/Manifold.hpp"
#include "Engine/Physics/2D/DiscEntity.hpp"
#include "Engine/Physics/2D/QuadEntity.hpp"
#include "Engine/Physics/2D/RectangleEntity.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"


Manifold::Manifold(Entity* e1, Entity* e2)
{
	m_e1 = e1;
	m_e2 = e2;
}

Manifold::~Manifold()
{

}


bool AABB2VsAABB2Manifold(Manifold* manifold)
{
	Entity* e1 = manifold->m_e1;
	Entity* e2 = manifold->m_e2;

	// from 1 to 2
	Vector2 displacement = e2->m_center - e1->m_center;

	QuadEntity* q1 = dynamic_cast<QuadEntity*>(e1);
	QuadEntity* q2 = dynamic_cast<QuadEntity*>(e2);
	if (q1 == nullptr || q2 == nullptr)
	{
		ASSERT_OR_DIE(false, "Assume manifold to hold quad entity, which is not ture");
	}

	AABB2& quad1 = q1->m_primitive;
	AABB2& quad2 = q2->m_primitive;

	Vector2 quad1BL = quad1.mins;
	Vector2 quad1BR = quad1BL + Vector2(quad1.GetDimensions().x, 0.f);
	Vector2 quad1TL = quad1BL + Vector2(0.f, quad1.GetDimensions().y);
	Vector2 quad1TR = quad1.maxs;
	Vector2 quad1Vertices[4] = {quad1BL, quad1TL, quad1TR, quad1BR};

	Vector2 quad2BL = quad2.mins;
	Vector2 quad2BR = quad2BL + Vector2(quad2.GetDimensions().x, 0.f);
	Vector2 quad2TL = quad2BL + Vector2(0.f, quad2.GetDimensions().y);
	Vector2 quad2TR = quad2.maxs;
	Vector2 quad2Vertices[4] = {quad2BL, quad2TL, quad2TR, quad2BR};

	Vector2 leftNormal1 = quad1.leftNormal;
	Vector2 upNormal1 = quad1.upNormal;
	Vector2 rightNormal1 = quad1.rightNormal;
	Vector2 downNormal1 = quad1.downNormal;

	Vector2 leftNormal2 = quad2.leftNormal;
	Vector2 upNormal2 = quad2.upNormal;
	Vector2 rightNormal2 = quad2.rightNormal;
	Vector2 downNormal2 = quad2.downNormal;

	// support point processing of quad 1

	// left normal of quad 1
	float signedDistanceLN1 = INFINITY;
	Vector2 supportPointLN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad2Vertices[i];
		Vector2 blToPt = spCandidate - quad1BL;
		float distance = DotProduct(leftNormal1, blToPt);

		if (distance < signedDistanceLN1)
		{
			signedDistanceLN1 = distance;
			supportPointLN1 = spCandidate;
		}
	}
	if (signedDistanceLN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for LN1 and it is recorded

	// up normal of quad 1
	float signedDistanceUN1 = INFINITY;
	Vector2 supportPointUN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad2Vertices[i];
		Vector2 tlToPt = spCandidate - quad1TL;
		float distance = DotProduct(upNormal1, tlToPt);

		if (distance < signedDistanceUN1)
		{
			signedDistanceUN1 = distance;
			supportPointUN1 = spCandidate;
		}
	}
	if (signedDistanceUN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for UN1 and it is recorded

	// right normal of quad 1
	float signedDistanceRN1 = INFINITY;
	Vector2 supportPointRN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad2Vertices[i];
		Vector2 trToPt = spCandidate - quad1TR;
		float distance = DotProduct(rightNormal1, trToPt);

		if (distance < signedDistanceRN1)
		{
			signedDistanceRN1 = distance;
			supportPointRN1 = spCandidate;
		}
	}
	if (signedDistanceRN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for RN1 and it is recorded

	// down normal of quad 1
	float signedDistanceDN1 = INFINITY;
	Vector2 supportPointDN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad2Vertices[i];
		Vector2 brToPt = spCandidate - quad1BR;
		float distance = DotProduct(downNormal1, brToPt);

		if (distance < signedDistanceDN1)
		{
			signedDistanceDN1 = distance;
			supportPointDN1 = spCandidate;
		}
	}
	if (signedDistanceDN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for DN1 and it is recorded

	Vector2 normal1To2 = Vector2::ZERO;
	float absDist1 = INFINITY;
	float absDistLN1 = abs(signedDistanceLN1);
	float absDistUN1 = abs(signedDistanceUN1);
	float absDistRN1 = abs(signedDistanceRN1);
	float absDistDN1 = abs(signedDistanceDN1);

	if (absDistLN1 < absDist1)
	{
		absDist1 = absDistLN1;
		normal1To2 = leftNormal1;
	}
	if (absDistUN1 < absDist1)
	{
		absDist1 = absDistUN1;
		normal1To2 = upNormal1;
	}
	if (absDistRN1 < absDist1)
	{
		absDist1 = absDistRN1;
		normal1To2 = rightNormal1;
	}
	if (absDistDN1 < absDist1)
	{
		absDist1 = absDistDN1;
		normal1To2 = downNormal1;
	}
	// at this moment we have normal and penetration candidate from quad 1

	// support point processing of quad 2

	// left normal of quad 2
	float signedDistanceLN2 = INFINITY;
	Vector2 supportPointLN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad1Vertices[i];
		Vector2 blToPt = spCandidate - quad2BL;
		float distance = DotProduct(leftNormal2, blToPt);

		if (distance < signedDistanceLN2)
		{
			signedDistanceLN2 = distance;
			supportPointLN2 = spCandidate;
		}
	}
	if (signedDistanceLN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for LN2 and it is recorded

	// up normal of quad 2
	float signedDistanceUN2 = INFINITY;
	Vector2 supportPointUN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad1Vertices[i];
		Vector2 tlToPt = spCandidate - quad2TL;
		float distance = DotProduct(upNormal2, tlToPt);

		if (distance < signedDistanceUN2)
		{
			signedDistanceUN2 = distance;
			supportPointUN2 = spCandidate;
		}
	}
	if (signedDistanceUN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for UN2 and it is recorded

	// right normal of quad 2
	float signedDistanceRN2 = INFINITY;
	Vector2 supportPointRN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad1Vertices[i];
		Vector2 trToPt = spCandidate - quad2TR;
		float distance = DotProduct(rightNormal2, trToPt);

		if (distance < signedDistanceRN2)
		{
			signedDistanceRN2 = distance;
			supportPointRN2 = spCandidate;
		}
	}
	if (signedDistanceRN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for RN2 and it is recorded

	// down normal of quad 2
	float signedDistanceDN2 = INFINITY;
	Vector2 supportPointDN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = quad1Vertices[i];
		Vector2 brToPt = spCandidate - quad2BR;
		float distance = DotProduct(downNormal2, brToPt);

		if (distance < signedDistanceDN2)
		{
			signedDistanceDN2 = distance;
			supportPointDN2 = spCandidate;
		}
	}
	if (signedDistanceDN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for DN2 and it is recorded

	// normal and penetration candidate from quad 2
	Vector2 normal2To1 = Vector2::ZERO;
	float absDist2 = INFINITY;
	float absDistLN2 = abs(signedDistanceLN2);
	float absDistUN2 = abs(signedDistanceUN2);
	float absDistRN2 = abs(signedDistanceRN2);
	float absDistDN2 = abs(signedDistanceDN2);

	if (absDistLN2 < absDist2)
	{
		absDist2 = absDistLN2;
		normal2To1 = leftNormal2;
	}
	if (absDistUN2 < absDist2)
	{
		absDist2 = absDistUN2;
		normal2To1 = upNormal2;
	}
	if (absDistRN2 < absDist2)
	{
		absDist2 = absDistRN2;
		normal2To1 = rightNormal2;
	}
	if (absDistDN2 < absDist2)
	{
		absDist2 = absDistDN2;
		normal2To1 = downNormal2;
	}

	// at this point we have normal and penetration candidate from quad 2

	// compare the candidate 
	if (absDist1 <= absDist2)
	{
		manifold->m_penetration = absDist1;
		manifold->m_normal = normal1To2;
		return true;
	}
	else
	{
		manifold->m_penetration = absDist2;
		manifold->m_normal = normal2To1;
		return true;
	}
}

bool AABB2VsDisc2Manifold(Manifold* manifold)
{
	QuadEntity* quad = dynamic_cast<QuadEntity*>(manifold->m_e1);
	DiscEntity* disc = dynamic_cast<DiscEntity*>(manifold->m_e2);

	if (quad == nullptr)
	{
		ASSERT_OR_DIE(false, "Make sure manifold contains a quad and is entity1");
	}

	if (disc == nullptr)
	{
		ASSERT_OR_DIE(false, "Make sure manifold contains a disc and is entity2");
	}

	// R1R2R3
	// R4  R5
	// R6R7R8

	Disc2 discPrimitive = disc->m_primitive;
	AABB2 quadPrimitive = quad->m_primitive;
	Vector2 discCenter = discPrimitive.center;
	float discRadius = discPrimitive.radius;
	bool inside = quadPrimitive.IsPointInside(discCenter);

	// get vertex of the quad
	Vector2 bl = quadPrimitive.mins;
	Vector2 br = bl + Vector2(quadPrimitive.GetDimensions().x, 0.f);
	Vector2 tl = bl + Vector2(0.f, quadPrimitive.GetDimensions().y);
	Vector2 tr = quadPrimitive.maxs;

	// compute vector from each vertex to interested point
	Vector2 blToPt = discCenter - bl;
	Vector2 brToPt = discCenter - br;
	Vector2 tlToPt = discCenter - tl;
	Vector2 trToPt = discCenter - tr;

	if (!inside)
	{
		// up region
		if (discCenter.y > tl.y)
		{
			// upper left, R1
			if (discCenter.x < tl.x)
			{
				float distance = tlToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -tlToPt.GetNormalized();
					return true;
				}
				return false;
			}
			// upper right, R3
			else if (discCenter.x > tr.x)
			{
				float distance = trToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -trToPt.GetNormalized();
					return true;
				}
				return false;
			}
			// upper middle, R2
			else
			{
				Vector2 tlToTr = tr - tl;
				Vector2 projected = GetProjectedVector(tlToPt, tlToTr);
				Vector2 vertical = tlToPt - projected;
				float distance = vertical.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -vertical.GetNormalized();
					return true;
				}
				return false;
			}
		}
		// bottom region
		else if (discCenter.y < bl.y)
		{
			// bottom left, R6
			if (discCenter.x < tl.x)
			{
				float distance = blToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -blToPt.GetNormalized();
					return true;
				}
				return false;
			}
			// bottom right, R8
			else if (discCenter.x > tr.x)
			{
				float distance = brToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -brToPt.GetNormalized();
					return true;
				}
				return false;
			}
			// bottom middle, R7
			else
			{
				Vector2 blToBr = br - bl;
				Vector2 projected = GetProjectedVector(blToPt, blToBr);
				Vector2 vertical = blToPt - projected;
				float distance = vertical.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -vertical.GetNormalized();
					return true;
				}
				return false;
			}
		}
		// middle region
		else
		{
			// middle left, R4
			if (discCenter.x <= tl.x)
			{
				Vector2 blToTl = tl - bl;
				Vector2 projected = GetProjectedVector(blToPt, blToTl);
				Vector2 vertical = blToPt - projected;
				float distance = vertical.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -vertical.GetNormalized();
					return true;
				}
				return false;
			}
			// middle right, R5
			else if (discCenter.x >= tr.x)
			{
				Vector2 brToTr = tr - br;
				Vector2 projected = GetProjectedVector(brToPt, brToTr);
				Vector2 vertical = brToPt - projected;
				float distance = vertical.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -vertical.GetNormalized();
					return true;
				}
				return false;
			}
			// piont in aabb
			else
			{
				// this will not get called
				return true;
			}
		}
	}
	else
	{
		// we are inside the aabb, we are definitely colliding
		// distance to up edge
		//Vector2 tlToPt = discCenter - tl;
		Vector2 tlToTr = tr - tl;
		Vector2 upToPtVertical = tlToPt - GetProjectedVector(tlToPt, tlToTr);
		float distanceToUp = upToPtVertical.GetLength();

		// distance to bottom edge
		//Vector2 blToPt = discCenter - bl;
		Vector2 blToBr = br - bl;
		Vector2 bottomToPtVertical = blToPt - GetProjectedVector(blToPt, blToBr);
		float distanceToBottom = bottomToPtVertical.GetLength();

		// distance to left edge
		Vector2 blToTl = tl - bl;
		Vector2 leftToPtVertical = blToPt - GetProjectedVector(blToPt, blToTl);
		float distanceToLeft = leftToPtVertical.GetLength();

		// distance to right edge
		//Vector2 brToPt = discCenter - br;
		Vector2 brToTr = tr - br;
		Vector2 rightToPtVertical = brToPt - GetProjectedVector(brToPt, brToTr);
		float distanceToRight = rightToPtVertical.GetLength();

		float penetration = 0.f;
		Vector2 normal = Vector2::ZERO;
		float minDistance = INFINITY;
		if (distanceToUp < minDistance)
		{
			normal = -upToPtVertical.GetNormalized();
			penetration = distanceToUp + discRadius;

			minDistance = distanceToUp;
		}

		if (distanceToBottom < minDistance)
		{
			normal = -bottomToPtVertical.GetNormalized();
			penetration = distanceToBottom + discRadius;

			minDistance = distanceToBottom;
		}

		if (distanceToLeft < minDistance)
		{
			normal = -leftToPtVertical.GetNormalized();
			penetration = distanceToLeft + discRadius;

			minDistance = distanceToLeft;
		}

		if (distanceToRight < minDistance)
		{
			normal = -rightToPtVertical.GetNormalized();
			penetration = distanceToRight + discRadius;

			minDistance = distanceToRight;
		}

		manifold->m_penetration = penetration;
		manifold->m_normal = normal;

		return true;
	}
}

bool DiscVsDiscManifold(Manifold* manifold)
{
	DiscEntity* disc1 = dynamic_cast<DiscEntity*>(manifold->m_e1);
	DiscEntity* disc2 = dynamic_cast<DiscEntity*>(manifold->m_e2);

	if (disc1 == nullptr || disc2 == nullptr)
	{
		ASSERT_OR_DIE(false, "Assume manifold to hold disc entity, which is not true");
	}

	Disc2& d1 = disc1->m_primitive;
	Disc2& d2 = disc2->m_primitive;
	Vector2 d1Center = d1.center;
	Vector2 d2Center = d2.center;
	float d1Rad = d1.radius;
	float d2Rad = d2.radius;
	Vector2 d1Tod2 = d2Center - d1Center;
	float distance = d1Tod2.GetLength();
	float threshold = d1Rad + d2Rad;

	if (distance > threshold)
	{
		return false;
	}
	else
	{
		manifold->m_normal = d1Tod2.GetNormalized();
		manifold->m_penetration = threshold - distance;
		manifold->m_start = d1Center + manifold->m_normal * (distance - d2Rad);
		manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
		return true;
	}
}

bool OBB2VsDiscManifold(Manifold* manifold)
{
	RectangleEntity* rec = dynamic_cast<RectangleEntity*>(manifold->m_e1);
	DiscEntity* disc = dynamic_cast<DiscEntity*>(manifold->m_e2);

	if (rec == nullptr)
	{
		ASSERT_OR_DIE(false, "Make sure manifold contains a rec and is entity1");
	}

	if (disc == nullptr)
	{
		ASSERT_OR_DIE(false, "Make sure manifold contains a disc and is entity2");
	}

	// R1R2R3
	// R4  R5
	// R6R7R8

	Disc2 discPrimitive = disc->m_primitive;
	OBB2 recPrimitive = rec->m_primitive;
	Vector2 discCenter = discPrimitive.center;
	float discRadius = discPrimitive.radius;
	bool inside = recPrimitive.IsPointInside(discCenter);

	// get vertex of the rec
	Vector2 bl = recPrimitive.GenerateBL();
	Vector2 br = recPrimitive.GenerateBR();
	Vector2 tl = recPrimitive.GenerateTL();
	Vector2 tr = recPrimitive.GenerateTR();

	// compute vector from each vertex to center of disc
	Vector2 blToPt = discCenter - bl;
	Vector2 brToPt = discCenter - br;
	Vector2 tlToPt = discCenter - tl;
	Vector2 trToPt = discCenter - tr;

	if (!inside)
	{
		// to tell in which region 
		bool upRegion = false;
		bool middleRegionH = false;
		bool downRegion = false;

		// see if the point is in up region
		Vector2 topEdge = tr - tl;
		Vector2 topPerpendicular = GetPerpendicularVector(tlToPt, topEdge);
		Vector2 topNormal = recPrimitive.GenerateUpNormal();
		float dotUp = DotProduct(topPerpendicular, topNormal);
		if (dotUp > 0.f)
		{
			upRegion = true;
		}

		// see if the point is in down region
		else
		{
			Vector2 bottomEdge = br - bl;
			Vector2 bottomPerpendicular = GetPerpendicularVector(blToPt, bottomEdge);
			Vector2 downNormal = recPrimitive.GenerateDownNormal();
			float dotDown = DotProduct(bottomPerpendicular, downNormal);
			if (dotDown > 0.f)
			{
				downRegion = true;
			}
			else 
			{
				middleRegionH = true;
			}
		}
		// now we know if the point is at up, middle horizontal or bottom region

		bool leftRegion = false;
		bool middleRegionV = false;
		bool rightRegion = false;

		// see if point is in left region
		Vector2 leftEdge = tl - bl;
		Vector2 leftPerpendicular = GetPerpendicularVector(blToPt, leftEdge);
		Vector2 leftNormal = recPrimitive.GenerateLeftNormal();
		float dotLeft = DotProduct(leftPerpendicular, leftNormal);
		if (dotLeft > 0.f)
		{
			leftRegion = true;
		}

		// see if the point is in right region
		else
		{
			Vector2 rightEdge = br - tr;
			Vector2 rightPerpendicular = GetPerpendicularVector(trToPt, rightEdge);
			Vector2 rightNormal = recPrimitive.GenerateRightNormal();
			float dotRight = DotProduct(rightPerpendicular, rightNormal);
			if (dotRight > 0.f)
			{
				rightRegion = true;
			}
			else
			{
				middleRegionV = true;
			}
		}
		// now we know if the point is at left, middle vertical or right region

		// in upper region
		if (upRegion)
		{
			// upper left, R1
			if (leftRegion)
			{
				float distance = tlToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -tlToPt.GetNormalized();
					manifold->m_start = tl;
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
			// upper right, R3
			else if (rightRegion)
			{
				float distance = trToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -trToPt.GetNormalized();
					manifold->m_start = tr;
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
			// upper middle, R2
			else
			{
				Vector2 tlToTr = tr - tl;
				Vector2 perpendicular = GetPerpendicularVector(tlToPt, tlToTr);
				float distance = perpendicular.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -perpendicular.GetNormalized();
					manifold->m_start = discCenter + (-perpendicular);
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
		}
		// bottom region
		else if (downRegion)
		{
			// bottom left, R6
			if (leftRegion)
			{
				float distance = blToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -blToPt.GetNormalized();
					manifold->m_start = bl;
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
			// bottom right, R8
			else if (rightRegion)
			{
				float distance = brToPt.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -brToPt.GetNormalized();
					manifold->m_start = br;
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
			// bottom middle, R7
			else
			{
				Vector2 blToBr = br - bl;
				Vector2 perpendicular = GetPerpendicularVector(blToPt, blToBr);
				float distance = perpendicular.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -perpendicular.GetNormalized();
					manifold->m_start = discCenter + (-perpendicular);
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
		}
		// middle region
		else
		{
			// middle left, R4
			if (leftRegion)
			{
				Vector2 blToTl = tl - bl;
				Vector2 perpendicular = GetPerpendicularVector(blToPt, blToTl);
				float distance = perpendicular.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -perpendicular.GetNormalized();
					manifold->m_start = discCenter + (-perpendicular);
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
			// middle right, R5
			else if (rightRegion)
			{
				Vector2 brToTr = tr - br;
				Vector2 perpendicular = GetPerpendicularVector(brToPt, brToTr);
				float distance = perpendicular.GetLength();
				if (distance < discRadius)
				{
					manifold->m_penetration = discRadius - distance;
					manifold->m_normal = -perpendicular.GetNormalized();
					manifold->m_start = discCenter + (-perpendicular);
					manifold->m_end = manifold->m_start + manifold->m_normal * manifold->m_penetration;
					return true;
				}
				return false;
			}
			// piont in obb
			else
			{
				// this will not get called in the first place
				return true;
			}
		}
	}
	else
	{
		// we are inside the aabb, we are definitely colliding
		// distance to up edge
		Vector2 tlToTr = tr - tl;
		Vector2 upToPtVertical = GetPerpendicularVector(tlToPt, tlToTr);
		float distanceToUp = upToPtVertical.GetLength();

		// distance to bottom edge
		Vector2 blToBr = br - bl;
		Vector2 bottomToPtVertical = GetPerpendicularVector(blToPt, blToBr);
		float distanceToBottom = bottomToPtVertical.GetLength();

		// distance to left edge
		Vector2 blToTl = tl - bl;
		Vector2 leftToPtVertical = GetPerpendicularVector(blToPt, blToTl);
		float distanceToLeft = leftToPtVertical.GetLength();

		// distance to right edge
		Vector2 brToTr = tr - br;
		Vector2 rightToPtVertical = GetPerpendicularVector(brToPt, brToTr);
		float distanceToRight = rightToPtVertical.GetLength();

		float penetration = 0.f;
		Vector2 normal = Vector2::ZERO;
		Vector2 start = Vector2::ZERO;
		Vector2 end = Vector2::ZERO;
		float minDistance = INFINITY;
		if (distanceToUp < minDistance)
		{
			normal = -upToPtVertical.GetNormalized();
			penetration = distanceToUp + discRadius;
			start = discCenter;
			end = start + normal * distanceToUp;

			minDistance = distanceToUp;
		}

		if (distanceToBottom < minDistance)
		{
			normal = -bottomToPtVertical.GetNormalized();
			penetration = distanceToBottom + discRadius;
			start = discCenter;
			end = start + normal * distanceToBottom;

			minDistance = distanceToBottom;
		}

		if (distanceToLeft < minDistance)
		{
			normal = -leftToPtVertical.GetNormalized();
			penetration = distanceToLeft + discRadius;
			start = discCenter;
			end = start + normal * distanceToLeft;

			minDistance = distanceToLeft;
		}

		if (distanceToRight < minDistance)
		{
			normal = -rightToPtVertical.GetNormalized();
			penetration = distanceToRight + discRadius;
			start = discCenter;
			end = start + normal * distanceToRight;

			minDistance = distanceToRight;
		}

		manifold->m_penetration = penetration;
		manifold->m_normal = normal;
		manifold->m_start = start;
		manifold->m_end = end;

		return true;
	}
}

bool OBB2VsOBB2Manifold(Manifold* manifold)
{
	Entity* e1 = manifold->m_e1;
	Entity* e2 = manifold->m_e2;

	// from 1 to 2
	Vector2 displacement = e2->m_center - e1->m_center;

	RectangleEntity* r1 = dynamic_cast<RectangleEntity*>(e1);
	RectangleEntity* r2 = dynamic_cast<RectangleEntity*>(e2);
	if (r1 == nullptr || r2 == nullptr)
	{
		ASSERT_OR_DIE(false, "Assume manifold to hold rec entity, which is not ture");
	}

	OBB2& rec1 = r1->m_primitive;
	OBB2& rec2 = r2->m_primitive;

	Vector2 rec1BL = rec1.GenerateBL();
	Vector2 rec1BR = rec1.GenerateBR();
	Vector2 rec1TL = rec1.GenerateTL();
	Vector2 rec1TR = rec1.GenerateTR();
	Vector2 rec1Vertices[4] = {rec1BL, rec1TL, rec1TR, rec1BR};

	Vector2 rec2BL = rec2.GenerateBL();
	Vector2 rec2BR = rec2.GenerateBR();
	Vector2 rec2TL = rec2.GenerateTL();
	Vector2 rec2TR = rec2.GenerateTR();
	Vector2 rec2Vertices[4] = {rec2BL, rec2TL, rec2TR, rec2BR};

	Vector2 leftNormal1  = rec1.GenerateLeftNormal();
	Vector2 upNormal1    = rec1.GenerateUpNormal();
	Vector2 rightNormal1 = rec1.GenerateRightNormal();
	Vector2 downNormal1  = rec1.GenerateDownNormal();

	Vector2 leftNormal2  = rec2.GenerateLeftNormal();
	Vector2 upNormal2    = rec2.GenerateUpNormal();
	Vector2 rightNormal2 = rec2.GenerateRightNormal();
	Vector2 downNormal2  = rec2.GenerateDownNormal();

	// support point processing of rec 1

	// left normal of rec 1
	float signedDistanceLN1 = INFINITY;
	Vector2 supportPointLN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec2Vertices[i];
		Vector2 blToPt = spCandidate - rec1BL;
		float distance = DotProduct(leftNormal1, blToPt);

		if (distance < signedDistanceLN1)
		{
			signedDistanceLN1 = distance;
			supportPointLN1 = spCandidate;
		}
	}
	if (signedDistanceLN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for LN1 and it is recorded

	// up normal of rec 1
	float signedDistanceUN1 = INFINITY;
	Vector2 supportPointUN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec2Vertices[i];
		Vector2 tlToPt = spCandidate - rec1TL;
		float distance = DotProduct(upNormal1, tlToPt);

		if (distance < signedDistanceUN1)
		{
			signedDistanceUN1 = distance;
			supportPointUN1 = spCandidate;
		}
	}
	if (signedDistanceUN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for UN1 and it is recorded

	// right normal of rec 1
	float signedDistanceRN1 = INFINITY;
	Vector2 supportPointRN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec2Vertices[i];
		Vector2 trToPt = spCandidate - rec1TR;
		float distance = DotProduct(rightNormal1, trToPt);

		if (distance < signedDistanceRN1)
		{
			signedDistanceRN1 = distance;
			supportPointRN1 = spCandidate;
		}
	}
	if (signedDistanceRN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for RN1 and it is recorded

	// down normal of rec 1
	float signedDistanceDN1 = INFINITY;
	Vector2 supportPointDN1 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec2Vertices[i];
		Vector2 brToPt = spCandidate - rec1BR;
		float distance = DotProduct(downNormal1, brToPt);

		if (distance < signedDistanceDN1)
		{
			signedDistanceDN1 = distance;
			supportPointDN1 = spCandidate;
		}
	}
	if (signedDistanceDN1 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for DN1 and it is recorded

	Vector2 normal1To2 = Vector2::ZERO;
	float absDist1 = INFINITY;
	Vector2 start1 = Vector2::ZERO;
	Vector2 end1 = Vector2::ZERO;
	float absDistLN1 = abs(signedDistanceLN1);
	float absDistUN1 = abs(signedDistanceUN1);
	float absDistRN1 = abs(signedDistanceRN1);
	float absDistDN1 = abs(signedDistanceDN1);

	if (absDistLN1 < absDist1)
	{
		absDist1 = absDistLN1;
		normal1To2 = leftNormal1;
		start1 = supportPointLN1;
		end1 = start1 + normal1To2 * absDist1;
	}
	if (absDistUN1 < absDist1)
	{
		absDist1 = absDistUN1;
		normal1To2 = upNormal1;
		start1 = supportPointUN1;
		end1 = start1 + normal1To2 * absDist1;
	}
	if (absDistRN1 < absDist1)
	{
		absDist1 = absDistRN1;
		normal1To2 = rightNormal1;
		start1 = supportPointRN1;
		end1 = start1 + normal1To2 * absDist1;
	}
	if (absDistDN1 < absDist1)
	{
		absDist1 = absDistDN1;
		normal1To2 = downNormal1;
		start1 = supportPointDN1;
		end1 = start1 + normal1To2 * absDist1;
	}
	// at this moment we have normal and penetration candidate from rec 1

	// support point processing of rec 2

	// left normal of rec 2
	float signedDistanceLN2 = INFINITY;
	Vector2 supportPointLN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec1Vertices[i];
		Vector2 blToPt = spCandidate - rec2BL;
		float distance = DotProduct(leftNormal2, blToPt);

		if (distance < signedDistanceLN2)
		{
			signedDistanceLN2 = distance;
			supportPointLN2 = spCandidate;
		}
	}
	if (signedDistanceLN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for LN2 and it is recorded

	// up normal of rec 2
	float signedDistanceUN2 = INFINITY;
	Vector2 supportPointUN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec1Vertices[i];
		Vector2 tlToPt = spCandidate - rec2TL;
		float distance = DotProduct(upNormal2, tlToPt);

		if (distance < signedDistanceUN2)
		{
			signedDistanceUN2 = distance;
			supportPointUN2 = spCandidate;
		}
	}
	if (signedDistanceUN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for UN2 and it is recorded

	// right normal of rec 2
	float signedDistanceRN2 = INFINITY;
	Vector2 supportPointRN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec1Vertices[i];
		Vector2 trToPt = spCandidate - rec2TR;
		float distance = DotProduct(rightNormal2, trToPt);

		if (distance < signedDistanceRN2)
		{
			signedDistanceRN2 = distance;
			supportPointRN2 = spCandidate;
		}
	}
	if (signedDistanceRN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for RN2 and it is recorded

	// down normal of rec 2
	float signedDistanceDN2 = INFINITY;
	Vector2 supportPointDN2 = Vector2::ZERO;
	for (int i = 0; i < 4; ++i)
	{
		Vector2 spCandidate = rec1Vertices[i];
		Vector2 brToPt = spCandidate - rec2BR;
		float distance = DotProduct(downNormal2, brToPt);

		if (distance < signedDistanceDN2)
		{
			signedDistanceDN2 = distance;
			supportPointDN2 = spCandidate;
		}
	}
	if (signedDistanceDN2 > 0.f)
	{
		return false;
	}
	// otherwise we have a SP for DN2 and it is recorded

	// normal and penetration candidate from rec 2
	Vector2 normal2To1 = Vector2::ZERO;
	float absDist2 = INFINITY;
	Vector2 start2 = Vector2::ZERO;
	Vector2 end2 = Vector2::ZERO;
	float absDistLN2 = abs(signedDistanceLN2);
	float absDistUN2 = abs(signedDistanceUN2);
	float absDistRN2 = abs(signedDistanceRN2);
	float absDistDN2 = abs(signedDistanceDN2);

	if (absDistLN2 < absDist2)
	{
		absDist2 = absDistLN2;
		normal2To1 = leftNormal2;
		end2 = supportPointLN2;
		start2 = end2 - (-normal2To1) * absDist2;
	}
	if (absDistUN2 < absDist2)
	{
		absDist2 = absDistUN2;
		normal2To1 = upNormal2;
		end2 = supportPointUN2;
		start2 = end2 - (-normal2To1) * absDist2;
	}
	if (absDistRN2 < absDist2)
	{
		absDist2 = absDistRN2;
		normal2To1 = rightNormal2;
		end2 = supportPointRN2;
		start2 = end2 - (-normal2To1) * absDist2;
	}
	if (absDistDN2 < absDist2)
	{
		absDist2 = absDistDN2;
		normal2To1 = downNormal2;
		end2 = supportPointDN2;
		start2 = end2 - (-normal2To1) * absDist2;
	}

	// at this point we have normal and penetration candidate from rec 2

	// round two candidates
	float roundedDist1 = floorf(absDist1 * 100.f) / 100.f;
	float roundedDist2 = floorf(absDist2 * 100.f) / 100.f;

	// compare the candidate 
	if (roundedDist1 <= roundedDist2)
	{
		manifold->m_penetration = absDist1;
		manifold->m_normal = normal1To2;
		manifold->m_start = start1;
		manifold->m_end = end1;
		return true;
	}
	else
	{
		manifold->m_penetration = absDist2;
		manifold->m_normal = -normal2To1;
		manifold->m_start = start2;
		manifold->m_end = end2;
		return true;
	}
}
