#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"

#include <vector>

enum eResolveScheme
{
	RESOLVE_ALL,
	RESOLVE_ITERATIVE,
	RESOLVE_COHERENT
};

class ContactResolver
{
protected:
	uint m_iterations;
	uint m_iterationsUsed;
	eResolveScheme m_scheme;

	CollisionData3* m_collision;

public:
	ContactResolver();							// default to all resolver
	ContactResolver(uint iterations);			// for iterative resolver
	ContactResolver(eResolveScheme scheme);	
	~ContactResolver(){}

	void SetIterations(uint iterations) { m_iterations = iterations; }
	void ResolveContacts(float deltaTime);
	void ResolveContactsIterative(float deltaTime);
	void ResolveContactsAll(float deltaTime);
	void ResolveContactCoherent(float deltaTime);
	void ClearRecords();

	CollisionData3* GetCollisionData() { return m_collision; }
};