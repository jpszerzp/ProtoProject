#include "Engine/Physics/3D/RF/CollisionDetector.hpp"

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

