#include "Engine/Physics/3D/RigidForceGenerator.hpp"

void GravityRigidForceGenerator::UpdateForce(Rigidbody3* body, float deltaTime)
{
	if (body->IsConst())
		return;

	// safe check
	if (body->HasInfiniteMass())
		return;

	// force is always applying to COM
	body->AddForce(m_gravity * body->GetMass());	
}	


void SpringRigidForceGenerator::UpdateForce(Rigidbody3* body, float deltaTime)
{
	Vector3 my_connect_world = body->GetPointInWorld(m_myConnect);
	Vector3 other_connect_world = body->GetPointInWorld(m_otherConnect);
	Vector3 extension = m_myConnect - other_connect_world;
	float delta = extension.GetLength() - m_restLength;
	Vector3 force = extension.GetNormalized() * (-delta * m_const);
	body->AddForcePointWorldCoord(force, my_connect_world);
}
