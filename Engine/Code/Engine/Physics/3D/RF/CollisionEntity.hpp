#pragma once

#include "Engine/Physics/MassData.hpp"
#include "Engine/Core/Quaternion.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class CollisionEntity
{
protected:
	Vector3 m_lin_vel = Vector3::ZERO;

	Vector3 m_center;

	Vector3 m_lin_acc = Vector3::ZERO;
	Vector3 m_last_lin_acc = Vector3::ZERO;

	Vector3 m_net_force = Vector3::ZERO;

	float m_mass;
	float m_inv_mass;

	Matrix44 m_transform_mat;
	//Transform m_transform;

	float m_lin_damp = 1.f;
	float m_ang_damp = 1.f;

public:
	virtual void Integrate(float deltaTime);
	virtual void ClearAcc();
	virtual ~CollisionEntity() {}

	void SetMass(const float& mass) { m_mass = mass; }
	void SetInvMass(const float& inv_mass) { m_inv_mass = inv_mass; }
	void SetCenter(const Vector3& center) { m_center = center; }

	virtual void CacheData(){}

	virtual void SetTensor(const Matrix33&){ ASSERT_OR_DIE(false, "entity does not have tensor"); }
	virtual void SetInvTensor(const Matrix33&){ ASSERT_OR_DIE(false, "entity does not have inverse tensor"); }
	virtual void SetInvTensorWorld(const Matrix33&){ ASSERT_OR_DIE(false, "entity does not have inverse tensor world"); }

	Matrix44 GetTransformMat4() const { return m_transform_mat; }
};

class CollisionRigidBody : public CollisionEntity
{
protected:
	Quaternion m_orientation;

	Vector3 m_ang_vel = Vector3::ZERO;

	Vector3 m_net_torque = Vector3::ZERO;

	Matrix33 m_tensor;
	Matrix33 m_inv_tensor;
	Matrix33 m_inv_tensor_world;

public:
	void Integrate(float deltaTime) override;
	void ClearAcc() override;
	CollisionRigidBody(const Vector3& center, const Quaternion& orientation);
	CollisionRigidBody(const Vector3& center, const Vector3& euler);
	~CollisionRigidBody(){}

	void CacheData() override;
	void CacheTransformMat(Matrix44& transform, const Vector3& position, const Quaternion& orientation);
	void CacheIITWorld(Matrix33& iitw, const Matrix33& iit, const Matrix44& transfrom_mat);

	void SetTensor(const Matrix33& tensor) override { m_tensor = tensor; }
	void SetInvTensor(const Matrix33& inv_tensor) override { m_inv_tensor = inv_tensor; }
	void SetInvTensorWorld(const Matrix33& inv_tensor_world) override { m_inv_tensor_world = inv_tensor_world; }
};