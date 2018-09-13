#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"

#include <vector>

/**
* The contact resolution routine for particle contacts. One
* resolver instance can be shared for the whole simulation.
*/
class ContactResolver
{
protected:
	/**
	* Holds the number of iterations allowed.
	*/
	uint m_iterations;
	/**
	* This is a performance tracking value - we keep a record
	* of the actual number of iterations used.
	*/
	uint m_iterationsUsed;
public:
	/**
	* Creates a new contact resolver.
	*/
	ContactResolver(){}
	ContactResolver(uint iterations);
	~ContactResolver(){}
	/**
	* Sets the number of iterations that can be used.
	*/
	void SetIterations(uint iterations) { m_iterations = iterations; }
	/**
	* Resolves a set of particle contacts for both penetration
	* and velocity.
	*/
	void ResolveContacts(std::vector<Contact3>& contacts,
		uint numContacts, float deltaTime);
};