#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"

void CollisionPrimitive::CalculateTransformMat4()
{

}

void CollisionPrimitive::AttachToRigidBody(CollisionRigidBody* rb)
{

}

//CollisionSphere::CollisionSphere(const Vector3& center, const Vector3& euler, const Vector3& scale)
//{
//
//}

CollisionSphere::CollisionSphere(const float& radius)
	: m_radius(radius)
{

}

void CollisionSphere::AttachToRigidBody(CollisionRigidBody* rb)
{
	Vector3 scale = Vector3(m_radius);

}
