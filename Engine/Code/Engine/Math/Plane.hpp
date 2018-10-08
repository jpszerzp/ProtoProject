#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/OBB3.hpp"

#define X_INVALID_EXTENSION -1
#define Z_INVALID_EXTENSION -1

class Entity3;

class Plane
{
private:
	Entity3* m_entity = nullptr;

public:
	Vector3 m_normal;
	float m_offset;

public:
	Plane(){}
	Plane(Vector3 normal, float offset)
		: m_normal(normal), m_offset(offset) {}
	~Plane(){}

	void Translate(Vector3 translation);

	Vector3 GetNormal() const { return m_normal; }
	float GetOffset() const { return m_offset; }
	Entity3* GetEntity() const { return m_entity; }

	void SetEntity(Entity3* ent) { m_entity = ent; }
};
