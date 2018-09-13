#include "Engine/Core/Ray3.hpp"
#include "Engine/Math/MathUtils.hpp"

Vector3 Ray3::Evaluate(float distanceFromStart)
{
	Vector3 res = m_start + m_normalizedDirection * distanceFromStart;

	return res;
}

//this assumes input point is ON ray
float Ray3::ReverseEvaluate(Vector3 point3DOnRay)
{
	Vector3 displacement = point3DOnRay - m_start;

	float lengthOfDisplacement = displacement.GetLength();
	Vector3 displacementNormalized = displacement / lengthOfDisplacement;

	float dot = DotProduct(displacementNormalized, m_normalizedDirection);

	if (dot > 0.f)
	{
		return lengthOfDisplacement;
	}
	else
	{
		return -lengthOfDisplacement;
	}
}


int Ray3::IntersectAABB3(std::vector<RaycastHit3>& hits, AABB3 bound)
{
	int intersection = 0;

	float boundMinX = bound.m_min.x;
	float boundMaxX = bound.m_max.x;
	float boundMinY = bound.m_min.y;
	float boundMaxY = bound.m_max.y;
	float boundMinZ = bound.m_min.z;
	float boundMaxZ = bound.m_max.z;

	float tMin = (boundMinX - m_start.x) / m_normalizedDirection.x;
	float tMax = (boundMaxX - m_start.x) / m_normalizedDirection.x;

	if (tMin > tMax)
	{
		SwapFloat(tMin, tMax);
	}

	float tyMin = (boundMinY - m_start.y) / m_normalizedDirection.y;
	float tyMax = (boundMaxY - m_start.y) / m_normalizedDirection.y;

	if (tyMin > tyMax)
	{
		SwapFloat(tyMin, tyMax);
	}

	if ((tMin > tyMax) || (tyMin > tMax))
	{
		return intersection;
	}

	if (tyMin > tMin)
	{
		tMin = tyMin;
	}

	if (tyMax < tMax)
	{
		tMax = tyMax;
	}

	float tzMin = (boundMinZ - m_start.z) / m_normalizedDirection.z;
	float tzMax = (boundMaxZ - m_start.z) / m_normalizedDirection.z;

	if (tzMin > tzMax)
	{
		SwapFloat(tzMin, tzMax);
	}

	if ((tMin > tzMax) || (tzMin > tMax))
	{
		return intersection;
	}

	if (tzMin > tMin)
	{
		tMin = tzMin;
	}

	if (tzMax < tMax)
	{
		tMax = tzMax;
	}

	intersection = 2;

	// if the origin of ray is in aabb3
	if (tMin < 0.f)
	{
		intersection--;

		Vector3 maxIntersectionLocation = m_start + m_normalizedDirection * tMax;
		RaycastHit3 maxIntersection = RaycastHit3(true, maxIntersectionLocation, tMax);

		hits.push_back(maxIntersection);
	}
	else
	{
		Vector3 minIntersectionLocation = m_start + m_normalizedDirection * tMin;
		RaycastHit3 minIntersection = RaycastHit3(true, minIntersectionLocation, tMin);

		Vector3 maxIntersectionLocation = m_start + m_normalizedDirection * tMax;
		RaycastHit3 maxIntersection = RaycastHit3(true, maxIntersectionLocation, tMax);

		hits.push_back(minIntersection);
		hits.push_back(maxIntersection);
	}

	return intersection;
}


int Ray3::IntersectSphere3(std::vector<RaycastHit3>& hits, Sphere3 sphere)
{
	int hitsNum = 0;

	float t0;
	float t1;
	float sphereRad = sphere.m_radius;

	Vector3 toSphereCenter = sphere.m_center - m_start;
	float projection = DotProduct(toSphereCenter, m_normalizedDirection);
	float distToCenter = DotProduct(toSphereCenter, toSphereCenter) - projection * projection;
	if (distToCenter > sphereRad)
	{
		// there is no intersection
		return hitsNum;
	}
	else
	{
		float offset = sqrtf(sphereRad * sphereRad - distToCenter * distToCenter);
		t0 = projection - offset;
		t1 = projection + offset;
	}
	
	/*
	if (t0 > t1)
	{
		std::swap(t0, t1);
	}
	*/

	if (t0 > 0)
	{
		hitsNum++;

		Vector3 hitLocation = Evaluate(t0);
		RaycastHit3 castHit = RaycastHit3(true, hitLocation, t0);

		hits.push_back(castHit);
	}

	if (t1 > 0)
	{
		hitsNum++;

		Vector3 hitLocation = Evaluate(t1);
		RaycastHit3 castHit = RaycastHit3(true, hitLocation, t1);

		hits.push_back(castHit);
	}

	return hitsNum;
}

bool Ray3::IsPt1ExceedingPt2AlongRay(Vector3 p1, Vector3 p2)
{
	float t1 = ReverseEvaluate(p1);
	float t2 = ReverseEvaluate(p2);

	if (t1 > t2)
	{
		return true;
	}
	else
	{
		return false;
	}
}