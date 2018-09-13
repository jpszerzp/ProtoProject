#pragma once

#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Physics/3D/ForceRegistry.hpp"

class Spring
{
public:
	float m_const;
	float m_restLength;

	Point* m_end1;
	Point* m_end2;

public:
	Spring(Point* e1, Point* e2, float coef, float restLength);
	~Spring(){}
};

class AnchorSpring : public Spring
{
public:
	AnchorSpring(Point* e1, Point* e2, float coef, float restLength)
		: Spring(e1, e2, coef, restLength){}
	~AnchorSpring(){}
};