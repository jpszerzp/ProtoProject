#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Renderer.hpp"

struct BoundingSphere
{
	Vector3 m_center;
	float m_radius;
	Transform m_transform;			
	Mesh* m_boundMesh = nullptr;	

public:
	BoundingSphere() : m_center(Vector3::ZERO), m_radius(0.f){}
	BoundingSphere(const Vector3& center, float radius)
		: m_center(center), m_radius(radius){}
	BoundingSphere(const BoundingSphere& child_one, const BoundingSphere& child_two);
	~BoundingSphere();
	bool Overlaps(const BoundingSphere* other) const;
	float GetVolume() const;
	float GetGrowth(const BoundingSphere& other) const;
	float GetRadius() const { return m_radius; }
	Vector3 GetCenter() const { return m_center; }

	// legacy code directly operating on center
	void Translate(Vector3 translation);
	void SetCenter(Vector3 center);

	void DrawBound(Renderer* renderer);
};