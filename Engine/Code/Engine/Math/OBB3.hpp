#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix33.hpp"

class OBB3
{
public:
	//Vector3  m_center;
	//Vector3  m_rot;
	//Vector3  m_scale;

	//Vector3  m_extents;
	//Vector3  m_forward;
	//Vector3  m_right;
	//Vector3  m_up;

	Vector3 m_center;			// OBB center point
	Vector3 m_basis[3];			// basis
	Vector3 m_extends;			// halfway extends of OBB along basis 

public:
	OBB3(){}
	~OBB3(){}
};