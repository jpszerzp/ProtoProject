#pragma once

#include "Engine/Core/Transform.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/RF/CollisionEntity.hpp"

class CollisionPrimitive
{
	Matrix44 m_transform_mat;

	CollisionRigidBody* m_rigid_body;

	void CalculateTransformMat4();

	void Render(Renderer* renderer);

	virtual void AttachToRigidBody(CollisionRigidBody* rb);
};

class CollisionSphere : public CollisionPrimitive
{
	float m_radius;

public:
	CollisionSphere(const float& radius);
	//CollisionSphere(const Vector3& center, const Vector3& euler, const Vector3& scale);

	void AttachToRigidBody(CollisionRigidBody* rb) override;
};