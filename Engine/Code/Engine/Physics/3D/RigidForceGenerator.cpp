#include "Engine/Physics/3D/RigidForceGenerator.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

/*
 * Force generates torque as a product in current model.
 * May use torque generator to generate torque exclusively.
 */

void GravityRigidForceGenerator::UpdateForce(Rigidbody3* body, float deltaTime)
{
	if (body->IsConst())
		return;

	// safe check
	if (body->HasInfiniteMass())
		return;

	// force is always applying to COM
	Vector3 force = m_gravity * body->GetMass3();
	body->AddForce(force);	
	
	Vector3 center = body->GetEntityCenter();
	DebugRenderLine(.01f, center, center + force, 
		5.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
}	


void AnchorSpringRigidForceGenerator::UpdateForce(Rigidbody3* body, float deltaTime)
{
	Vector3 attach_world = body->GetPointInWorld(m_attachLocal);
	Vector3 extension = m_anchorWorld - attach_world;
	float delta = extension.GetLength() - m_restLength;
	Vector3 force = extension.GetNormalized() * (delta * m_const);
	body->AddForcePointWorldCoord(force, attach_world);

	DebugRenderLine(.01f, attach_world, attach_world + force,
		5.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
}
