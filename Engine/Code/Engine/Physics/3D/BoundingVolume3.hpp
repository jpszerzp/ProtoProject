#pragma once
#include "Engine/Math/Vector3.hpp"

struct BoundingSphere
{
	Vector3 m_center;
	float m_radius;

public:
	BoundingSphere() : m_center(Vector3::ZERO), m_radius(0.f){}
	BoundingSphere(const Vector3& center, float radius)
		: m_center(center), m_radius(radius){}
	BoundingSphere(const BoundingSphere& child_one, const BoundingSphere& child_two);
	bool Overlaps(BoundingSphere* other) const;
	float GetVolume() const;
	float GetGrowth(const BoundingSphere& other) const;

	// legacy 
	void Translate(Vector3 translation);
	void SetCenter(Vector3 center);
};