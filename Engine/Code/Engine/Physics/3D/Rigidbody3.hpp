#pragma once

#include "Engine/Physics/3D/Entity3.hpp"

#define ACC_LIMIT 10000.f

class Rigidbody3 : public Entity3
{
public:
	// laws of motion introduced
	Quaternion m_orientation;
	Vector3 m_angularVelocity;
	Vector3 m_torqueAcc;
	float m_angularDamp;
	Vector3 m_lastFrameLinearAcc;

	Matrix44 m_cachedTransform;
	Matrix33 m_inverseInertiaTensorWorld;	// in world space

	// sleep system
	float m_motion;
	bool m_canSleep;
	float m_sleepThreshold = .3f;
	bool m_awake;

public:
	Rigidbody3();
	virtual ~Rigidbody3();

	void InitializeRigid();

	void SetQuaternionIdentity();
	void SetQuaternion(const Quaternion& orientation);
	void SetQuaternion(const float r, const float x, const float y, const float z);
	void SetAwake(bool awake);
	void SetCanSleep(bool value);
	void SetAngularVelocity(const Vector3& angular) { m_angularVelocity = angular; }

	// just before integration
	void CacheData();
	void CacheTransform(Matrix44& transform, 
		const Vector3& position, const Quaternion& orientation);
	void CacheInverseInertiaTensorWorld(
		Matrix33& inv_inertia_tensor_world,
		const Matrix33& inv_inertia_tensor_body,
		const Matrix44& rot_mat);

	// only to torqueacc
	void AddForce(Vector3 force) override;
	void AddTorque(Vector3 torque);
	// to force and torque acc
	void AddForcePointObjectCoord(const Vector3& force, const Vector3& point_local);		
	void AddForcePointWorldCoord(const Vector3& force, const Vector3 point_world);		
	Vector3 GetPointInWorld(const Vector3& pt_local);
	void ClearAccs();
	void IncrementAngularVelocity(Vector3 angular) { m_angularVelocity += angular; }

	virtual void Render(Renderer* renderer) override;
	virtual void Integrate(float deltaTime) override;
	virtual void Simulate(float deltaTime, Vector3& simulate_pos, Quaternion& simulate_orient);
	virtual void UpdateDynamicsCore(float deltaTime) override;
	virtual void UpdateSleepSystem(float deltaTime) override;

	Vector3 GetAngularVelocity() const { return m_angularVelocity; }
	Quaternion GetQuaternion() const { return m_orientation; }
	bool CanSleep() const { return m_canSleep; }
	bool IsAwake() const { return m_awake; }
};