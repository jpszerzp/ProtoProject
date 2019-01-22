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

	// derived data
	float m_motion;
	bool m_awake;
	bool m_sleepable;
	Matrix44 m_transform_mat;	

	float m_lin_damp = 1.f;
	float m_ang_damp = 1.f;

public:
	virtual void Integrate(float deltaTime);
	virtual void ClearAcc();
	virtual ~CollisionEntity() {}

	void SetMass(const float& mass) { m_mass = mass; }
	void SetInvMass(const float& inv_mass) { m_inv_mass = inv_mass; }
	void SetCenter(const Vector3& center) { m_center = center; }
	void SetLinearVelocity(const Vector3& lin_vel) { m_lin_vel = lin_vel; }
	void SetSleepable(bool sleepable);
	void SetBaseLinearAcceleration(const Vector3& acc) { m_lin_acc = acc; }

	virtual void CacheData(){}

	virtual void SetAwake(bool){}
	virtual void SetAngularVelocity(const Vector3&) { ASSERT_OR_DIE(false, "entity does not have angular velocity"); }
	virtual void SetTensor(const Matrix33&){ ASSERT_OR_DIE(false, "entity does not have tensor"); }
	virtual void SetInvTensor(const Matrix33&){ ASSERT_OR_DIE(false, "entity does not have inverse tensor"); }
	virtual void SetInvTensorWorld(const Matrix33&){ ASSERT_OR_DIE(false, "entity does not have inverse tensor world"); }

	Matrix44 GetTransformMat4() const { return m_transform_mat; }
	Vector3 GetCenter() const { return m_center; }
	float GetMass() const { return m_mass; }
	float GetInvMass() const { return m_inv_mass; }
	Vector3 GetLinearVelocity() const { return m_lin_vel; }
	Vector3 GetLastFrameLinearAcc() const { return m_last_lin_acc; }
	bool IsAwake() const { return m_awake; }
	bool IsSleepable() const { return m_sleepable; }
	virtual Vector3 GetAngularVelocity() const { ASSERT_OR_DIE(false, "entity does not have angular velocity"); }

	void AddLinearVelocity(const Vector3& v);
	void AddForce(const Vector3& f);
};

class CollisionRigidBody : public CollisionEntity
{
protected:
	Quaternion m_orientation;

	Vector3 m_ang_vel = Vector3::ZERO;

	Vector3 m_net_torque = Vector3::ZERO;

	Matrix33 m_tensor;
	Matrix33 m_inv_tensor;

	// derived data
	Matrix33 m_inv_tensor_world;	

public:
	void Integrate(float deltaTime) override;
	void ClearAcc() override;
	CollisionRigidBody(const float& mass, const Vector3& center, const Vector3& euler);
	~CollisionRigidBody(){}

	void CacheData() override;
	void CacheTransformMat(Matrix44& transform, const Vector3& position, const Quaternion& orientation);
	void CacheIITWorld(Matrix33& iitw, const Matrix33& iit, const Matrix44& transfrom_mat);

	void SetTensor(const Matrix33& tensor) override { m_tensor = tensor; }
	void SetInvTensor(const Matrix33& inv_tensor) override { m_inv_tensor = inv_tensor; }
	void SetInvTensorWorld(const Matrix33& inv_tensor_world) override { m_inv_tensor_world = inv_tensor_world; }
	void SetAngularVelocity(const Vector3& ang_vel) override { m_ang_vel = ang_vel; }
	void SetAwake(bool awake) override;

	void SetOrientation(const Quaternion& q) { m_orientation = q; }

	Quaternion GetOrientation() const { return m_orientation; }
	void GetIITWorld(Matrix33* iitw) const;
	Vector3 GetAngularVelocity() const override { return m_ang_vel; }

	void AddAngularVelocity(const Vector3& v);
};