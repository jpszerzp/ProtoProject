#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Quad.hpp"

enum eCCDResolver_ID : uint
{
	CCD_SPHERE_PLANE = 0
};

struct CCDResolver
{
	eCCDResolver_ID m_id;

	CCDResolver(){}
	~CCDResolver(){}
};


struct CCDResolver_SpherePlane : CCDResolver
{
	// properties for ccd resolver on sphere vs plane:
	// signed distance before integration - sc
	// signed distance after integratiom - se
	// sphere radius - r
	float sc;
	float se;
	float r;

	// involved game objects
	Sphere* s = nullptr;
	Quad* q = nullptr;

	CCDResolver_SpherePlane(Sphere* s_arg, Quad* q_arg, float r_arg, float sc_arg)
		: s(s_arg), q(q_arg), r(r_arg), sc(sc_arg)
	{
		// se is not known upon construction

		m_id = CCD_SPHERE_PLANE;
	}

	~CCDResolver_SpherePlane(){}
};

