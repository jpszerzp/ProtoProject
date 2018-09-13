#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"

class OBB2
{
public:
	Vector2 m_center;
	float	m_rotation;
	Vector2 m_scale;

public:
	Vector2 GenerateLeftNormal() const;
	Vector2 GenerateRightNormal() const;
	Vector2 GenerateUpNormal() const;
	Vector2 GenerateDownNormal() const;

	Vector2 GenerateBL() const;
	Vector2 GenerateBR() const;
	Vector2 GenerateTL() const;
	Vector2 GenerateTR() const;

	AABB2 GenerateAABB() const;

	bool IsPointInside( const Vector2& point ) const;	

	OBB2();
	OBB2(Vector2 center, float rotation, Vector2 scale);
	~OBB2();
};