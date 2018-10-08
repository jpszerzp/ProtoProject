#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Primitive3.hpp"

class Entity3;

class Particle
{
private:
	Entity3* m_entity = nullptr;

public:
	Vector3 m_position;
	float m_size;

	Particle();
	Particle(Vector3 pos, float size);
	~Particle();

	float GetPointSize() const { return m_size; }
	Vector3 GetCenter() const { return m_position; }
	Entity3* GetEntity() const { return m_entity; }

	void Translate(Vector3 translation);

	void SetCenter(const Vector3& center);
	void SetEntity(Entity3* ent) { m_entity = ent; }
};