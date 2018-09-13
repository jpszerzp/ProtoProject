#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Sphere3.hpp"

#include <vector>

struct RaycastHit3 
{
	bool    m_hit;
	Vector3 m_hitLocation;
	float   m_extents;
	//Vector3 m_hitNormal;

	RaycastHit3()
	{
		m_hit = false;
		m_hitLocation = Vector3(INFINITY, INFINITY, INFINITY);
		m_extents = -1.f;
		//m_hitNormal = Vector3(INFINITY, INFINITY, INFINITY);
	}
	
	RaycastHit3(bool hit, Vector3 hitLocation, float extents)
		: m_hit(hit), m_hitLocation(hitLocation), m_extents(extents)
	{

	}
};

struct Ray3
{
	Ray3() {}
	Ray3(Vector3 start, Vector3 direction)
		: m_start(start), m_normalizedDirection(direction)
	{

	}

	Vector3 m_start;
	Vector3 m_normalizedDirection;

	Vector3 Evaluate(float distanceFromStart);
	float   ReverseEvaluate(Vector3 point3DOnPoint);

	int IntersectAABB3(std::vector<RaycastHit3>& hits, AABB3 bound);
	int IntersectSphere3(std::vector<RaycastHit3>& hits, Sphere3 sphere);

	bool IsPt1ExceedingPt2AlongRay(Vector3 p1, Vector3 p2);
};