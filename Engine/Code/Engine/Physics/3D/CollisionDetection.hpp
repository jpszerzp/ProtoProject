#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Physics/3D/SphereEntity3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

/*
 * Contains data from contact generation.
 */
class Contact3
{
public:
	Entity3* m_e1;
	Entity3* m_e2;

	// core
	float m_penetration;
	Vector3 m_normal;
	Vector3 m_point;

	Vector3 m_start;
	Vector3 m_end;

	float m_restitution;

public:
	Contact3();
	Contact3(Entity3* e1, Entity3* e2);
	Contact3(Entity3* e1, Entity3* e2, Vector3 normal, Vector3 point, float penetration);
	~Contact3();

	float ComputeSeparatingVelocity() const;
	void ResolveContact(float deltaTime);
	void ResolveContactNaive();		// navie: positional change only

private:
	void ResolveVelocity(float deltaTime);
	void ResolvePenetration(float deltaTime);
};

/**
* Collision data in 3D scenes.
*/
struct CollisionData3
{
	std::vector<Contact3> m_contacts;
	std::vector<Contact3> m_impulsive_contacts;
	uint m_maxContacts = 0;

	/*
	// coherence contact
	Contact3* m_coherenceContacts;
	uint m_coherenceContactsCount = 0;
	*/

	CollisionData3(){}
	CollisionData3(uint max) { m_maxContacts = max; }
	~CollisionData3()
	{
		ClearContacts();
	}

	void ClearContacts()
	{
		m_contacts.clear();
	}

	std::vector<Contact3>& GetContacts() { return m_contacts; }
};

class CollisionDetector
{
public:
	CollisionDetector();
	~CollisionDetector();

	static uint Sphere3VsSphere3(const Sphere3& s1, 
		const Sphere3& s2, CollisionData3* data);
	static uint Sphere3VsPlane3(const Sphere3& sph,
		const Plane& pl, CollisionData3* data);
	static uint Sphere3VsAABB3(const Sphere3& sph,
		const AABB3& aabb3, CollisionData3* data);
	static uint AABB3VsPlane3(const AABB3& aabb,
		const Plane& plane, CollisionData3* data);
	static uint AABB3VsPlane3Single(const AABB3& aabb,
		const Plane& plane, CollisionData3* data);

	static uint AABB3VsAABB3(const AABB3& aabb3_1,
		const AABB3& aabb3_2, CollisionData3* data);
	static uint AABB3VsAABB3Single(const AABB3& aabb3_1,
		const AABB3& aabb3_2, CollisionData3* data);
	//static uint NonCoherentProcessingAABB3VsAABB3(const AABB3& aabb3_1,
	//	const AABB3& aabb3_2, CollisionData3* data);
	//static uint CoherentProcessingAABB3VsAABB3(const AABB3& aabb_1,
	//	const AABB3& aabb3_2, CollisionData3* data);

	// TODO
	static void BoxAndPointPenetration(const AABB3& aabb, const Vector3& point, float* out_penetration);
	static void BoxAndEdgePenetration(const AABB3& aabb, const Vector3& edge, float* out_penetration);
	static void EdgeAndEdgePenetration(const Vector3& e1, const Vector3& e2, float* out_penetration);
	static uint BoxAndPointContact(const AABB3& aabb, const Vector3& point, CollisionData3* data);
	static uint BoxAndEdgeContact(const AABB3& aabb, const Vector3& edge, CollisionData3* data);
	static uint EdgeAndEdgeContact(const Vector3& edge1, const Vector3& edge2, CollisionData3* data);
};

