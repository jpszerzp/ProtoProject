#pragma once

#include "Engine/Physics/3D/RF/CollisionEntity.hpp"

class Collision
{
	CollisionRigidBody* m_collision_bodies[2];

	float m_friction = 0.f;

	float m_restitution = 0.f;

	Vector3 m_contact_pt_world;

	Vector3 m_contact_normal_world;

	float m_penetration;

	Matrix33 m_to_world;

	Vector3 m_closing_vel;

	float m_desired_vel;

	Vector3 m_relative_contact_pt[2];
};