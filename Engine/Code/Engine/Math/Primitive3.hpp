#pragma once

#include "Engine/Math/Vector3.hpp"

class Entity3;

class Primitive3
{
public:
	Entity3* m_entity = nullptr;

public:
	virtual void Translate(Vector3){}
	virtual void SetCenter(const Vector3&){}

	void SetEntity(Entity3* entity) { m_entity = entity; }
	Entity3* GetEntity() const { return m_entity; }

	Primitive3(){}
	virtual ~Primitive3(){}
};