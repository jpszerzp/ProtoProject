#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"
#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Physics/3D/PointEntity3.hpp"
#include "Engine/Physics/3D/BoxEntity3.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#define INVALID_DEPTH_BOX_TO_POINT -1.f
#define INVALID_DEPTH_EDGE_TO_EDGE -1.f

Contact3::Contact3()
{

}


Contact3::Contact3(Entity3* e1, Entity3* e2)
{
	m_e1 = e1;
	m_e2 = e2;
}

Contact3::Contact3(Entity3* e1, Entity3* e2, Vector3 normal, Vector3 point, float penetration)
{
	m_e1 = e1;
	m_e2 = e2;
	m_normal = normal;
	m_point = point;
	m_penetration = penetration;
}

Contact3::~Contact3()
{

}

/*
void Contact3::ResolveContactNaive()
{
	// normal is toward first entity
	GameObject* g1 = m_e1->GetGameobject();
	GameObject* g2 = m_e2->GetGameobject();

	// calculate proportions for entities in correcting positions
	if (!m_e1->IsConst() && !m_e2->IsConst())
	{
		float mass1 = m_e1->GetMassData3().m_mass;
		float mass2 = m_e2->GetMassData3().m_mass;
		Vector3 correction1 = m_normal * (m_penetration * (mass2 / (mass1 + mass2)));
		Vector3 correction2 = -m_normal * (m_penetration * (mass1 / (mass1 + mass2)));

		// game object position changed, driven by entity
		g1->EntityDriveTranslate(correction1);
		g2->EntityDriveTranslate(correction2);
	}
	else if (m_e1->IsConst() && !m_e2->IsConst())
	{
		Vector3 correction2 = -m_normal * m_penetration;
		g2->EntityDriveTranslate(correction2);
	}
	else if (!m_e1->IsConst() && m_e2->IsConst())
	{
		Vector3 correction1 = m_normal * m_penetration;
		g1->EntityDriveTranslate(correction1);
	}
	else 
	{
		// no correction of positions if both entities are const

	}
}
*/

void Contact3::ResolveContact(float deltaTime)
{
	ResolveVelocity(deltaTime);
	ResolvePenetration(deltaTime);
}

float Contact3::ComputeSeparatingVelocity() const
{
	Vector3 e1_vel = m_e1->GetLinearVelocity();
	Vector3 e2_vel;
	if (m_e2 != nullptr)
		e2_vel = m_e2->GetLinearVelocity();
	Vector3 rel_vel = e1_vel - e2_vel;
	return DotProduct(rel_vel, m_normal);
}

void Contact3::ResolveVelocity(float deltaTime)
{
	float separate = ComputeSeparatingVelocity();
	// separating or stationary
	if (separate > 0)
		return;

	// Calculate the new separating velocity and delta velocity
	float new_separate = -separate * m_restitution;

	// resolve resting contact 
	// Check the velocity build-up due to acceleration only.
	Vector3 e1_acc = m_e1->GetLinearAcceleration();
	Vector3 e2_acc = Vector3::ZERO;
	if (m_e2 != nullptr)
		e2_acc = m_e2->GetLinearAcceleration();
	Vector3 rel_acc = e1_acc - e2_acc;
	float rel_vel = DotProduct(rel_acc, m_normal) * deltaTime;
	// If we’ve got a closing velocity due to acceleration build-up,
	// remove it from the new separating velocity.
	if (rel_vel < 0.f)
	{
		new_separate += m_restitution * rel_vel;
		// Make sure we haven’t removed more than was there to remove.
		if (new_separate < 0) 
			new_separate = 0;
	}

	float delta_vel = new_separate - separate;

	// We apply the change in velocity to each object in proportion to
	// its inverse mass (i.e., those with lower inverse mass [higher
	// actual mass] get less change in velocity).
	float e1_inv_mass = m_e1->GetMassData3().m_invMass;
	float e2_inv_mass = 0.f;
	if (m_e2 != nullptr)
		e2_inv_mass = m_e2->GetMassData3().m_invMass;
	float total_inv_mass = e1_inv_mass + e2_inv_mass;
	// If all particles have infinite mass, then impulses have no effect.
	if (total_inv_mass <= 0.f) 
		return;
	// Calculate the impulse to apply.
	float impulse_amount = delta_vel / total_inv_mass;
	// Find the amount of impulse per unit of inverse mass.
	Vector3 impulse_per_inv_mass = m_normal * impulse_amount;


	// Apply impulses: they are applied in the direction of the contact,
	// and are proportional to the inverse mass.
	if (!m_e1->IsEntityStatic() && !m_e1->IsEntityKinematic())
		m_e1->SetLinearVelocity(m_e1->GetLinearVelocity() +
			impulse_per_inv_mass * m_e1->GetMassData3().m_invMass);
	if (m_e2 != nullptr)
	{
		if (!m_e2->IsEntityStatic() && !m_e2->IsEntityKinematic())
			// Particle 1 goes in the opposite direction.
			m_e2->SetLinearVelocity(m_e2->GetLinearVelocity() +
				impulse_per_inv_mass * (-m_e2->GetMassData3().m_invMass));
	}
}

void Contact3::ResolvePenetration(float deltaTime)
{
	// If we don’t have any penetration, skip this step.
	if (m_penetration <= 0) 
		return;
	// The movement of each object is based on its inverse mass, so
	// total that.
	float e1_inv_mass = m_e1->GetMassData3().m_invMass;
	float e2_inv_mass = 0.f;
	if (m_e2 != nullptr)
		e2_inv_mass = m_e2->GetMassData3().m_invMass;
	float total_inv_mass = e1_inv_mass + e2_inv_mass;
	// If all particles have infinite mass, then we do nothing.
	if (total_inv_mass <= 0) 
		return;
	// Find the amount of penetration resolution per unit of inverse mass.
	Vector3 move_per_inv_mass = m_normal * (m_penetration / total_inv_mass);
	// Apply the penetration resolution.
	if (!m_e1->IsEntityStatic())
		m_e1->SetEntityCenter(m_e1->GetEntityCenter() +
			move_per_inv_mass * m_e1->GetMassData3().m_invMass);
	if (m_e2 != nullptr)
	{
		if (!m_e2->IsEntityStatic())
			m_e2->SetEntityCenter(m_e2->GetEntityCenter() +
				move_per_inv_mass * (-m_e2->GetMassData3().m_invMass));
	}
}

CollisionDetector::CollisionDetector()
{

}

CollisionDetector::~CollisionDetector()
{

}

uint CollisionDetector::Sphere3VsSphere3(const Sphere3& s1, const Sphere3& s2, CollisionData3* data)
{
	if (data->m_contacts.size() > data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	Vector3 s1Pos = s1.m_center;
	Vector3 s2Pos = s2.m_center;
	float s1Rad = s1.m_radius;
	float s2Rad = s2.m_radius;

	Vector3 midLine = s1Pos - s2Pos;
	float length = midLine.GetLength();

	if (length <= 0.f || length >= (s1Rad + s2Rad))
	{
		// if mid line length is invalid
		// or, larger than radius threshold, return directly
		return 0;
	}

	// get normal
	Vector3 normal = midLine.GetNormalized();
	Vector3 point = s2Pos + midLine * 0.5f;
	float penetration = s1Rad + s2Rad - length;

	Contact3 theContact = Contact3(s1.GetEntity(), s2.GetEntity(),
		normal, point, penetration);
	data->m_contacts.push_back(theContact);

	return 1;
}

uint CollisionDetector::Sphere3VsPlane3(const Sphere3& sph, const Plane& pl, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	float sphereRad = sph.GetRadius();
	Vector3 spherePos = sph.GetCenter();
	Vector3 planeNormal = pl.GetNormal().GetNormalized();			// guarantee to be normalized
	float sphereToOriginAlongPlaneDir = DotProduct(planeNormal, spherePos);
	float signedDistToPlane = sphereToOriginAlongPlaneDir - pl.GetOffset();
	if (abs(signedDistToPlane) >= sphereRad)
	{
		return 0;
	}

	// we are certain that there is some contact between sphere and plane

	// check which side of plane we are on
	Vector3 usedNormal = planeNormal;
	float penetration = -signedDistToPlane;
	if (signedDistToPlane < 0)
	{
		usedNormal *= -1.f;
		penetration = -penetration;
	}
	penetration += sph.GetRadius();

	Vector3 contactPoint = spherePos - planeNormal * signedDistToPlane;
	Contact3 theContact = Contact3(sph.GetEntity(), pl.GetEntity(),
		usedNormal, contactPoint, penetration);
	data->m_contacts.push_back(theContact);

	return 1;
}

uint CollisionDetector::Sphere3VsAABB3(const Sphere3& sph, const AABB3& aabb3, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	// transform sphere to aabb3 coord
	// get transform of aabb3 first and transform the basis from that
	const Transform& aabb3Transform = aabb3.GetEntity()->GetEntityTransform();

	float sphereRad = sph.GetRadius();
	Vector3 worldSpherePos = sph.GetCenter();
	Vector3 localSpherePos = Transform::TransformWorldToLocalPosOrthogonal(worldSpherePos, aabb3Transform);
	Vector3 aabb3HalfDim = aabb3.GetDimensions() / 2.f;

	// early out in aabb3 coord
	bool xtest = (abs(localSpherePos.x) - sphereRad) > aabb3HalfDim.x;
	bool ytest = (abs(localSpherePos.y) - sphereRad) > aabb3HalfDim.y;
	bool ztest = (abs(localSpherePos.z) - sphereRad) > aabb3HalfDim.z;
	if (xtest || ytest || ztest)
	{
		return 0;
	}

	Vector3 closestPointLocal = Vector3::ZERO;
	float dist;

	// clamp x
	dist = localSpherePos.x;
	if (dist > aabb3HalfDim.x) dist = aabb3HalfDim.x;
	if (dist < -aabb3HalfDim.x) dist = -aabb3HalfDim.x;
	closestPointLocal.x = dist;

	// clamp y
	dist = localSpherePos.y;
	if (dist > aabb3HalfDim.y) dist = aabb3HalfDim.y;
	if (dist < -aabb3HalfDim.y) dist = -aabb3HalfDim.y;
	closestPointLocal.y = dist;

	// clamp z
	dist = localSpherePos.z;
	if (dist > aabb3HalfDim.z) dist = aabb3HalfDim.z;
	if (dist < -aabb3HalfDim.z) dist = -aabb3HalfDim.z;
	closestPointLocal.z = dist;

	// check if we are in contact
	dist = (closestPointLocal - localSpherePos).GetLengthSquared();
	if (dist > sphereRad * sphereRad) return 0;

	// at this point we get the contact point in the local space
	// we need to transform it back to world space
	Vector3 closestPointWorld = Transform::TransformLocalToWorldPos(closestPointLocal, aabb3Transform);

	Vector3 usedNormal = (worldSpherePos - closestPointWorld).GetNormalized();
	Vector3 contactPoint = closestPointWorld;
	float penetration = sphereRad - sqrtf(dist);
	Contact3 theContact = Contact3(sph.GetEntity(), aabb3.GetEntity(),
		usedNormal, contactPoint, penetration);
	data->m_contacts.push_back(theContact);

	return 1;
}

uint CollisionDetector::AABB3VsAABB3(const AABB3& aabb3_1, const AABB3& aabb3_2, CollisionData3* data)
{
	/////////////////////////////////////// NON-COHERENCE PROCESSING ///////////////////////////////////////
	//uint succeed = NonCoherentProcessingAABB3VsAABB3(aabb3_1, aabb3_2, data);
	/////////////////////////////////////// END ///////////////////////////////////////

	/////////////////////////////////////// COHERENCE PROCESSING ///////////////////////////////////////
	//uint succeed = CoherentProcessingAABB3VsAABB3(aabb3_1, aabb3_2, data);	// comment out one or the other for coherency
	/////////////////////////////////////// END ///////////////////////////////////////

	return 0;
}

uint CollisionDetector::AABB3VsAABB3Single(const AABB3& aabb3_1, const AABB3& aabb3_2, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	Vector3 overlap_axis;
	float penetration;
	bool overlapped = AABB3VsAABB3Intersection(aabb3_1, aabb3_2, overlap_axis, penetration);

	if (!overlapped)
		return 0;

	// get normal
	Vector3 second_to_first_aabb3_center = aabb3_1.GetCenter() - aabb3_2.GetCenter();
	overlap_axis.NormalizeAndGetLength();
	float extend = DotProduct(second_to_first_aabb3_center, overlap_axis);
	Vector3 usedNormal = (overlap_axis * extend).GetNormalized();

	// get contact point
	Vector3 halfDim = aabb3_1.GetHalfDimensions();
	Vector3 to_second_normal = -usedNormal;
	float abs_half_dim = abs(DotProduct(halfDim, to_second_normal));
	Vector3 first_half_dim_along_normal = to_second_normal * abs_half_dim;
	Vector3 to_second_overlap = to_second_normal * penetration;
	Vector3 to_contact_point = first_half_dim_along_normal - to_second_overlap;
	Vector3 contactPoint = aabb3_1.GetCenter() + to_contact_point;

	// create contact
	Contact3 theContact = Contact3(aabb3_1.GetEntity(), aabb3_2.GetEntity(),
		usedNormal, contactPoint, penetration);
	data->m_contacts.push_back(theContact);

	return 1;
}

uint CollisionDetector::OBB3VsPlane3(const OBB3& obb, const Plane& plane, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
		// no contacts amount left, return directly
		return 0;

	float r = obb.m_halfExt.x * abs(DotProduct(plane.m_normal, obb.m_right)) +
		obb.m_halfExt.y * abs(DotProduct(plane.m_normal, obb.m_up)) +
		obb.m_halfExt.z * abs(DotProduct(plane.m_normal, obb.m_forward));
	float s = DotProduct(plane.m_normal, obb.m_center) - plane.m_offset;
	
	if (abs(s) > r)
		return 0;
	else
	{
		float penetration = r - abs(s);		// > 0
		Vector3 usedNormal;
		if (s > 0)
			usedNormal = plane.GetNormal();
		else
			usedNormal = -plane.GetNormal();
		Vector3 contactPoint = obb.GetCenter() + (-usedNormal) * s;
		Contact3 theContact = Contact3(obb.GetEntity(), plane.GetEntity(),
			usedNormal, contactPoint, penetration);

		data->m_contacts.push_back(theContact);
		return 1;
	}
}

uint CollisionDetector::OBB3VsSphere3(const OBB3& obb, const Sphere3& sphere, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
		// no contacts amount left, return directly
		return 0;

	// get obb transform
	BoxEntity3* boxEnt = dynamic_cast<BoxEntity3*>(obb.GetEntity());
	ASSERT_OR_DIE(boxEnt != nullptr, "OBB is not using particle entity, try rigid body");
	const Transform& t = boxEnt->GetEntityTransform();
	
	// transform sphere to local coord of obb 
	Vector3 center_local = Transform::WorldToLocalOrthogonal(sphere.GetCenter(), t);
	float r = sphere.GetRadius();
	Sphere3 local_sph = Sphere3(center_local, r);

	// construct local aabb
	Vector3 halfExt = obb.GetHalfExt();
	AABB3 local_box = AABB3(-halfExt, halfExt);
	Vector3 aabb3HalfDim = local_box.GetDimensions() / 2.f;

	// early out in aabb3 coord
	bool xtest = (abs(center_local.x) - r) > aabb3HalfDim.x;
	bool ytest = (abs(center_local.y) - r) > aabb3HalfDim.y;
	bool ztest = (abs(center_local.z) - r) > aabb3HalfDim.z;
	if (xtest || ytest || ztest)
		return false;

	Vector3 closestPointLocal = Vector3::ZERO;
	float dist;

	// clamp x
	dist = center_local.x;
	if (dist > aabb3HalfDim.x) dist = aabb3HalfDim.x;
	if (dist < -aabb3HalfDim.x) dist = -aabb3HalfDim.x;
	closestPointLocal.x = dist;

	// clamp y
	dist = center_local.y;
	if (dist > aabb3HalfDim.y) dist = aabb3HalfDim.y;
	if (dist < -aabb3HalfDim.y) dist = -aabb3HalfDim.y;
	closestPointLocal.y = dist;

	// clamp z
	dist = center_local.z;
	if (dist > aabb3HalfDim.z) dist = aabb3HalfDim.z;
	if (dist < -aabb3HalfDim.z) dist = -aabb3HalfDim.z;
	closestPointLocal.z = dist;

	// check if we are in contact
	dist = (closestPointLocal - center_local).GetLengthSquared();
	if (dist > r * r) return 0;

	// at this point we get the contact point in the local space
	// we need to transform it back to world space
	Vector3 closestPointWorld = Transform::LocalToWorldPos(closestPointLocal, t);

	Vector3 usedNormal = (sphere.GetCenter() - closestPointWorld).GetNormalized();
	Vector3 contactPoint = closestPointWorld;
	float penetration = r - sqrtf(dist);
	Contact3 theContact = Contact3(sphere.GetEntity(), obb.GetEntity(),
		usedNormal, contactPoint, penetration);

	data->m_contacts.push_back(theContact);
	return 1;
}

uint CollisionDetector::OBB3VsOBB3(const OBB3& obb1, const OBB3& obb2, CollisionData3* data)
{
	return 0;
}

uint CollisionDetector::Entity3VsEntity3(Entity3* e1, Entity3* e2, CollisionData3* data)
{
	uint res = 0;

	SphereEntity3* s1 = dynamic_cast<SphereEntity3*>(e1);
	SphereRB3* srb1 = dynamic_cast<SphereRB3*>(e1);
	CubeEntity3* c1 = dynamic_cast<CubeEntity3*>(e1);
	QuadEntity3* q1 = dynamic_cast<QuadEntity3*>(e1);
	//PointEntity3* p1 = dynamic_cast<PointEntity3*>(e1);

	SphereEntity3* s2 = dynamic_cast<SphereEntity3*>(e2);
	SphereRB3* srb2 = dynamic_cast<SphereRB3*>(e2);
	CubeEntity3* c2 = dynamic_cast<CubeEntity3*>(e2);
	QuadEntity3* q2 = dynamic_cast<QuadEntity3*>(e2);
	//PointEntity3* p2 = dynamic_cast<PointEntity3*>(e2);

	if (s1 != nullptr)
	{
		if (s2 != nullptr)
		{
			const Sphere3& sph1 = s1->GetSpherePrimitive();
			const Sphere3& sph2 = s2->GetSpherePrimitive();

			res = Sphere3VsSphere3(sph1, sph2, data);
		}
		else if (srb2 != nullptr)
		{
			const Sphere3& sph1 = s1->GetSpherePrimitive();
			const Sphere3& sph2 = srb2->GetSpherePrimitive();

			res = Sphere3VsSphere3(sph1, sph2, data);
		}
		else if (c2 != nullptr)
		{
			const Sphere3& sph = s1->GetSpherePrimitive();
			const AABB3& aabb3 = c2->GetCubePrimitive();

			res = Sphere3VsAABB3(sph, aabb3, data);
		}
		else if (q2 != nullptr)
		{
			const Sphere3& sph = s1->GetSpherePrimitive();
			const Plane& pl = q2->GetPlanePrimitive();

			res = Sphere3VsPlane3(sph, pl, data);
		}
	}
	else if (srb1 != nullptr)
	{
		if (s2 != nullptr)
		{
			const Sphere3& sph1 = srb1->GetSpherePrimitive();
			const Sphere3& sph2 = s2->GetSpherePrimitive();

			res = Sphere3VsSphere3(sph1, sph2, data);
		}
		else if (srb2 != nullptr)
		{
			const Sphere3& sph1 = srb1->GetSpherePrimitive();
			const Sphere3& sph2 = srb2->GetSpherePrimitive();

			res = Sphere3VsSphere3(sph1, sph2, data);
		}
		else if (c2 != nullptr)
		{
			const Sphere3& sph = srb1->GetSpherePrimitive();
			const AABB3& aabb3 = c2->GetCubePrimitive();

			res = Sphere3VsAABB3(sph, aabb3, data);
		}
		else if (q2 != nullptr)
		{
			const Sphere3& sph = srb1->GetSpherePrimitive();
			const Plane& pl = q2->GetPlanePrimitive();

			res = Sphere3VsPlane3(sph, pl, data);
		}
	}
	else if (c1 != nullptr)
	{
		if (s2 != nullptr)
		{
			const AABB3& aabb3 = c1->GetCubePrimitive();
			const Sphere3& sph = s2->GetSpherePrimitive();

			res = Sphere3VsAABB3(sph, aabb3, data);
		}
		else if (srb2 != nullptr)
		{
			const AABB3& aabb3 = c1->GetCubePrimitive();
			const Sphere3& sph = srb2->GetSpherePrimitive();

			res = Sphere3VsAABB3(sph, aabb3, data);
		}
		else if (c2 != nullptr)
		{
			const AABB3& aabb3_1 = c1->GetCubePrimitive();
			const AABB3& aabb3_2 = c2->GetCubePrimitive();

			res = AABB3VsAABB3Single(aabb3_1, aabb3_2, data);
		}
		else if (q2 != nullptr)
		{
			const AABB3& aabb3 = c1->GetCubePrimitive();
			const Plane& pl = q2->GetPlanePrimitive();

			res = AABB3VsPlane3Single(aabb3, pl, data);
		}
	}
	else if (q1 != nullptr)
	{
		if (s2 != nullptr)
		{
			const Plane& pl = q1->GetPlanePrimitive();
			const Sphere3& sph = s2->GetSpherePrimitive();

			res = Sphere3VsPlane3(sph, pl, data);
		}
		else if (srb2 != nullptr)
		{
			const Plane& pl = q1->GetPlanePrimitive();
			const Sphere3& sph = srb2->GetSpherePrimitive();

			res = Sphere3VsPlane3(sph, pl, data);
		}
		else if (c2 != nullptr)
		{
			const Plane& pl = q1->GetPlanePrimitive();
			const AABB3& aabb = c2->GetCubePrimitive();

			res = AABB3VsPlane3Single(aabb, pl, data);
		}
		else if (q2 != nullptr)
		{
			TODO("Later deal with quad vs quad");
		}
	}

	return res;
}

/*
uint CollisionDetector::NonCoherentProcessingAABB3VsAABB3(const AABB3& aabb3_1, const AABB3& aabb3_2, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	// early out with SAT
	bool overlapped = AABB3VsAABB3Intersection(aabb3_1, aabb3_2);
	if (!overlapped) return 0;

	// two aabbs do overlap
	// determine one contact to use, either point-face or edge-edge

	Vector3 vertices_1[8];
	aabb3_1.GetVertices(vertices_1);
	float vert_depth[8];

	// go over vertices of 1 to add depths against 2
	for (uint i = 0; i < 8; ++i)
	{
		float vp = INVALID_DEPTH_BOX_TO_POINT;
		CollisionDetector::BoxAndPointPenetration(aabb3_2, vertices_1[i], &vp);

		vert_depth[i] = vp;
	}

	Vector3 edges_1[12];
	aabb3_1.GetEdges(edges_1);
	float edge_depth[12];			// record edge penetration for all edges of 1

	// go over edges of 1 to add edge depths against 2 (against 2's edges under the hook)
	for (uint i = 0; i < 12; ++i)
	{
		float ep = INVALID_DEPTH_EDGE_TO_EDGE;
		CollisionDetector::BoxAndEdgePenetration(aabb3_2, edges_1[i], &ep);

		edge_depth[i] = ep;
	}

	// get the deepest depth and its relevant information of both situations
	//int v_large_index = PickLargestElementIndexArray(vert_depth, 8);
	//int e_large_index = PickLargestElementIndexArray(edge_depth, 8);
	int v_large_index = 0;
	int e_large_index = 0;
	float v_large_depth = vert_depth[v_large_index];
	float e_large_depth = edge_depth[e_large_index];

	if (v_large_depth > e_large_depth)
	{
		// point-face depth larger than edge-edge depth
		uint succeeded = CollisionDetector::BoxAndPointContact(aabb3_2, vertices_1[v_large_index], data);
		return succeeded;
	}
	else 
	{
		uint succeeded = CollisionDetector::BoxAndEdgeContact(aabb3_2, edges_1[e_large_index], data);
		return succeeded;
	}
}
*/

//uint CollisionDetector::CoherentProcessingAABB3VsAABB3(const AABB3& aabb_1, const AABB3& aabb3_2, CollisionData3* data)
//{
//	/*
//	// go over vertices of 1 to check penetration against 2
//	for (uint i = 0; i < 8; ++i)
//	{
//	float penetration;
//	Vector3 vert_1 = vertices_1[i];
//	uint pointInBox = CollisionDetector::AABB3AndPointPenetration(aabb3_2, vert_1, &penetration);
//
//	if (pointInBox == 1)
//	{
//	// point is in box, take the shallowest penetration along axes
//	vertices_1_depth[i] = penetration;
//	}
//	else
//	{
//	// point is out of box
//	vertices_1_depth[i] = INVALID_DEPTH_BOX_TO_POINT;
//	}
//	}
//
//	// go over vertices of 2 to check penetration against 1
//	for (uint j = 0; j < 8; ++j)
//	{
//	float penetration;
//	Vector3 vert_2 = vertices_2[j];
//	uint pointInBox = CollisionDetector::AABB3AndPointPenetration(aabb3_1, vert_2, &penetration);
//
//	if (pointInBox == 1)
//	{
//	vertices_2_depth[j] = penetration;
//	}
//	else
//	{
//	vertices_2_depth[j] = INVALID_DEPTH_BOX_TO_POINT;
//	}
//	}
//
//	// pick index of deepest penetrations
//	int deep_ind_1 = PickLargestElementIndexArray(vertices_1_depth, 8);
//	int deep_ind_2 = PickLargestElementIndexArray(vertices_2_depth, 8);
//	float depth_1 = vertices_1_depth[deep_ind_1];
//	float depth_2 = vertices_2_depth[deep_ind_2];
//
//	if (depth_1 > depth_2)
//	{
//	// use a vert of aabb_1 against aabb_2
//	CollisionDetector::AABB3AndPointContact(aabb3_2, vertices_1[deep_ind_1], &pointToBoxDeepestContact);
//	}
//	else
//	{
//	CollisionDetector::AABB3AndPointContact(aabb3_1, vertices_2[deep_ind_2], &pointToBoxDeepestContact);
//	}
//	// at this point we have deepest contact for face-point case
//	*/
//
//	return 0;
//}

uint CollisionDetector::AABB3VsPlane3(const AABB3& aabb, const Plane& plane, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	bool intersected = AABB3VsPlaneIntersection(aabb, plane);

	if (!intersected)
	{
		return 0;
	}
	else
	{
		// there is/are some vertex/vertices of aabb intersecting the plane
		Vector3 center = aabb.GetCenter();
		Vector3 vertices[8];
		aabb.GetVertices(vertices);

		// we need to know which one/ones is/are colliding exactly
		for (uint i = 0; i < 8; ++i)
		{
			Vector3 vert = vertices[i];

			float vertDot = DotProduct(vert, plane.GetNormal().GetNormalized());
			float signedVertDist = vertDot - plane.GetOffset();

			float centerDot = DotProduct(center, plane.GetNormal().GetNormalized());
			float signedCenterDist = centerDot - plane.GetOffset();

			// check aabb is colliding from which side of plane
			// if aabb is colliding from front side of plane
			if (signedCenterDist > 0)
			{
				if (signedVertDist < 0 && data->m_contacts.size() < data->m_maxContacts)
				{
					// collided
					Vector3 usedNormal;
					float penetration;
					Vector3 contactPt;			// contact point is on the plane

					usedNormal = plane.GetNormal().GetNormalized();
					penetration = -signedVertDist;
					contactPt = vert + usedNormal * penetration;

					Contact3 theContact = Contact3(aabb.GetEntity(), plane.GetEntity(),
						usedNormal, contactPt, penetration);
					data->m_contacts.push_back(theContact);
				}
				else
				{
					// this vertex not colliding, check the next one
					continue;
				}
			}

			// if aabb is colliding from back side of plane
			else
			{
				if (signedVertDist > 0 && data->m_contacts.size() < data->m_maxContacts)
				{
					// collided
					Vector3 usedNormal;
					float penetration;
					Vector3 contactPt;			// contact point is on the plane

					usedNormal = -plane.GetNormal().GetNormalized();
					penetration = signedVertDist;
					contactPt = vert + usedNormal * penetration;

					Contact3 theContact = Contact3(aabb.GetEntity(), plane.GetEntity(),
						usedNormal, contactPt, penetration);
					data->m_contacts.push_back(theContact);
				}
				else
				{
					continue;
				}
			}
		}

		return 1;
	}
}


uint CollisionDetector::AABB3VsPlane3Single(const AABB3& aabb, const Plane& plane, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
	{
		// no contacts amount left, return directly
		return 0;
	}

	bool intersected = AABB3VsPlaneIntersection(aabb, plane);

	if (!intersected)
	{
		return 0;
	}
	else
	{
		// keep the deepest penetration
		Contact3 used_contact = Contact3();
		used_contact.m_e1 = aabb.GetEntity();
		used_contact.m_e2 = plane.GetEntity();
		float max_penetration = -INFINITY;
		// there is/are some vertex/vertices of aabb intersecting the plane
		Vector3 center = aabb.GetCenter();
		Vector3 vertices[8];
		aabb.GetVertices(vertices);

		// we need to know which one/ones is/are colliding exactly
		for (uint i = 0; i < 8; ++i)
		{
			Vector3 vert = vertices[i];

			float vertDot = DotProduct(vert, plane.GetNormal().GetNormalized());
			float signedVertDist = vertDot - plane.GetOffset();

			float centerDot = DotProduct(center, plane.GetNormal().GetNormalized());
			float signedCenterDist = centerDot - plane.GetOffset();

			// check aabb is colliding from which side of plane
			// if aabb is colliding from front side of plane
			if (signedCenterDist > 0)
			{
				if (signedVertDist < 0 && data->m_contacts.size() < data->m_maxContacts)
				{
					if (abs(signedVertDist) > max_penetration)
					{
						// collided
						Vector3 usedNormal;
						float penetration;
						Vector3 contactPt;			// contact point is on the plane

						usedNormal = plane.GetNormal().GetNormalized();
						penetration = -signedVertDist;
						contactPt = vert + usedNormal * penetration;

						used_contact.m_normal = usedNormal;
						used_contact.m_point = contactPt;
						used_contact.m_penetration = penetration;

						//data->m_contacts.push_back(used_contact);
					}
				}
				else
				{
					// this vertex not colliding, check the next one
					continue;
				}
			}

			// if aabb is colliding from back side of plane
			else
			{
				if (signedVertDist > 0 && data->m_contacts.size() < data->m_maxContacts)
				{
					if (signedVertDist > max_penetration)
					{
						// collided
						Vector3 usedNormal;
						float penetration;
						Vector3 contactPt;			// contact point is on the plane

						usedNormal = -plane.GetNormal().GetNormalized();
						penetration = signedVertDist;
						contactPt = vert + usedNormal * penetration;

						used_contact.m_normal = usedNormal;
						used_contact.m_point = contactPt;
						used_contact.m_penetration = penetration;

						//data->m_contacts.push_back(used_contact);
					}
				}
				else
				{
					continue;
				}
			}
		}

		data->m_contacts.push_back(used_contact);
		return 1;
	}
}

void CollisionDetector::BoxAndPointPenetration(const AABB3& aabb, const Vector3& point, float* out_penetration)
{
	// todo: generate box and point penetration, if invalid leave the out value be

}

void CollisionDetector::BoxAndEdgePenetration(const AABB3& aabb, const Vector3& edge, float* out_penetration)
{
	// todo: go over all edges of aabb against input edge, retrieve the shallowest penetration E 

}

void CollisionDetector::EdgeAndEdgePenetration(const Vector3& e1, const Vector3& e2, float* out_penetration)
{
	// todo: retrieve the penetration of the pair of edge

}

uint CollisionDetector::BoxAndPointContact(const AABB3& aabb, const Vector3& point, CollisionData3* data)
{
	// todo: contact between a box and a point
	return 0;
}

uint CollisionDetector::BoxAndEdgeContact(const AABB3& aabb, const Vector3& edge, CollisionData3* data)
{
	// todo: contact between a box and an edge
	return 0;
}

uint CollisionDetector::EdgeAndEdgeContact(const Vector3& edge1, const Vector3& edge2, CollisionData3* data)
{
	// todo: contact between an edge and an edge
	return 0;
}
