#include "Engine/Physics/3D/RF/CollisionDetector.hpp"
#include "Engine/Math/MathUtils.hpp"

static Vector3 GenerateContactPoint(const Vector3& p1, const Vector3& d1,
	float one_size, const Vector3& p2, const Vector3& d2, float two_size, bool use_one)
{
	Vector3 toSt, cOne, cTwo;
	float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
	float denom, mua, mub;

	smOne = d1.GetLengthSquared();
	smTwo = d2.GetLengthSquared();
	dpOneTwo = DotProduct(d2, d1);

	toSt = p1 - p2;
	dpStaOne = DotProduct(d1, toSt);
	dpStaTwo = DotProduct(d2, toSt);

	denom = smOne * smTwo - dpOneTwo * dpOneTwo;

	// Zero denominator indicates parrallel lines
	if (abs(denom) < 0.0001f)
	{
		return use_one ? p1 : p2;
	}

	mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
	mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

	// If either of the edges has the nearest point out
	// of bounds, then the edges aren't crossed, we have
	// an edge-face contact. Our point is on the edge, which
	// we know from the useOne parameter.
	if (mua > one_size || mua < -one_size ||
		mub > two_size || mub < -two_size)
	{
		return use_one ? p1 : p2;
	}
	else
	{
		cOne = p1 + d1 * mua;
		cTwo = p2 + d2 * mub;

		return cOne * 0.5 + cTwo * 0.5;
	}
}

// for box only
static float ProjectToAxis(const CollisionBox& b, const Vector3& axis)
{
	float x = b.GetHalfSize().x * abs(DotProduct(axis, b.GetBasisAndPosition(0)));
	float y = b.GetHalfSize().y * abs(DotProduct(axis, b.GetBasisAndPosition(1)));
	float z = b.GetHalfSize().z * abs(DotProduct(axis, b.GetBasisAndPosition(2)));

	return x + y + z;
}

static float PenetrationOnAxis(const CollisionBox& b1,
	const CollisionBox& b2, const Vector3& axis, const Vector3& disp)
{
	float half_project_1 = ProjectToAxis(b1, axis);
	float half_project_2 = ProjectToAxis(b2, axis);

	float dist = abs(DotProduct(disp, axis));

	return (half_project_1 + half_project_2 - dist);
}

static bool TryAxis(const CollisionBox& b1, const CollisionBox& b2, Vector3 axis, 
	const Vector3& disp, unsigned index, float& smallest_pen, unsigned& smallest_index)
{
	if (axis.GetLengthSquared() < .0001)
		return true;

	axis.Normalize();

	float penetration = PenetrationOnAxis(b1, b2, axis, disp);

	if (penetration < 0.f)
		return false;

	if (penetration < smallest_pen)
	{
		smallest_pen = penetration;
		smallest_index = index;
	}

	return true;
}

uint CollisionSensor::SphereVsSphere(const CollisionSphere& s1, const CollisionSphere& s2, CollisionKeep* c_data)
{
	// see if still allow collisions
	if (c_data->m_collision_left <= 0)
		return 0;

	Vector3 first_position = s1.GetBasisAndPosition(3);
	Vector3 second_position = s2.GetBasisAndPosition(3);

	Vector3 disp = first_position - second_position;
	float dist = disp.GetLength();

	// if they do not collide
	if (dist <= 0.f || dist >= (s1.GetRadius() + s2.GetRadius()))
		return 0;

	// normal from 2 to 1, pushing 1 away
	Vector3 normal = disp.GetNormalized();

	Collision* this_collision = c_data->m_collision;
	this_collision->SetCollisionNormalWorld(normal);
	this_collision->SetCollisionPtWorld(first_position + (disp * -.5f));		// error?
	this_collision->SetPenetration(s1.GetRadius() + s2.GetRadius() - dist);
	this_collision->SetBodies(s1.GetRigidBody(), s2.GetRigidBody());
	this_collision->SetFriction(c_data->m_global_friction);
	this_collision->SetRestitution(c_data->m_global_restitution);

	c_data->NotifyAddedCollisions(1);

	return 1;
}

void FillPointFaceBoxBox(const CollisionBox& b1, const CollisionBox& b2,
	const Vector3& disp, CollisionKeep* c_data, unsigned best, float pen)
{
	Collision* collision = c_data->m_collision;

	Vector3 normal = b1.GetBasisAndPosition(best);
	if (DotProduct(b1.GetBasisAndPosition(best), disp) > 0.f)
		normal *= -1.f;

	Vector3 vert = b2.GetHalfSize();
	if (DotProduct(b2.GetBasisAndPosition(0), normal) < 0.f)
		vert.x = -vert.x;
	if (DotProduct(b2.GetBasisAndPosition(1), normal) < 0.f)
		vert.y = -vert.y;
	if (DotProduct(b2.GetBasisAndPosition(2), normal) < 0.f)
		vert.z = -vert.z;

	// collision data
	collision->m_normal = normal;
	collision->m_penetration = pen;
	collision->m_pos = b2.GetTransformMat4() * vert;
	collision->SetBodies(b1.GetRigidBody(), b2.GetRigidBody());
	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);
}


uint CollisionSensor::BoxVsBox(const CollisionBox& b1, const CollisionBox& b2, CollisionKeep* c_data)
{
	if (c_data->m_collision_left <= 0)
		return 0;

	Vector3 disp = b2.GetBasisAndPosition(3) - b1.GetBasisAndPosition(3);

	float pen = FLT_MAX;
	unsigned best = 0xffffff;

	// b1 basis 
	if (!TryAxis(b1, b2, b1.GetBasisAndPosition(0), disp, 0, pen, best))
		return 0;
	if (!TryAxis(b1, b2, b1.GetBasisAndPosition(1), disp, 1, pen, best))
		return 0;
	if (!TryAxis(b1, b2, b1.GetBasisAndPosition(2), disp, 2, pen, best))
		return 0;

	// b2 basis
	if (!TryAxis(b1, b2, b2.GetBasisAndPosition(0), disp, 3, pen, best))
		return 0;
	if (!TryAxis(b1, b2, b2.GetBasisAndPosition(1), disp, 4, pen, best))
		return 0;
	if (!TryAxis(b1, b2, b2.GetBasisAndPosition(2), disp, 5, pen, best))
		return 0;

	unsigned best_major_axis = best;

	// cross product axis
	Vector3 cross = b1.GetBasisAndPosition(0).Cross(b2.GetBasisAndPosition(0));
	if (!TryAxis(b1, b2, cross, disp, 6, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(0).Cross(b2.GetBasisAndPosition(1));
	if (!TryAxis(b1, b2, cross, disp, 7, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(0).Cross(b2.GetBasisAndPosition(2));
	if (!TryAxis(b1, b2, cross, disp, 8, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(1).Cross(b2.GetBasisAndPosition(0));
	if (!TryAxis(b1, b2, cross, disp, 9, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(1).Cross(b2.GetBasisAndPosition(1));
	if (!TryAxis(b1, b2, cross, disp, 10, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(1).Cross(b2.GetBasisAndPosition(2));
	if (!TryAxis(b1, b2, cross, disp, 11, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(2).Cross(b2.GetBasisAndPosition(0));
	if (!TryAxis(b1, b2, cross, disp, 12, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(2).Cross(b2.GetBasisAndPosition(1));
	if (!TryAxis(b1, b2, cross, disp, 13, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(2).Cross(b2.GetBasisAndPosition(2));
	if (!TryAxis(b1, b2, cross, disp, 14, pen, best))
		return 0;

	ASSERT_OR_DIE(best != 0xffffff, "should have penetration");

	// axis is one of b1's basis
	if (best < 3)
	{
		FillPointFaceBoxBox(b1, b2, disp, c_data, best, pen);
		c_data->NotifyAddedCollisions(1);
		return 1;
	}
	else if (best < 6)
	{
		FillPointFaceBoxBox(b2, b1, -disp, c_data, best - 3, pen);
		c_data->NotifyAddedCollisions(1);
		return 1;
	}
	else
	{
		best -= 6;
		unsigned index1 = best / 3;
		unsigned index2 = best % 3;
		Vector3 axis1 = b1.GetBasisAndPosition(index1);
		Vector3 axis2 = b2.GetBasisAndPosition(index2);
		Vector3 axis = axis1.Cross(axis2);
		axis.Normalize();

		if (DotProduct(axis, disp) > 0.f)
			axis *= -1.f;

		Vector3 pt1 = b1.GetHalfSize();
		Vector3 pt2 = b2.GetHalfSize();
		for (unsigned i = 0; i < 3; ++i)
		{
			if (i == index1)
				pt1[i] = 0;
			else if (DotProduct(b1.GetBasisAndPosition(i), axis) > 0)
				pt1[i] = -pt1[i];

			if (i == index2)
				pt2[i] = 0;
			else if (DotProduct(b2.GetBasisAndPosition(i), axis) < 0)
				pt2[i] = -pt2[i];
		}

		pt1 = b1.GetTransformMat4() * pt1;
		pt2 = b2.GetTransformMat4() * pt2;

		Vector3 close_vert = GenerateContactPoint(pt1, axis1, b1.GetHalfSize()[index1],
			pt2, axis2, b2.GetHalfSize()[index2], best_major_axis > 2);

		Collision* collision = c_data->m_collision;

		collision->m_penetration = pen;
		collision->m_normal = axis;
		collision->m_pos = close_vert;
		collision->SetBodies(b1.GetRigidBody(), b2.GetRigidBody());
		collision->SetFriction(c_data->m_global_friction);
		collision->SetRestitution(c_data->m_global_restitution);
		
		c_data->NotifyAddedCollisions(1);

		return 1;
	}

	return 0;
}


uint CollisionSensor::SphereVsPlane(const CollisionSphere& sphere, const CollisionPlane& plane, CollisionKeep* c_data)
{
	if (c_data->m_collision_left <= 0)
		return 0;

	Vector3 position = sphere.GetBasisAndPosition(3);

	float dist = DotProduct(plane.GetNormal(), position) - plane.GetOffset();

	if (dist * dist > sphere.GetRadius() * sphere.GetRadius())
		return 0;

	Vector3 normal = plane.GetNormal();
	float penetration = -dist;
	
	if (dist < 0.f)
	{
		normal *= -1.f;
		penetration *= -1.f;
	}

	penetration += sphere.GetRadius();

	Collision* collision = c_data->m_collision;
	collision->m_normal = normal;
	collision->m_penetration = penetration;
	collision->m_pos = position - plane.GetNormal() * dist;
	collision->SetBodies(sphere.GetRigidBody(), nullptr);		// plane is environment, not considered in resolution
	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);

	c_data->NotifyAddedCollisions(1);

	return 1;
}

uint CollisionSensor::BoxVsHalfPlane(const CollisionBox& box, const CollisionPlane& plane, CollisionKeep* c_data)
{
	// since it is half space, do not consider collisions if box is in negative space of plane
	float center_dist = DotProduct(box.GetRigidBody()->GetCenter(), plane.GetNormal());
	if (center_dist <= plane.GetOffset())
		return 0;

	if (c_data->m_collision_left <= 0) 
		return 0;

	static float components[8][3] = {{1,1,1},{-1,1,1},{1,-1,1},{-1,-1,1},
	{1,1,-1},{-1,1,-1},{1,-1,-1},{-1,-1,-1}};

	Collision* collision = c_data->m_collision;
	uint contactsUsed = 0;
	for (uint i = 0; i < 8; i++) 
	{
		Vector3 vertexPos = Vector3(components[i][0], components[i][1], components[i][2]);
		vertexPos = vertexPos * box.GetHalfSize();
		vertexPos = box.GetTransformMat4() * vertexPos;

		float vertexDistance = DotProduct(vertexPos, plane.GetNormal());

		if (vertexDistance <= plane.GetOffset())
		{
			collision->m_pos = plane.GetNormal();
			collision->m_pos *= (vertexDistance-plane.GetOffset());
			collision->m_pos += vertexPos;
			collision->m_normal = plane.GetNormal();
			collision->m_penetration = plane.GetOffset() - vertexDistance;

			collision->SetBodies(box.GetRigidBody(), nullptr);

			collision->SetFriction(c_data->m_global_friction);
			collision->SetRestitution(c_data->m_global_restitution);

			// Move onto the next contact
			collision++;
			contactsUsed++;
			if (contactsUsed == (uint)c_data->m_collision_left) 
				return contactsUsed;
		}
	}

	c_data->NotifyAddedCollisions(contactsUsed);

	return contactsUsed;
}

uint CollisionSensor::BoxVsSphere(const CollisionBox& box, const CollisionSphere& sphere, CollisionKeep* c_data)
{
	// Transform the centre of the sphere into box coordinates
	Vector3 centre = sphere.GetBasisAndPosition(3);
	Vector3 relCentre = box.GetTransformMat4().MultiplyInverse(centre);

	// Early out check to see if we can exclude the contact
	if (abs(relCentre.x) - sphere.GetRadius() > box.GetHalfSize().x ||
		abs(relCentre.y) - sphere.GetRadius() > box.GetHalfSize().y ||
		abs(relCentre.z) - sphere.GetRadius() > box.GetHalfSize().z)
	{
		return 0;
	}

	Vector3 closestPt(0,0,0);
	float dist;

	// Clamp each coordinate to the box.
	dist = relCentre.x;
	if (dist > box.GetHalfSize().x) 
		dist = box.GetHalfSize().x;
	if (dist < -box.GetHalfSize().x)
		dist = -box.GetHalfSize().x;
	closestPt.x = dist;

	dist = relCentre.y;
	if (dist > box.GetHalfSize().y) 
		dist = box.GetHalfSize().y;
	if (dist < -box.GetHalfSize().y) 
		dist = -box.GetHalfSize().y;
	closestPt.y = dist;

	dist = relCentre.z;
	if (dist > box.GetHalfSize().z) 
		dist = box.GetHalfSize().z;
	if (dist < -box.GetHalfSize().z) 
		dist = -box.GetHalfSize().z;
	closestPt.z = dist;

	// Check we're in contact
	dist = (closestPt - relCentre).GetLengthSquared();
	if (dist > sphere.GetRadius() * sphere.GetRadius()) 
		return 0;

	// Compile the contact
	Vector3 closestPtWorld = box.GetTransformMat4() * closestPt;

	Collision* collision = c_data->m_collision;
	collision->m_normal = (closestPtWorld - centre);
	collision->m_normal.Normalize();
	collision->m_pos = closestPtWorld;
	collision->m_penetration = sphere.GetRadius() - sqrtf(dist);
	collision->SetBodies(box.GetRigidBody(), sphere.GetRigidBody());

	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);

	c_data->NotifyAddedCollisions(1);

	return 1;
}
