#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Primitive3.hpp"

class Entity3;

class AABB3
{
private:
	Entity3* m_entity = nullptr;

public:
	Vector3 m_min;
	Vector3 m_max;

public:
	AABB3() : m_min(Vector3::ZERO), m_max(Vector3::ZERO){}
	AABB3(Vector3 min, Vector3 max)
		: m_min(min), m_max(max) {}
	~AABB3(){}

	Vector3 GetRightAxis() const { return Vector3(1.f, 0.f, 0.f); }
	Vector3 GetUpAxis() const { return Vector3(0.f, 1.f, 0.f); }
	Vector3 GetForwardAxis() const { return Vector3(0.f, 0.f, 1.f); }
	Vector3 GetDimensions() const;
	Vector3 GetHalfDimensions() const;
	Vector3 GetCenter() const;
	void GetVertices(Vector3* out_vertices) const;
	void GetEdges(Vector3* out_edges) const;
	Entity3* GetEntity() const { return m_entity; }

	void Translate(Vector3 translation);
	
	void SetCenter(const Vector3& center);
	void SetEntity(Entity3* ent) { m_entity = ent; }

	float ProjectJHalfDimToAxisAbs(const Vector3& axis) const;
};

bool OverlapOnAxis(const AABB3& box1, const AABB3& box2, const Vector3& axis, float& overlap);
bool OverlapOnAxis(const AABB3& box1, const AABB3& box2, const Vector3& axis);			// SAT on AABB3
bool CollideAABB3(const AABB3& box1, const AABB3& box2);								// ordinary AABB3 test
