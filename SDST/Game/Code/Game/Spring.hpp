#pragma once

#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Physics/3D/ForceRegistry.hpp"

/*
 * Particle(gameobject) dependent spring
 */
class Spring
{
public:
	float m_const;
	float m_restLength;

	Point* m_end1;
	Point* m_end2;

public:
	Spring(Point* e1, Point* e2, float coef, float restLength)
		: m_end1(e1), m_end2(e2), m_const(coef), m_restLength(restLength){}
	~Spring(){}
};

class AnchorSpring : public Spring
{
public:
	AnchorSpring(Point* e1, Point* e2, float coef, float restLength)
		: Spring(e1, e2, coef, restLength){}
	~AnchorSpring(){}
};

/*
 * General entity dependent spring
 */
class GeneralEntitySpring
{
public:
	float m_const;
	float m_restLength;

	Entity3* m_r1;
	Entity3* m_r2;

public:
	GeneralEntitySpring(Entity3* r1, Entity3* r2, float coef, float rl)
		: m_r1(r1), m_r2(r2), m_const(coef), m_restLength(rl){}
	~GeneralEntitySpring(){}
};

class GeneralRigidAnchorSpring : public GeneralEntitySpring
{
public:
	// r1 is the anchor, r2 is the attached rigid body
	GeneralRigidAnchorSpring(Entity3* r1, Rigidbody3* r2, float coef, float rl)
		: GeneralEntitySpring(r1, r2, coef, rl){}
	~GeneralRigidAnchorSpring(){}
};