#include "Engine/Physics/3D/ForceGenerator.hpp"

void GravityGenerator::UpdateForce(CollisionRigidBody*, float)
{
	// change from ent to rb...
}

void DragGenerator::UpdateForce(CollisionRigidBody*, float)
{
	// change from ent to rb...
}

void SpringGenerator::UpdateForce(CollisionRigidBody* r, float)
{
	Vector3 extension = r->GetCenter() - m_other->GetCenter();

	float delta = extension.GetLength() - m_restLength;

	Vector3 force = extension.GetNormalized() * (-delta * m_const);

	r->AddForce(force);
}

void AnchorSpringGenerator::UpdateForce(CollisionRigidBody* r, float)
{
	Vector3 extension = r->GetCenter() - m_anchor;

	float delta = extension.GetLength() - m_restLength;

	Vector3 force = extension.GetNormalized() * (-delta * m_const);

	r->AddForce(force);
}