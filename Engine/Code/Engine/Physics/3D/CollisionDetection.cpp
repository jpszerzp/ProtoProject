#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"
#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Physics/3D/PointEntity3.hpp"
#include "Engine/Physics/3D/BoxEntity3.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Line3.hpp"

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

void CollisionData3::ClearCoherent()
{
	for (std::vector<Contact3>::size_type idx = 0; idx < m_contacts.size(); ++idx)
	{
		if (m_contacts[idx].m_penetration < COHERENT_THRESHOLD)
		{
			std::vector<Contact3>::iterator it = m_contacts.begin() + idx;
			m_contacts.erase(it);
			idx--;
		}
	}
}


bool CollisionData3::HasAndUpdateContact(const Contact3& contact)
{
	bool found = false;

	for (Contact3& c : m_contacts)
	{
		if (contact.m_type == POINT_FACE)
		{
			if (contact.m_f1 == UNKNOWN)
			{
				found = (contact.m_f2 == c.m_f2);
				if (found)
				{
					c = contact;
					break;
				}
			}
			else 
			{
				found = (contact.m_f1 == c.m_f1);
				if (found)
				{
					c = contact;
					break;
				}
			}
		}
		else if (contact.m_type == EDGE_EDGE)
		{
			found = (contact.m_f1 == c.m_f1) && (contact.m_f2 == c.m_f2);
			if (found)
			{
				c = contact;
				break;
			}
		}
		else
		{
			// we do not care if contact has other types
			// we will not use this function in those cases either
			break;
		}
	}

	return found;
}

const Vector3 CollisionDetector::ISA = Vector3(-INFINITY);

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

uint CollisionDetector::OBB3VsOBB3Shallow(const OBB3& obb1, const OBB3& obb2, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
		// no contacts amount left, return directly
		return 0;

	Vector3 obb1_local[3] = {obb1.m_right, obb1.m_up, obb1.m_forward};	// x y z
	Vector3 obb2_local[3] = {obb2.m_right, obb2.m_up, obb2.m_forward};

	// SAT, if fails, no collision
	// 1 - rotation matrix where obb2 is expressed with obb1 coord (obb2 -> obb1)
	float entries[9];
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int idx = 3 * j + i;
			entries[idx] = DotProduct(obb1_local[i], obb2_local[j]);
		}
	}
	Matrix33 rotation = Matrix33(entries);

	// 2 - find translation t from obb1 to obb2, and use that to translate to obb1's frame
	Vector3 t = obb2.m_center - obb1.m_center;
	t = Vector3(DotProduct(t, obb1.m_right), 
		DotProduct(t, obb1.m_up),
		DotProduct(t, obb1.m_forward));

	// 3 - robust SAT
	float abs_entries[9];
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int idx = 3 * j + i;
			abs_entries[idx] = abs(entries[idx]) + .0f;		// offset to prevent parallel axis but cause representation accuracy issue
		}
	}
	Matrix33 abs_rotation = Matrix33(abs_entries);
	// axis record 
	IntVector2 axis_pair;
	//float deepest = -INFINITY;
	float shallowest = INFINITY;

	// 4 - test obb1 basis
	float r1, r2;
	for (int i = 0; i < 3; ++i)
	{
		r1 = obb1.GetHalfExtCopy()[i];
		r2 = obb2.GetHalfExtCopy()[0] * abs_rotation[i] +
			obb2.GetHalfExtCopy()[1] * abs_rotation[3 + i] +
			obb2.GetHalfExtCopy()[2] * abs_rotation[6 + i];
		if (abs(t[i]) > (r1 + r2))
			return 0;
		else
		{
			float overlap = (r1 + r2) - abs(t[i]);
			//if (overlap > deepest)
			//{
			//	deepest = overlap;
			//	axis_pair = IntVector2(i, -1);	// means the i axis of obb1, no axis from obb2
			//}
			if (overlap < shallowest)
			{
				shallowest = overlap;
				axis_pair = IntVector2(i, -1);
			}
		}
	}

	// 5 - test obb2 basis
	for (int i = 0; i < 3; ++i)
	{
		r1 = obb1.GetHalfExtCopy()[0] * abs_rotation[3 * i] +
			obb1.GetHalfExtCopy()[1] * abs_rotation[3 * i + 1] +
			obb1.GetHalfExtCopy()[2] * abs_rotation[3 * i + 2];
		r2 = obb2.GetHalfExtCopy()[i];
		float dist = abs(t[0] * rotation[3*i] + t[1] * rotation[3*i+1] + t[2] * rotation[3*i+2]); 
		if (dist > (r1 + r2))
			return 0;
		else
		{
			float overlap = (r1 + r2) - dist;
			//if (overlap > deepest)
			//{
			//	deepest = overlap;
			//	axis_pair = IntVector2(-1, i);	// means no axis from obb1, and i axis from obb2
			//}
			if (overlap < shallowest)
			{
				shallowest = overlap;
				axis_pair = IntVector2(-1, i);
			}
		}
	}

	// 6 - test axis obb1x cross obb2x
	r1 = obb1.m_halfExt[1] * abs_rotation[2] + obb1.m_halfExt[2] * abs_rotation[1];
	r2 = obb2.m_halfExt[1] * abs_rotation[6] + obb2.m_halfExt[2] * abs_rotation[3];
	float dist = abs(t[2] * rotation[1] - t[1] * rotation[2]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(0, 0);	// x axis from both
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(0, 0);
		}
	}

	// 7 - test axis obb1x cross obb2y
	r1 = obb1.m_halfExt[1] * abs_rotation[5] + obb1.m_halfExt[2] * abs_rotation[4];
	r2 = obb2.m_halfExt[0] * abs_rotation[6] + obb2.m_halfExt[2] * abs_rotation[0];
	dist = abs(t[2] * rotation[4] - t[1] * rotation[5]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(0, 1);	// x from 1, y from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(0, 1);
		}
	}

	// 8 - test axis obb1x cross obb2z
	r1 = obb1.m_halfExt[1] * abs_rotation[8] + obb1.m_halfExt[2] * abs_rotation[7];
	r2 = obb2.m_halfExt[0] * abs_rotation[3] + obb2.m_halfExt[1] * abs_rotation[0];
	dist = abs(t[2] * rotation[7] - t[1] * rotation[8]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(0, 2);	// x from 1, z from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(0, 2);
		}
	}

	// 9 - test axis obb1y cross obb2x
	r1 = obb1.m_halfExt[0] * abs_rotation[2] + obb1.m_halfExt[2] * abs_rotation[0];
	r2 = obb2.m_halfExt[1] * abs_rotation[7] + obb2.m_halfExt[2] * abs_rotation[4];
	dist = abs(t[0] * rotation[2] - t[2] * rotation[0]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(1, 0);	// y from 1, x from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(1, 0);
		}
	}

	// 10 - test axis obb1y cross obb2y
	r1 = obb1.m_halfExt[0] * abs_rotation[5] + obb1.m_halfExt[2] * abs_rotation[3];
	r2 = obb2.m_halfExt[0] * abs_rotation[7] + obb2.m_halfExt[2] * abs_rotation[1];
	dist = abs(t[0] * rotation[5] - t[2] * rotation[3]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(1, 1);	// y from 1, y from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(1, 1);
		}
	}

	// 11 - test axis obb1y cross obb2z
	r1 = obb1.m_halfExt[0] * abs_rotation[8] + obb1.m_halfExt[2] * abs_rotation[6];
	r2 = obb2.m_halfExt[0] * abs_rotation[4] + obb2.m_halfExt[1] * abs_rotation[1];
	dist = abs(t[0] * rotation[8] - t[2] * rotation[6]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(1, 2);	// y from 1, z from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(1, 2);
		}
	}

	// 12 - test axis obb1z cross obb2x
	r1 = obb1.m_halfExt[0] * abs_rotation[1] + obb1.m_halfExt[1] * abs_rotation[0];
	r2 = obb2.m_halfExt[1] * abs_rotation[8] + obb2.m_halfExt[2] * abs_rotation[5];
	dist = abs(t[1] * rotation[0] - t[0] * rotation[1]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(2, 0);	// z from 1, x from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(2, 0);
		}
	}

	// 13 - test axis obb1z cross obb2y
	r1 = obb1.m_halfExt[0] * abs_rotation[4] + obb1.m_halfExt[1] * abs_rotation[3];
	r2 = obb2.m_halfExt[0] * abs_rotation[8] + obb2.m_halfExt[2] * abs_rotation[2];
	dist = abs(t[1] * rotation[3] - t[0] * rotation[4]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(2, 1);	// z from 1, y from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(2, 1);
		}
	}

	// 14 - test axis obb1z cross obb2z
	r1 = obb1.m_halfExt[0] * abs_rotation[7] + obb1.m_halfExt[1] * abs_rotation[6];
	r2 = obb2.m_halfExt[0] * abs_rotation[5] + obb2.m_halfExt[1] * abs_rotation[2];
	dist = abs(t[1] * rotation[6] - t[0] * rotation[7]);
	if (dist > (r1 + r2))
		return 0;
	else
	{
		float overlap = (r1 + r2) - dist;
		//if (overlap > deepest)
		//{
		//	deepest = overlap;
		//	axis_pair = IntVector2(2, 2);	// z from 1, z from 2
		//}
		if (overlap < shallowest)
		{
			shallowest = overlap;
			axis_pair = IntVector2(2, 2);
		}
	}

	// 15 - restore axis and hence find normal
	Vector3 usedNormal;
	Vector3 a1 = ISA;
	Vector3 a2 = ISA;
	int basis1 = axis_pair.x;
	int basis2 = axis_pair.y;
	switch (basis1)
	{
	case -1: break;
	case 0: a1 = obb1.m_right; break;
	case 1: a1 = obb1.m_up; break;
	case 2: a1 = obb1.m_forward; break;
	default: break;
	}
	switch (basis2)
	{
	case -1: break;
	case 0: a2 = obb2.m_right; break;
	case 1: a2 = obb2.m_up; break;
	case 2: a2 = obb2.m_forward; break;
	default: break;
	}
	if(a1 == ISA)
		// valid subject axis is a2
		usedNormal = a2;
	else if (a2 == ISA)
		usedNormal = a1;
	else
		usedNormal = a1.Cross(a2);
	if (DotProduct(usedNormal, obb2.m_center - obb1.m_center) > 0.f)	// should not equal 0
		usedNormal *= -1.f;

	// 16 - generate contact
	usedNormal.NormalizeAndGetLength();
	Vector3 contactPoint = obb1.m_center;
	//float penetration = deepest;
	float penetration = shallowest;
	Contact3 theContact = Contact3(obb1.GetEntity(), obb2.GetEntity(),
		usedNormal, contactPoint, penetration);

	data->m_contacts.push_back(theContact);
	return 1;
}

uint CollisionDetector::OBB3VsOBB3Deep(const OBB3& obb1, const OBB3& obb2, CollisionData3* data)
{
	if (data->m_contacts.size() >= data->m_maxContacts)
		// no contacts amount left, return directly
		return 0;

	BoxEntity3* e1 = static_cast<BoxEntity3*>(obb1.GetEntity());
	BoxEntity3* e2 = static_cast<BoxEntity3*>(obb2.GetEntity());

	// 1 - get all candidates of obb1 verts intersecting with obb2
	std::vector<Contact3> candidates1;
	for (int i = 0; i < 8; ++i)
	{
		eContactFeature feature = e1->m_features[i];
		Vector3 vert = e1->GetFeaturedPoint(feature);

		Contact3 contact;
		contact.m_e1 = e1; contact.m_e2 = e2;
		contact.m_f1 = feature;	contact.m_f2 = UNKNOWN;
		uint intersected = OBB3VsPoint(obb2, vert, contact, false);
		if (intersected == 1) 
			candidates1.push_back(contact);
	}

	// 2 - pick the deepest as final candidate of obb1
	Contact3 final1;
	final1.m_penetration = -INFINITY;
	for (const Contact3& c : candidates1)
	{
		if (c.m_penetration > final1.m_penetration)
			final1 = c;
	}

	// 3 - get all candidates of obb2 verts intersecting with obb1
	std::vector<Contact3> candidates2;
	for (int i = 0; i < 8; ++i)
	{
		eContactFeature feature = e2->m_features[i];
		Vector3 vert = e2->GetFeaturedPoint(feature);

		Contact3 contact;
		contact.m_e1 = e1; contact.m_e2 = e2;
		contact.m_f1 = UNKNOWN; contact.m_f2 = feature; 
		uint intersected = OBB3VsPoint(obb1, vert, contact, true);
		if (intersected == 1) 
			candidates1.push_back(contact);
	}

	// 4 - pick the deepest as final candidate of obb2
	Contact3 final2;
	final2.m_penetration = -INFINITY;
	for (const Contact3& c : candidates2)
	{
		if (c.m_penetration > final2.m_penetration)
			final2 = c;
	}

	// 5 - pick the overall deepest, set as point-face candidate
	Contact3 pointFace;
	pointFace.m_penetration = -INFINITY;
	if (final1.m_penetration > final2.m_penetration)
		pointFace = final1;
	else if (final1.m_penetration < final2.m_penetration)
		pointFace = final2;
	else
	{
		if (final1.m_penetration != -INFINITY)
			// default to final1 if penetrations are the same
			pointFace = final1;
		// otherwise there is no point-face contact, leaving the candidate has a -INFINITY penetration
	}
	pointFace.m_type = POINT_FACE;

	std::vector<Contact3> edgeCandidates;
	// edge to edge contacts
	for (int i = 14; i < 26; ++i)
	{
		// 6 - get each edge for 1
		eContactFeature feature1 = e1->m_features[i];
		LineSegment3 seg1 = e1->GetFeaturedEdge(feature1);

		// 7 - compute its shallowest penetration (not separation) with each edge of 2
		Contact3 candidate;
		candidate.m_penetration = INFINITY;
		candidate.m_e1 = e1;
		candidate.m_e2 = e2;
		candidate.m_f1 = feature1;
		candidate.m_type = EDGE_EDGE;
		for (int j = 14; j < 26; ++j)
		{
			eContactFeature feature2 = e2->m_features[j];
			LineSegment3 seg2 = e2->GetFeaturedEdge(feature2);

			float t1, t2;
			Vector3 close1, close2;
			float distSquared = LineSegment3::ClosestPointsSegments(seg1, seg2, t1, t2, close1, close2);

			// see if this dist is pen dist or sep dist
			float distSame = (close2 - obb2.m_center).GetLength();
			float distDiff = (close1 - obb2.m_center).GetLength();
			if (distDiff < distSame)		// this is a pen
			{
				float dist = sqrtf(distSquared);
				if (dist < candidate.m_penetration)
				{
					candidate.m_penetration = dist;
					candidate.m_f2 = feature2;
					candidate.m_point = close1;
					candidate.m_normal = close2 - close1;
					candidate.m_normal.NormalizeAndGetLength();
				}
			}
		}

		// 8 - sanity check for the candidate, is it valid?
		if (candidate.m_penetration != INFINITY)
			edgeCandidates.push_back(candidate);
	}

	// 9 - get the contact with deepest pen
	Contact3 edgeEdge;
	edgeEdge.m_penetration = -INFINITY;
	for (const Contact3& c : edgeCandidates)
	{
		if (c.m_penetration > edgeEdge.m_penetration)
			edgeEdge = c;
	}

	// 10 - get the deeper of point-face winner and edge-edge winner
	Contact3 winner;
	winner.m_penetration = -INFINITY;		// -INFINITY means the contact is not valid
	if (pointFace.m_penetration > edgeEdge.m_penetration)
		winner = pointFace;
	else if (edgeEdge.m_penetration > pointFace.m_penetration)
		winner = edgeEdge;
	else
	{
		if (pointFace.m_penetration != -INFINITY)
			winner = pointFace;			// default to point face contact if pen is the same
		// otherwise winner contact remains to be invalid
		else
			// meaning that there is no valid contact
			return 0;
	}

	// 11 - if the winner contact already exists, update it
	bool existed = data->HasAndUpdateContact(winner);

	// 12 - if not, push it as a new contact
	if (!existed)
		data->m_contacts.push_back(winner);

	return 1;		
	// in this case, 1 does not necessarily mean that we have 1 more collision,
	// but also could mean that we "updated" 1 collision
}

uint CollisionDetector::OBB3VsPoint(const OBB3& obb, const Vector3& p, Contact3& contact, bool reverse)
{
	BoxEntity3* ent = static_cast<BoxEntity3*>(obb.GetEntity());

	Vector3 local = Transform::WorldToLocalOrthogonal(p, ent->GetEntityTransform());

	Vector3 normal;
	float shallow = obb.GetHalfExt().x - abs(local.x);
	if (shallow < 0.f) return 0;
	normal = obb.m_right.GetNormalized() * ((local.x < 0.f) ? -1.f : 1.f);		// in world coord
	if (reverse)
		normal *= -1.f;

	float depth = obb.GetHalfExt().y - abs(local.y);
	if (depth < 0.f) return 0;
	else if (depth < shallow)
	{
		shallow = depth;
		normal = obb.m_up.GetNormalized() * ((local.y < 0.f) ? -1.f : 1.f);	// in world
		if (reverse)
			normal *= -1.f;
	}

	depth = obb.GetHalfExt().z - abs(local.z);
	if (depth < 0.f) return 0;
	else if  (depth < shallow)
	{
		shallow = depth;
		normal = obb.m_forward.GetNormalized() * ((local.z < 0.f) ? -1.f : 1.f);	// in world
		if (reverse)
			normal *= -1.f;
	}

	// assumption: contact entity already set at this point
	contact.m_normal = normal;
	contact.m_penetration = shallow;
	contact.m_point = p;
	
	return 1;
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

/*
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
*/