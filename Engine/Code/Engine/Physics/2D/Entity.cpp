#include "Engine/Physics/2D/Entity.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

Entity::Entity()
{

}

Entity::~Entity()
{

}

void Entity::Update(float)
{

}

void Entity::UpdateInput()
{

}

void Entity::UpdateBoundAABB()
{

}

void Entity::UpdateBoundDisc()
{

}

void Entity::UpdateLinearPosition(float deltaSeconds)
{
	// only for burst entities that has high acceleration in short time
	m_center = m_center + m_velocity * deltaSeconds;
}

void Entity::UpdateLinearAcceleration()
{
	m_acceleration = m_netForce * (m_massData.m_invMass);
}

void Entity::UpdateLinearVelocity(float deltaSeconds)
{
	m_velocity = m_velocity + m_acceleration * deltaSeconds;

	TODO("damp/drag 2D");
}

void Entity::UpdateAngularPosition(float deltaSeconds)
{
	m_orientation = m_orientation + m_signedRotationVel * deltaSeconds;
}

void Entity::UpdateAngularAcceleration()
{
	//no torque supported for now, all changes on angular velocity are moment based

}

void Entity::UpdateAngularVelocity(float deltaSeconds)
{
	m_signedRotationVel = m_signedRotationVel + m_signedRotationAcc * deltaSeconds;
}

void Entity::LimitSpeed(float limit)
{
	Vector2 velocityDir = m_velocity.GetNormalized();
	float spd = m_velocity.GetLength();
	spd = ClampFloat(spd, 0.f, limit);
	m_velocity = velocityDir * spd;
}

void Entity::Integrate(float deltaSeconds)
{
	// linear
	UpdateLinearPosition(deltaSeconds);
	UpdateLinearAcceleration();
	UpdateLinearVelocity(deltaSeconds);

	// angular
	UpdateAngularPosition(deltaSeconds);
	UpdateAngularAcceleration();
	UpdateAngularVelocity(deltaSeconds);
}

void Entity::ClearForce()
{
	m_netForce = Vector2::ZERO;
}

void Entity::Render(Renderer*)
{

}

void Entity::TranslateEntity(Vector2 translation)
{
	m_center += translation;
}

