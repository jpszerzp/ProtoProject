#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Physics/3D/SphereEntity3.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define MAX_CONTACTS 2000
#define COHERENT_THRESHOLD 0.005f

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
	eContactFeature m_f1 = FEATURE_NO_CARE;	// feature from e1
	eContactFeature m_f2 = FEATURE_NO_CARE;	// feature from e2

	Entity3* m_e1;
	Entity3* m_e2;

	// core
	float m_penetration;
	Vector3 m_normal;
	Vector3 m_point;

	float m_restitution;
	float m_friction;

	// transformation convenience properties
	Matrix33 m_toWorld;		// matrix transform basis from contact coord to world coord, with local basis of contact coord as components
	Vector3 m_closingVel;
	float m_desiredVelDelta;
	Vector3 m_relativePosWorld[2];

public:
	Contact3();
	Contact3(Entity3* e1, Entity3* e2);
	Contact3(Entity3* e1, Entity3* e2, Vector3 normal, Vector3 point, float penetration, float res = 1.f, float friction = 0.f);
	~Contact3();

	float ComputeSeparatingVelocity() const;
	void ResolveContact(float deltaTime);
	void ResolveVelocity(float deltaTime);
	void ResolvePenetration(float deltaTime);
	void MakeToWorld();

	// vel change
	float GetVelPerImpulseContact();
	float GetDeltaVel();
	Vector3 ComputeContactImpulse();
	Vector3 ComputeContactImpulseFriction();
	Vector3 ComputeWorldImpulse();
	Vector3 ComputeWorldImpulseFriction();

	// iit short for inverse inertia tensor
	Vector3 RF_ComputeFrictionlessImpulse();

	// pos change
	void SolveNonlinearProjection( float angularInertia[2], float linearInertia[2], float angularMove[2], float linearMove[2]);

	void ApplyImpulse();
	void ResolveVelocityCoherent(Vector3 linearChange[2], Vector3 angularChange[2]);
	void ResolvePositionCoherent(Vector3 linearChange[2], Vector3 angularChange[2]);
	void RF_ResolvePositionCoherent(Vector3 linearChange[2], Vector3 angularChange[2]);

	// coherent
	void PrepareInternal(float deltaTime);
	void SwapEntities();
	Vector3 ComputeContactVelocity(int idx, Entity3* ent, float deltaTime);
	void ComputeDesiredVelDeltaCoherent();

	// resting
	void ComputeDesiredVelDeltaResting(float deltaTime);

	// contact may wake entities up
	void WakeUp();
	
	bool DoesFeatureMatter() const { return m_type != NO_CARE; }
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
	//bool FeatureMatchAndUpdate(const Contact3& comparer, Contact3& comparee);
	//bool EntityMatchAndUpdate(const Contact3& comparer, Contact3& comparee);

	//std::vector<Contact3>& GetContacts() { return m_contacts; }
};

class CollisionDetector
{
protected:
	const static Vector3 ISA;

public:
	CollisionDetector();
	~CollisionDetector();

	// sphere vs sphere
	static bool Sphere3VsSphere3Core(const Sphere3& s1, const Sphere3& s2, Contact3& contact);
	static uint Sphere3VsSphere3Single(const Sphere3& s1, const Sphere3& s2, CollisionData3* data);
	static uint Sphere3VsSphere3Coherent(const Sphere3& s1, const Sphere3& s2, CollisionData3* data);

	// sphere vs plane
	static bool Sphere3VsPlane3Core(const Sphere3& sph, const Plane& pl, Contact3& contact);
	static uint Sphere3VsPlane3Single(const Sphere3& sph, const Plane& pl, CollisionData3* data);
	static uint Sphere3VsPlane3Coherent(const Sphere3& sph, const Plane& pl, CollisionData3* data);
	static uint Sphere3VsPlane3Continuous(const Sphere3& sph, const Plane& pl, const Vector3& v, float& t, Vector3& hit);

	// obb3 vs plane
	static bool OBB3VsPlane3Core(const OBB3& obb, const Plane& pl, Contact3& contact);
	static uint OBB3VsPlane3Single(const OBB3& obb, const Plane& plane, CollisionData3* data);
	static uint OBB3VsPlane3Coherent(const OBB3& obb, const Plane& plane, CollisionData3* data);

	// obb3 vs sphere3
	static bool OBB3VsSphere3Core(const OBB3& obb, const Sphere3& sphere, Contact3& contact);
	static uint OBB3VsSphere3Single(const OBB3& obb, const Sphere3& sphere, CollisionData3* data);
	static uint OBB3VsSphere3Coherent(const OBB3& obb, const Sphere3& sphere, CollisionData3* data);

	// obb3 vs obb3
	static bool OBB3VsOBB3Intersected(const OBB3& obb_0, const OBB3& obb_1);
	static void OBB3VsOBB3CoreBreakdownPtVsFace(const OBB3& obb_0, const OBB3& obb_1, Vector3& pt, Vector3& face_center);
	static bool OBB3VsOBB3Core(const OBB3& obb_0, const OBB3& obb_1, Contact3& contact);
	static uint OBB3VsOBB3Single(const OBB3& obb1, const OBB3& obb2, CollisionData3* data);
	static uint OBB3VsOBB3Coherent(const OBB3& obb1, const OBB3& obb2, CollisionData3* data);
	static bool OBB3VsOBB3StepOne(const OBB3& obb1, const OBB3& obb2, std::tuple<Vector3, Vector3, float>& c_info2, OBB3Face& obb1_face, OBB3Vert& obb2_vert);
	static bool OBB3VsOBB3StepTwo(const OBB3& obb1, const OBB3& obb2, std::tuple<Vector3, Vector3, float>& c_info1, OBB3Face& obb2_face, OBB3Vert& obb1_vert);
	static bool OBB3VsOBB3StepThree(const OBB3& obb1, const OBB3& obb2, std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3>& c_info, OBB3Edge& obb1_edge, OBB3Edge& obb2_edge);

	// obb3 vs point
	static uint OBB3VsPoint(const OBB3& obb, const Vector3& p, Contact3& contact, bool reverse);
	
	// general entity detection
	//static uint Entity3VsEntity3(Entity3* e1, Entity3* e2, CollisionData3* data);
	static uint Rigid3VsRigid3(Rigidbody3* rb1, Rigidbody3* rb2, CollisionData3* data);

	// AABB deprecated
	// sphere vs aabb3
	static uint Sphere3VsAABB3(const Sphere3& sph, const AABB3& aabb3, CollisionData3* data);

	// aabb3 vs plane
	static uint AABB3VsPlane3Single(const AABB3& aabb, const Plane& plane, CollisionData3* data);
	static uint AABB3VsPlane3Coherent(const AABB3& aabb, const Plane& plane, CollisionData3* data);

	// aabb3 vs aabb3
	static uint AABB3VsAABB3Single(const AABB3& aabb3_1, const AABB3& aabb3_2, CollisionData3* data);
	static uint AABB3VsAABB3Coherent(const AABB3& aabb3_1, const AABB3& aabb3_2, CollisionData3* data);
};

extern Mesh* obb2_vert_to_obb1_face_0;
extern Mesh* obb2_vert_to_obb1_face_1;
extern Mesh* obb2_vert_to_obb1_face_2;
extern Mesh* obb2_vert_to_obb1_face_3;
extern Mesh* obb2_vert_to_obb1_face_4;
extern Mesh* obb2_vert_to_obb1_face_5;
extern Mesh* obb1_vert_to_obb2_face_0;
extern Mesh* obb1_vert_to_obb2_face_1;
extern Mesh* obb1_vert_to_obb2_face_2;
extern Mesh* obb1_vert_to_obb2_face_3;
extern Mesh* obb1_vert_to_obb2_face_4;
extern Mesh* obb1_vert_to_obb2_face_5;
extern Mesh* obb2_vert_0_winner;
extern Mesh* obb2_vert_1_winner;
extern Mesh* obb2_vert_2_winner;
extern Mesh* obb2_vert_3_winner;
extern Mesh* obb2_vert_4_winner;
extern Mesh* obb2_vert_5_winner;
extern Mesh* obb2_vert_6_winner;
extern Mesh* obb2_vert_7_winner;
extern Mesh* obb1_vert_0_winner;
extern Mesh* obb1_vert_1_winner;
extern Mesh* obb1_vert_2_winner;
extern Mesh* obb1_vert_3_winner;
extern Mesh* obb1_vert_4_winner;
extern Mesh* obb1_vert_5_winner;
extern Mesh* obb1_vert_6_winner;
extern Mesh* obb1_vert_7_winner;
extern Mesh* obb2_pt_obb1_face_winner;
extern Mesh* obb1_pt_obb2_face_winner;