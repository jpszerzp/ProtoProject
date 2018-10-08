#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Primitive3.hpp"

class Entity3;

class Sphere3 
{
private:
	Entity3* m_entity = nullptr;

public:
	Vector3 m_center;
	float	m_radius;

public:
	Sphere3() : m_center(Vector3::ZERO), m_radius(0.f){}
	Sphere3(Vector3 center, float radius)
		: m_center(center), m_radius(radius){}
	~Sphere3(){}

	float GetRadius() const { return m_radius; }
	Vector3 GetCenter() const { return m_center; }
	Entity3* GetEntity() const { return m_entity; }

	void Translate(Vector3 translation);

	void SetCenter(const Vector3& center);
	void SetEntity(Entity3* ent) { m_entity = ent; }
};