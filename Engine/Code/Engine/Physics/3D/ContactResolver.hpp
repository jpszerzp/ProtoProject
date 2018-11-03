#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"

#include <vector>

#define COHERENT_POS_ITER 2
#define COHERENT_VEL_ITER 2
#define COHERENT_POS_EPSILON 0.01f
#define COHERENT_VEL_EPSILON 0.01f

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

	// iterative
	void ResolveContactsIterative(float deltaTime);

	// all
	void ResolveContactsAll(float deltaTime);

	// coherent
	void ResolveContactsCoherent(float deltaTime);
	void PrepareContactsCoherent(float deltaTime);
	void ResolvePositionsCoherent(float deltaTime);
	void ResolveVelocityCoherent(float deltaTime);

	void ClearRecords();

	CollisionData3* GetCollisionData() { return m_collision; }
};