#pragma once

#include "Engine/Math/Vector3.hpp"

class Entity3;

/*
 * DEPRECATED.
 */

class Primitive3
{
private:
	Entity3* m_entity = nullptr;

public:
	virtual void Translate(Vector3 translation);

	void SetEntity(Entity3* entity) { m_entity = entity; }
	Entity3* GetEntity() const { return m_entity; }
};