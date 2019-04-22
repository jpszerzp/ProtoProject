#pragma once

//#include "Engine/Physics/3D/PHYSX/PhysXProcessor.hpp"

#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Physics/3D/ForceRegistry.hpp"
#include "Engine/Physics/3D/RF/CollisionEntity.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"

/*
 * Particle(gameobject) dependent spring
 */
class Spring
{
public:
	float m_const;
	float m_restLength;

	CollisionPoint* m_end1;
	CollisionPoint* m_end2;

public:
	Spring(CollisionPoint* e1, CollisionPoint* e2, float coef, float restLength)
		: m_end1(e1), m_end2(e2), m_const(coef), m_restLength(restLength){}
	~Spring(){}
};

class AnchorSpring : public Spring
{
public:
	AnchorSpring(CollisionPoint* e1, CollisionPoint* e2, float coef, float restLength)
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

	CollisionEntity* m_r1;
	CollisionEntity* m_r2;

public:
	GeneralEntitySpring(CollisionEntity* r1, CollisionEntity* r2, float coef, float rl)
		: m_r1(r1), m_r2(r2), m_const(coef), m_restLength(rl){}
	~GeneralEntitySpring(){}
};

class GeneralRigidAnchorSpring : public GeneralEntitySpring
{
public:
	// r1 is the anchor, r2 is the attached rigid body
	GeneralRigidAnchorSpring(CollisionEntity* r1, CollisionRigidBody* r2, float coef, float rl)
		: GeneralEntitySpring(r1, r2, coef, rl){}
	~GeneralRigidAnchorSpring(){}
};