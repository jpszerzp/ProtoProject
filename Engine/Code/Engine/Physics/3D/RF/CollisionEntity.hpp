#pragma once

#include "Engine/Physics/MassData.hpp"
#include "Engine/Core/Quaternion.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

enum eParticleVerlet
{
	VERLET_VEL_P,
	VERLET_BASIC_P,
	VERLET_P_NUM,
};

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

	float m_slow = 1.f;

	// freeze has highest priority on pause
	bool m_frozen = false;

	// verlet specific
	Vector3 m_last_center = Vector3::ZERO;
	Vector3 m_half_step_vel = Vector3::ZERO;

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
	void SetSlow(const float& slow) { m_slow = slow; }
	void SetFrozen(bool val) { m_frozen = val; }

	virtual void CacheData(){}

	virtual void SetAwake(bool){}
	virtual void SetAngularVelocity(const Vector3&) { ASSERT_OR_DIE(false, "general entity does not have angular velocity"); }
	virtual void SetTensor(const Matrix33&){ ASSERT_OR_DIE(false, "general entity does not have tensor"); }
	virtual void SetInvTensor(const Matrix33&){ ASSERT_OR_DIE(false, "general entity does not have inverse tensor"); }
	virtual void SetInvTensorWorld(const Matrix33&){ ASSERT_OR_DIE(false, "general entity does not have inverse tensor world"); }

	Matrix44 GetTransformMat4() const { return m_transform_mat; }
	Vector3 GetCenter() const { return m_center; }
	float GetMass() const { return m_mass; }
	float GetInvMass() const { return m_inv_mass; }
	Vector3 GetLinearVelocity() const { return m_lin_vel; }
	Vector3 GetLastFrameLinearAcc() const { return m_last_lin_acc; }
	bool IsAwake() const { return m_awake; }
	bool IsSleepable() const { return m_sleepable; }
	bool IsFrozen() const { return m_frozen; }
	virtual Vector3 GetAngularVelocity() const { ASSERT_OR_DIE(false, "general entity does not have angular velocity"); }
	virtual float GetRealTimeMotion() const;
	virtual Matrix33 GetTensor() const { ASSERT_OR_DIE(false, "general entity does not have tensor"); }

	void AddLinearVelocity(const Vector3& v);
	void AddForce(const Vector3& f);
};

class CollisionRigidBody : public CollisionEntity
{
protected:
	// if this body is for particle...
	// if yes, rigid body works effectively like an entity...
	bool m_particle = false;
	bool m_verlet = false;
	eParticleVerlet m_verlet_p = VERLET_P_NUM;

	Quaternion m_orientation;

	Vector3 m_ang_vel = Vector3::ZERO;

	Vector3 m_net_torque = Vector3::ZERO;

	Matrix33 m_tensor;
	Matrix33 m_inv_tensor;

	// derived data
	Matrix33 m_inv_tensor_world;	

public:
	void Integrate(float deltaTime) override;
	void IntegrateEulerParticle(float dt);
	void IntegrateVerletParticle(float dt);

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
	void SetParticle(bool particle) { m_particle = particle; }
	void SetVerlet(bool val) { m_verlet = val; }
	void SetVerletScheme(eParticleVerlet val) { m_verlet_p = val; }

	void SetOrientation(const Quaternion& q) { m_orientation = q; }

	Quaternion GetOrientation() const { return m_orientation; }
	void GetIITWorld(Matrix33* iitw) const;
	Vector3 GetAngularVelocity() const override { return m_ang_vel; }
	float GetRealTimeMotion() const override;
	Matrix33 GetTensor() const override { return m_tensor; }
	bool IsVerlet() const { return m_verlet; }
	bool IsParticle() const { return m_particle; }
	eParticleVerlet GetParticleVerletScheme() const { return m_verlet_p; }

	void AddAngularVelocity(const Vector3& v);
};