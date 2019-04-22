#pragma once

#include "Engine/Core/Transform.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/RF/CollisionEntity.hpp"

class CollisionPrimitive
{
	Matrix44 m_transform_mat;

	CollisionRigidBody* m_rigid_body = nullptr;

	// for render pipeline
	Mesh* m_mesh = nullptr;
	Shader* m_shader = nullptr;
	Texture* m_texture = nullptr;
	Vector4 m_tint;

	virtual void AttachToRigidBody(CollisionRigidBody* rb);

public:
	void SetRigidBody(CollisionRigidBody* rb) { m_rigid_body = rb; }
	void SetPrimitiveTransformMat4(const Matrix44& transform) { m_transform_mat = transform; }
	void SetMesh(Mesh* mesh) { m_mesh = mesh; }
	void SetShader(Shader* shader) { m_shader = shader; }
	void SetTexture(Texture* texture) { m_texture = texture; }
	void SetTint(const Vector4& tint) { m_tint = tint; }

	void Update(float deltaTime);

	void Render(Renderer* renderer);
};

class CollisionSphere : public CollisionPrimitive
{
	float m_radius;

public:
	CollisionSphere(const float& radius);

	void AttachToRigidBody(CollisionRigidBody* rb) override;
};