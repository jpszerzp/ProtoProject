#include "Engine/Physics/3D/ForceGenerator.hpp"

void GravityGenerator::UpdateForce(CollisionRigidBody*, float)
{
	/*
	// if entity is const (infinite mass), force does not apply
	if (entity3D->IsConst())
		return;

	entity3D->AddForce(m_gravity * entity3D->GetMass3());
	*/
}

void DragGenerator::UpdateForce(CollisionRigidBody*, float)
{
	/*
	Vector3 vel = entity3D->GetLinearVelocity();

	float velAmount = vel.GetLength();
	float coef = m_dragCoef1 * velAmount + m_dragCoef2 * velAmount * velAmount;

	Vector3 force = vel.GetNormalized();
	force *= -coef;

	entity3D->AddForce(force);
	*/
}

void SpringGenerator::UpdateForce(CollisionRigidBody* r, float)
{
	Vector3 extension = r->GetCenter() - m_other->GetCenter();
	
	float delta = extension.GetLength() - m_restLength;

	Vector3 force = extension.GetNormalized() * (-delta * m_const);

	r->AddForce(force);
}

void AnchorSpringGenerator::UpdateForce(CollisionRigidBody*, float)
{
	/*
	Vector3 extension = entity3D->GetEntityCenter() - m_anchor;

	float delta = extension.GetLength() - m_restLength;

	Vector3 force = extension.GetNormalized() * (-delta * m_const);

	entity3D->AddForce(force);
	*/
}
