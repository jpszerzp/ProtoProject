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

public:
	virtual void AttachToRigidBody(CollisionRigidBody* rb);

	void SetRigidBody(CollisionRigidBody* rb) { m_rigid_body = rb; }
	void SetPrimitiveTransformMat4(const Matrix44& transform) { m_transform_mat = transform; }
	void SetMesh(Mesh* mesh) { m_mesh = mesh; }
	void SetShader(Shader* shader) { m_shader = shader; }
	void SetTexture(Texture* texture) { m_texture = texture; }
	void SetTint(const Vector4& tint) { m_tint = tint; }
	void SetRigidBodyPosition(const Vector3& pos);

	void Update(float deltaTime);

	void Render(Renderer* renderer);

	CollisionRigidBody* GetRigidBody() const { return m_rigid_body; }
	Vector3 GetBasisAndPosition(uint index) const;
	Matrix44 GetTransformMat4() const { return m_transform_mat; }
	Mesh* GetMesh() const { return m_mesh; }
	Shader* GetShader() const { return m_shader; }
	Texture* GetTexture() const { return m_texture; }
	Vector4 GetTint() const { return m_tint; }
};

class CollisionSphere : public CollisionPrimitive
{
	float m_radius;

public:
	CollisionSphere(const float& radius);

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	float GetRadius() const { return m_radius; }
};

class CollisionBox : public CollisionPrimitive
{
	Vector3 m_half_size;

public:
	CollisionBox(const Vector3& half);

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	Vector3 GetHalfSize() const { return m_half_size; }
};

class CollisionPlane : public CollisionPrimitive
{
	Vector3 m_normal;

	float m_offset;

	Vector2 m_bound;

public:
	CollisionPlane(const Vector2& bound, const Vector3& normal, const float& offset);

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	Vector3 GetNormal() const { return m_normal; }
	float GetOffset() const { return m_offset; }
};