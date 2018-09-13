#include "Game/Spring.hpp"

Spring::Spring(Point* e1, Point* e2, float coef, float restLength)
	: m_end1(e1), m_end2(e2), m_const(coef), m_restLength(restLength)
{

}

