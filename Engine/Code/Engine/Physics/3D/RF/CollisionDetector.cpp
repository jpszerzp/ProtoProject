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
	//if (DotProduct(b1.GetBasisAndPosition(best), disp) < 0.f)
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
	collision->m_pos = b2.GetRigidBody()->GetTransformMat4() * vert;	// rigid body transform used as it is not affected by scale
	//collision->m_pos = b2.GetTransformMat4() * vert;	// rigid body transform used as it is not affected by scale
	collision->SetBodies(b1.GetRigidBody(), b2.GetRigidBody());
	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);
}

void FillPointFaceBoxConvex(const CollisionBox& b, const CollisionConvexObject& cobj,
	const Vector3& disp, CollisionKeep* c_data, unsigned best, float pen,
	Vector3 b_min, Vector3 b_max, Vector3 c_min, Vector3 c_max)
{
	Collision* collision = c_data->m_collision;

	//const Vector3& n = b.GetBasisAndPosition(best).GetNormalized();
	const Vector3& n = b.GetBasisAndPosition(best);

	collision->m_normal = n;
	collision->m_penetration = pen;
	collision->m_pos = c_min;
	collision->SetBodies(b.GetRigidBody(), cobj.GetRigidBody());
	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);
}

void FillPointFaceConvexBox(const CollisionConvexObject& cobj, const CollisionBox& b, 
	const Vector3& disp, CollisionKeep* c_data, unsigned best, float pen,
	Vector3 b_min, Vector3 b_max, Vector3 c_min, Vector3 c_max)
{
	Collision* collision = c_data->m_collision;

	// there has been 3 axis candidate already indexed with this same variable
	// polygon normals are normalized
	const Vector3& n = cobj.GetPoly(best - 3).m_normal;

	collision->m_normal = n;
	collision->m_penetration = pen;
	collision->m_pos = b_min;
	collision->SetBodies(cobj.GetRigidBody(), b.GetRigidBody());
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
	if (!SATTestBoxVsBox(b1, b2, b1.GetBasisAndPosition(0), disp, 0, pen, best))
		return 0;
	if (!SATTestBoxVsBox(b1, b2, b1.GetBasisAndPosition(1), disp, 1, pen, best))
		return 0;
	if (!SATTestBoxVsBox(b1, b2, b1.GetBasisAndPosition(2), disp, 2, pen, best))
		return 0;

	// b2 basis
	if (!SATTestBoxVsBox(b1, b2, b2.GetBasisAndPosition(0), disp, 3, pen, best))
		return 0;
	if (!SATTestBoxVsBox(b1, b2, b2.GetBasisAndPosition(1), disp, 4, pen, best))
		return 0;
	if (!SATTestBoxVsBox(b1, b2, b2.GetBasisAndPosition(2), disp, 5, pen, best))
		return 0;

	unsigned best_major_axis = best;

	// cross product axis
	Vector3 cross = b1.GetBasisAndPosition(0).Cross(b2.GetBasisAndPosition(0));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 6, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(0).Cross(b2.GetBasisAndPosition(1));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 7, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(0).Cross(b2.GetBasisAndPosition(2));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 8, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(1).Cross(b2.GetBasisAndPosition(0));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 9, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(1).Cross(b2.GetBasisAndPosition(1));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 10, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(1).Cross(b2.GetBasisAndPosition(2));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 11, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(2).Cross(b2.GetBasisAndPosition(0));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 12, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(2).Cross(b2.GetBasisAndPosition(1));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 13, pen, best))
		return 0;

	cross = b1.GetBasisAndPosition(2).Cross(b2.GetBasisAndPosition(2));
	if (!SATTestBoxVsBox(b1, b2, cross, disp, 14, pen, best))
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
		//ASSERT_OR_DIE(AreFloatsCloseEnough(axis.GetLengthSquared(), 1.f), "basis should be unit vector");

		if (DotProduct(axis, disp) > 0.f)
		//if (DotProduct(axis, disp) < 0.f)
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

		// rigid body transform used as they are not scaled
		pt1 = b1.GetRigidBody()->GetTransformMat4() * pt1;
		pt2 = b2.GetRigidBody()->GetTransformMat4() * pt2;
		//pt1 = b1.GetTransformMat4() * pt1;
		//pt2 = b2.GetTransformMat4() * pt2;

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
	float rad = sphere.GetRadius();

	float dist = DotProduct(plane.GetNormal(), position) - plane.GetOffset();

	if (dist * dist > rad * rad)
		return 0;

	Vector3 normal = plane.GetNormal();
	float penetration = -dist;
	
	if (dist < 0.f)
	{
		normal *= -1.f;
		penetration *= -1.f;
	}

	penetration += rad;

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

// ...for now we do not resolve, hence putting no data into the keep...
uint CollisionSensor::SphereVsPlaneContinuous(const CollisionSphere& sph, const CollisionPlane& pl, 
	const Vector3& v, float& t, Vector3& hit, CollisionKeep*)
{
	// sph to plane distance
	float dist = DotProduct(pl.GetNormal(), sph.GetCenter());
	dist -= pl.GetOffset();

	if (abs(dist) <= sph.GetRadius())
	{
		// sphere is already overlapping plane.
		// set TOI to 0 and impact point to sphere center
		t = 0.f;
		hit = sph.GetCenter();
		return 1;
	}
	else
	{
		float denom = DotProduct(pl.GetNormal(), v);
		if (denom * dist >= 0.f)
			// sphere moving parallel to or away from the plane
			return 0;
		else
		{
			// sphere moving towards the plane
			// use +r if sphere in front of plane, -r otherwise
			float r = dist > 0.f ? sph.GetRadius() : -sph.GetRadius();
			t = (r - dist) / denom;
			hit = sph.GetCenter() + v * t - pl.GetNormal() * r;
			return 1;
		}
	}
}

uint CollisionSensor::BoxVsHalfPlane(const CollisionBox& box, const CollisionPlane& plane, CollisionKeep* c_data)
{
	// since it is half space, do not consider collisions if box is in negative space of plane
	float center_dist = DotProduct(box.GetRigidBody()->GetCenter(), plane.GetNormal());
	if (center_dist <= plane.GetOffset())
		return 0;

	if (c_data->m_collision_left <= 0) 
		return 0;

	static float units[8][3] = 
	{
	{1,1,1},
	{-1,1,1},
	{1,-1,1},
	{-1,-1,1},
	{1,1,-1},
	{-1,1,-1},
	{1,-1,-1},
	{-1,-1,-1}
	};

	Collision* collision = c_data->m_collision;
	uint contactsUsed = 0;
	for (uint i = 0; i < 8; i++) 
	{
		Vector3 vertexPos = Vector3(units[i][0], units[i][1], units[i][2]);
		vertexPos = vertexPos * box.GetHalfSize();
		vertexPos = box.GetRigidBody()->GetTransformMat4() * vertexPos;
		// rigid body transform is used here as it is not affected by scale

		float vertexDistance = DotProduct(vertexPos, plane.GetNormal());

		if (vertexDistance <= plane.GetOffset())
		{
			collision->m_pos = vertexPos;
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
	Vector3 center = sphere.GetBasisAndPosition(3);
	Vector3 relCenter = box.GetRigidBody()->GetTransformMat4().MultiplyInverse(center);
	//Vector3 relCentre = box.GetTransformMat4().MultiplyInverse(centre);

	if (abs(relCenter.x) - sphere.GetRadius() > box.GetHalfSize().x ||
		abs(relCenter.y) - sphere.GetRadius() > box.GetHalfSize().y ||
		abs(relCenter.z) - sphere.GetRadius() > box.GetHalfSize().z)
	{
		return 0;
	}

	Vector3 closestPt(0,0,0);
	float dist;

	dist = relCenter.x;
	if (dist > box.GetHalfSize().x) 
		dist = box.GetHalfSize().x;
	if (dist < -box.GetHalfSize().x)
		dist = -box.GetHalfSize().x;
	closestPt.x = dist;

	dist = relCenter.y;
	if (dist > box.GetHalfSize().y) 
		dist = box.GetHalfSize().y;
	if (dist < -box.GetHalfSize().y) 
		dist = -box.GetHalfSize().y;
	closestPt.y = dist;

	dist = relCenter.z;
	if (dist > box.GetHalfSize().z) 
		dist = box.GetHalfSize().z;
	if (dist < -box.GetHalfSize().z) 
		dist = -box.GetHalfSize().z;
	closestPt.z = dist;

	dist = (closestPt - relCenter).GetLengthSquared();
	if (dist > sphere.GetRadius() * sphere.GetRadius()) 
		return 0;

	//Vector3 closestPtWorld = box.GetTransformMat4() * closestPt;
	Vector3 closestPtWorld = box.GetRigidBody()->GetTransformMat4() * closestPt;

	Collision* collision = c_data->m_collision;
	collision->m_normal = (closestPtWorld - center).GetNormalized();
	collision->m_pos = closestPtWorld;
	collision->m_penetration = sphere.GetRadius() - sqrtf(dist);
	collision->SetBodies(box.GetRigidBody(), sphere.GetRigidBody());

	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);

	c_data->NotifyAddedCollisions(1);

	return 1;
}

uint CollisionSensor::ConvexVsHalfPlane(const CollisionConvexObject& convex, const CollisionPlane& plane, CollisionKeep* c_data)
{
	// if negative half space, ignore
	float center_dist = DotProduct(convex.GetRigidBody()->GetCenter(), plane.GetNormal());
	if (center_dist <= plane.GetOffset())
		return 0;

	if (c_data->m_collision_left <= 0) 
		return 0;

	Collision* collision = c_data->m_collision;
	uint contactAcc = 0;
	for (int i = 0; i < convex.GetVertNum(); ++i)
	{
		// this is unit vert
		//const Vector3& unit_vert_pos = convex.GetUnitVert(i);
		
		// ...need to transform it to world space
		//Vector3 world_vert = convex.GetTransformMat4() * unit_vert_pos;
		Vector3 world_vert = convex.GetWorldVert(i);

		// get its distance to plane 
		float vert_dist = DotProduct(world_vert, plane.GetNormal());

		// if the vert goes beyond the plane, consider it an overlap
		if (vert_dist <= plane.GetOffset())
		{
			collision->m_pos = world_vert;
			collision->m_normal = plane.GetNormal();
			collision->m_penetration = plane.GetOffset() - vert_dist;

			collision->SetBodies(convex.GetRigidBody(), nullptr);

			collision->SetFriction(c_data->m_global_friction);
			collision->SetRestitution(c_data->m_global_restitution);

			// to the next contact
			collision++;
			contactAcc++;
			if (contactAcc == (uint)c_data->m_collision_left)
				return contactAcc;
		}
	}

	c_data->NotifyAddedCollisions(contactAcc);

	return contactAcc;
}

uint CollisionSensor::ConvexVsBox(const CollisionConvexObject& convex, const CollisionBox& box, CollisionKeep* c_data)
{
	if (c_data->m_collision_left <= 0)
		return 0;

	Vector3 disp = convex.GetBasisAndPosition(3) - box.GetBasisAndPosition(3);

	float pen = FLT_MAX;
	unsigned best = 0xffffff;
	int idx = 0;

	Vector3 box_min, convex_min = Vector3(FLT_MAX);
	Vector3 box_max, convex_max = Vector3(FLT_MIN);

	// box basis
	bool passed = SATTestBoxVsConvex(box, convex, box.GetBasisAndPosition(0), 
		disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
	if (!passed)
		return 0;
	idx++;

	passed = SATTestBoxVsConvex(box, convex, box.GetBasisAndPosition(1), 
		disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
	if (!passed)
		return 0;
	idx++;

	passed = SATTestBoxVsConvex(box, convex, box.GetBasisAndPosition(2), 
		disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
	if (!passed)
		return 0;
	idx++;

	// convex face normals
	std::vector<Vector3> axes = convex.GetAxes();
	for (int i = 0; i < axes.size(); ++i)
	{
		passed = SATTestBoxVsConvex(box, convex, axes[i], 
			disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
		if (!passed)
			return 0;
		idx++;
	}

	// the best idx not considering cross product axes
	unsigned best_axis_record = best;

	// cross product axes
	// for those cross with 0 axis of box
	for (int i = 0; i < axes.size(); ++i)
	{
		const Vector3& box_axis_0 = box.GetBasisAndPosition(0);
		const Vector3& convex_face_n = axes[i];

		Vector3 cross = box_axis_0.Cross(convex_face_n);

		passed = SATTestBoxVsConvex(box, convex, cross,
			disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
		if (!passed)
			return 0;
		idx++;
	}

	// for those cross with 1 axis of box
	for (int i = 0; i < axes.size(); ++i)
	{
		const Vector3& box_axis_1 = box.GetBasisAndPosition(1);
		const Vector3& convex_face_n = axes[i];

		Vector3 cross = box_axis_1.Cross(convex_face_n);

		passed = SATTestBoxVsConvex(box, convex, cross, 
			disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
		if (!passed)
			return 0;
		idx++;
	}

	// for those cross with 2 axis of box
	for (int i = 0; i < axes.size(); ++i)
	{
		const Vector3& box_axis_2 = box.GetBasisAndPosition(2);
		const Vector3& convex_face_n = axes[i];

		Vector3 cross = box_axis_2.Cross(convex_face_n);

		passed = SATTestBoxVsConvex(box, convex, cross,
			disp, idx, pen, best, box_min, box_max, convex_min, convex_max);
		if (!passed)
			return 0;
		idx++;
	}

	ASSERT_OR_DIE(best != 0xffffff, "should have penetration");

	int box_idx_milestone = 2;
	int convex_idx_milestone = box_idx_milestone + convex.GetPolyNum();
	// axis is box's basis
	if (best <= box_idx_milestone)
	{
		FillPointFaceBoxConvex(box, convex, disp, c_data, 
			best, pen, box_min, box_max, convex_min, convex_max);
		c_data->NotifyAddedCollisions(1);
		return 1;
	}
	// axis is convex normals
	else if (best <= convex_idx_milestone)
	{
		FillPointFaceConvexBox(convex, box, -disp, c_data, 
			best, pen, box_min, box_max, convex_min, convex_max);
		c_data->NotifyAddedCollisions(1);
		return 1;
	}
	// axis is cross product
	else
	{
		int ee_idx = best - (convex_idx_milestone + 1);
		unsigned idx1 = ee_idx / convex.GetPolyNum();	 // box edge
		unsigned idx2 = ee_idx % convex.GetPolyNum();	 // convex edge
		const Vector3& axis1 = box.GetBasisAndPosition(idx1);
		const Vector3& axis2 = convex.GetPoly(idx2).m_normal;
		Vector3 sep_axis = axis1.Cross(axis2);
		sep_axis.Normalize();

		// disp is toward convex, and we set the first collision body as box and the other convex (box to convex)
		if (DotProduct(sep_axis, disp) < 0.f)
			sep_axis *= -1.f;

		Collision* collision = c_data->m_collision;

		collision->m_penetration = pen;
		collision->m_normal = sep_axis;
		collision->m_pos = convex_min;
		collision->SetBodies(box.GetRigidBody(), convex.GetRigidBody());
		collision->SetFriction(c_data->m_global_friction);
		collision->SetRestitution(c_data->m_global_restitution);

		c_data->NotifyAddedCollisions(1);
		return 1;
	}

	return 0;
}

uint CollisionSensor::ConvexVsSphere(const CollisionConvexObject& convex, const CollisionSphere& sph, CollisionKeep* c_data)
{
	if (c_data->m_collision_left <= 0) 
		return 0;

	Collision* collision = c_data->m_collision;

	float best = FLT_MAX;
	Vector3 best_vert = Vector3::INVALID;
	Vector3 best_norm = Vector3::INVALID;

	const std::vector<ConvexPolygon>& poly_refs = convex.GetPolyRefs();
	for (int i = 0; i < poly_refs.size(); ++i)
	{
		const ConvexPolygon& poly = poly_refs[i];
		const int& idx0 = poly.m_vert_idx[0];
		const int& idx1 = poly.m_vert_idx[1];
		const int& idx2 = poly.m_vert_idx[2];
		const Vector3& world_vert_0 = convex.GetWorldVert(idx0);
		const Vector3& world_vert_1 = convex.GetWorldVert(idx1);
		const Vector3& world_vert_2 = convex.GetWorldVert(idx2);
		Vector3 e0to2 = world_vert_2 - world_vert_0;
		Vector3 e0to1 = world_vert_1 - world_vert_0;
		Vector3 n = e0to2.Cross(e0to1);			// we are in left-handed system
		//n *= -1.f;
		n.Normalize();
		//const Vector3& n = poly.m_normal;

		float ext = DotProduct(world_vert_0, n);

		// get sphere projection on to the axis
		const Vector3& sph_center = sph.GetCenter();
		const float& sph_rad = sph.GetRadius();
		float ext_sph_center = DotProduct(sph_center, n);

		if (ext_sph_center - sph_rad > ext)
			return 0;

		// this axis pass, update best case if the overlap is shallower
		float overlap = (ext - (ext_sph_center - sph_rad));
		if (overlap < best)
		{
			best = overlap;
			best_vert = sph_center - (n * sph_rad);
			best_norm = n;
		}
	}

	collision->m_pos = best_vert;
	collision->m_normal = best_norm;
	collision->m_penetration = best;

	collision->SetBodies(convex.GetRigidBody(), sph.GetRigidBody());

	collision->SetFriction(c_data->m_global_friction);
	collision->SetRestitution(c_data->m_global_restitution);

	// limit sph vs convex case to 1 contact at a time
	c_data->NotifyAddedCollisions(1);

	return 1;
}

uint CollisionSensor::LineVsAABB(const CollisionLine& line, const CollisionBox& aabb, CollisionKeep* c_data)
{
	ASSERT_OR_DIE(false, "Funtion unimplemented.");
	return 1;
}