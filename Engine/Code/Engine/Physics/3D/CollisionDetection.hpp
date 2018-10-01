#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Physics/3D/SphereEntity3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define MAX_CONTACTS 200
#define COHERENT_THRESHOLD -0.01f

enum eContactFeature
{
	V1, V2, V3, V4, V5, V6, V7, V8,
	F1, F2, F3, F4, F5, F6,
	E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12,
	UNKNOWN
};

enum eContactType
{
	POINT_FACE,
	EDGE_EDGE,
	NO_CARE
};

/*
 * Contains data from contact generation.
 */
class Contact3
{
public:
	eContactType m_type = NO_CARE;
	eContactFeature m_f1;	// feature from e1
	eContactFeature m_f2;	// feature from e2

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
	//void ResolveContactNaive();		// navie: positional change only

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
	uint m_maxContacts = 0;

	CollisionData3(){}
	CollisionData3(uint max) { m_maxContacts = max; }
	~CollisionData3() { ClearContacts(); }

	void ClearContacts() { m_contacts.clear(); }
	void ClearCoherent();

	bool HasAndUpdateContact(const Contact3& contact);

	std::vector<Contact3>& GetContacts() { return m_contacts; }
};

class CollisionDetector
{
protected:
	const static Vector3 ISA;

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

	static uint OBB3VsPlane3(const OBB3& obb,
		const Plane& plane, CollisionData3* data);
	static uint OBB3VsSphere3(const OBB3& obb,
		const Sphere3& sphere, CollisionData3* data);
	static uint OBB3VsOBB3Shallow(const OBB3& obb1,
		const OBB3& obb2, CollisionData3* data);
	static uint OBB3VsOBB3Deep(const OBB3& obb1,
		const OBB3& obb2, CollisionData3* data);
	static uint OBB3VsPoint(const OBB3& obb, const Vector3& p, Contact3& contact, bool reverse);
	
	// general entity detection
	static uint Entity3VsEntity3(Entity3* e1, Entity3* e2, CollisionData3* data);

	// TODO
	//static void BoxAndPointPenetration(const AABB3& aabb, const Vector3& point, float* out_penetration);
	//static void BoxAndEdgePenetration(const AABB3& aabb, const Vector3& edge, float* out_penetration);
	//static void EdgeAndEdgePenetration(const Vector3& e1, const Vector3& e2, float* out_penetration);
	//static uint BoxAndPointContact(const AABB3& aabb, const Vector3& point, CollisionData3* data);
	//static uint BoxAndEdgeContact(const AABB3& aabb, const Vector3& edge, CollisionData3* data);
	//static uint EdgeAndEdgeContact(const Vector3& edge1, const Vector3& edge2, CollisionData3* data);
};

