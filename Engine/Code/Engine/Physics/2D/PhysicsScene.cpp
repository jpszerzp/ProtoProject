#include "Engine/Physics/2D/PhysicsScene.hpp"
#include "Engine/Physics/2D/Manifold.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Util/DataUtils.hpp"

sCollisionPair::sCollisionPair(Entity* e1, Entity* e2)
{
	m_e1 = e1;
	m_e2 = e2;
}

sCollisionPair::~sCollisionPair()
{

}


PhysicsScene::PhysicsScene()
{
	m_graivty = Vector2(0.f, -GRAVITY_CONST * 1.f);

	Window* window = Window::GetInstance();
	float winWidth = window->GetWindowWidth();
	float winHeight = window->GetWindowHeight();
	Vector2 winMin = Vector2::ZERO - Vector2(winWidth / 2.f, winHeight / 2.f);
	Vector2 winMax = Vector2::ZERO + Vector2(winWidth / 2.f, winHeight / 2.f);
	m_tree = new Quadtree(0, AABB2(winMin, winMax));

	m_bpScheme = BP_BOUND;
	m_scheme = PHYSICS_CORRECTIVE;
}

PhysicsScene::~PhysicsScene()
{
	m_axisList.clear();
	m_activeList.clear();

	DeleteVector(m_pairs);
	DeleteVector(m_discEntities);
}

void PhysicsScene::SortAxisList(eSAPAxis axis)
{
	// insertion sort
	int i, j;
	Entity* key = nullptr;
	for (i = 1; i < m_axisList.size(); ++i)
	{
		key = m_axisList[i];
		j = i - 1;

		// Move elements of vector from 0 to i - 1 that have 
		// greater position than key does along specified axis
		// to one position ahead
		bool greaterPos = false;
		switch (axis)
		{
		case AXIS_X:
			greaterPos = DescendBoundAABBMinX(*m_axisList[j], *key);
			break;
		case AXIS_Y:
			greaterPos = DescendBoundAABBMinY(*m_axisList[j], *key);
			break;
		case AXIS_Z:
			break;
		case AXIS_NUM:
			break;
		default:
			break;
		}
		while (j >= 0 && greaterPos)
		{
			m_axisList[j + 1] = m_axisList[j];
			j = j - 1;
		}
		m_axisList[j + 1] = key;
	}

	// sorted list of objects along wanted axis
}

void PhysicsScene::ProcessActiveList(eSAPAxis axis)
{
	// make sure active list and pair list are empty
	m_activeList.clear();
	DeleteVector(m_pairs);

	// begin on left of axis lits
	for (std::vector<Entity*>::size_type idx = 0; idx < m_axisList.size(); ++idx)
	{
		Entity* e = m_axisList[idx];
		m_activeList.push_back(e);

		// the next element is valid
		Entity* next = nullptr;
		if ((idx + 1) < m_axisList.size())
		{
			next = m_axisList[idx + 1];
		}
		else
		{
			// have come to end of axis list, all pairs processed
			return;
		}

		switch (axis)
		{
		case AXIS_X:
			// go thru active list
			for (int activeIdx = (int)(m_activeList.size()- 1U); activeIdx >= 0; --activeIdx)
			{
				Entity* currentEntity = m_activeList[activeIdx];
				float nextLeft = next->m_boundAABB.mins.x;
				float currentRight = currentEntity->m_boundAABB.maxs.x;

				if (nextLeft > currentRight)
				{
					// remove this current entity from active list
					m_activeList[activeIdx] = m_activeList[m_activeList.size() - 1U];
					m_activeList.pop_back();
				}
				else
				{
					// add this collision pair
					sCollisionPair* pair = new sCollisionPair(next, currentEntity);
					m_pairs.push_back(pair);

					next->m_passedBroadphase = true;
					currentEntity->m_passedBroadphase = true;
				}
			}
			break;
		case AXIS_Y:
			break;
		case AXIS_Z:
			break;
		case AXIS_NUM:
			break;
		default:
			break;
		}
	}
}

void PhysicsScene::AddQuadPhysicsEntity(QuadEntity* quadEntity)
{
	m_quadEntities.push_back(quadEntity);
}

void PhysicsScene::AddDiscPhysicsEntity(DiscEntity* discEntity)
{
	m_discEntities.push_back(discEntity);
	m_axisList.push_back(discEntity);
}

void PhysicsScene::AddRectanglePhysicsEntity(RectangleEntity* rectangleEntity)
{
	m_rectangleEntities.push_back(rectangleEntity);
	m_axisList.push_back(rectangleEntity);
}

void PhysicsScene::ApplyGravity(Entity* entity)
{
	// for now only applies gravity
	if (m_gravityOn)
	{
		entity->m_netForce = m_graivty;
	}
	else
	{
		entity->m_netForce = Vector2::ZERO;
	}
}

void PhysicsScene::FinalizeTransformDiscEntity()
{
	for each (DiscEntity* disc in m_discEntities)
	{
		disc->UpdateTransformFinal();
	}
}

void PhysicsScene::FinalizeTransformQuadEntity()
{
	for each (QuadEntity* quad in m_quadEntities)
	{
		quad->UpdateTransformFinal();
	}
}

void PhysicsScene::FinalizeTransformRectangleEntity()
{
	for each (RectangleEntity* rectangle in m_rectangleEntities)
	{
		rectangle->UpdateTransformFinal();
	}
}

void PhysicsScene::UpdateScene(float deltaTime)
{
	UpdateInput();
	
	UpdateDiscPhysicsEntities(deltaTime);
	UpdateRectanglePhysicsEntities(deltaTime);

	switch (m_scheme)
	{
	case PHYSICS_CORRECTIVE:
	{
		switch (m_bpScheme)
		{
		case BP_BOUND:
			ProcessCorrectivePhysics();
			break;
		case BP_SAP:
			ProcessSAP();
			break;
		case BP_QUAD:
			ProcessQuadtree();
			break;
		case NUM_BP:
			break;
		default:
			break;
		}
	}
		break;
	case PHYSICS_PREVENTATIVE:
	{
		ProcessPreventativePhysics(deltaTime);
	}
		break;
	case PHYSICS_CONTINUOUS:
	{
		ProcessContinuousPhysics(deltaTime);
	}
		break;
	case NUM_OF_SCHEME:
		break;
	default:
		break;
	}

	UpdateQuadPhysicsEntities(deltaTime);		// lazy updates on quad for CCD

	FinalizeTransformDiscEntity();
	FinalizeTransformRectangleEntity();
	FinalizeTransformQuadEntity();
}

void PhysicsScene::UpdateQuadPhysicsEntities(float deltaTime)
{
	for each (QuadEntity* quad in m_quadEntities)
	{
		quad->Update(deltaTime);
	}
}

void PhysicsScene::UpdateDiscPhysicsEntities(float deltaTime)
{
	for each (DiscEntity* disc in m_discEntities)
	{
		disc->Update(deltaTime);
	}
}

void PhysicsScene::UpdateRectanglePhysicsEntities(float deltaTime)
{
	for each (RectangleEntity* rectangle in m_rectangleEntities)
	{
		rectangle->Update(deltaTime);
	}
}

void PhysicsScene::UpdateDelete()
{
	for (int discIdx = (int)(m_discEntities.size()- 1U); discIdx >= 0; --discIdx)
	{
		DiscEntity* disc = m_discEntities[discIdx];

		if (disc->m_dead)
		{
			m_discEntities[discIdx] = m_discEntities[m_discEntities.size() - 1U];
			m_discEntities.pop_back();
		}
	}
	for (int recIdx = (int)(m_rectangleEntities.size()- 1U); recIdx >= 0; --recIdx)
	{
		RectangleEntity* rec = m_rectangleEntities[recIdx];

		if (rec->m_dead)
		{
			m_rectangleEntities[recIdx] = m_rectangleEntities[m_rectangleEntities.size() - 1U];
			m_rectangleEntities.pop_back();
		}
	}
	for (int quadIdx = (int)(m_quadEntities.size()- 1U); quadIdx >= 0; --quadIdx)
	{
		QuadEntity* quad = m_quadEntities[quadIdx];

		if (quad->m_dead)
		{
			m_quadEntities[quadIdx] = m_quadEntities[m_quadEntities.size() - 1U];
			m_quadEntities.pop_back();
		}
	}

	for (int idx = (int)(m_axisList.size()- 1U); idx >= 0; --idx)
	{ 
		Entity* ent = m_axisList[idx];

		if (ent->m_dead)
		{
			m_axisList[idx] = m_axisList[m_axisList.size() - 1U];
			m_axisList.pop_back();
		}
	}
}

void PhysicsScene::UpdateInput()
{
	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_F5))
	{
		m_drawBP = !m_drawBP;
	}
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_F11))
	{
		if (m_bpScheme == BP_BOUND)
		{
			m_bpScheme = BP_SAP;
		}
		else if (m_bpScheme == BP_SAP)
		{
			m_bpScheme = BP_QUAD;
		}
		else if (m_bpScheme == BP_QUAD)
		{
			m_bpScheme = BP_BOUND;
		}
	}
}

void PhysicsScene::RenderScene(Renderer* renderer)
{
	RenderDiscPhysicsEntities(renderer);
	RenderRectanglePhysicsEntities(renderer);
	RenderQuadPhysicsEntities(renderer);
}

void PhysicsScene::RenderQuadPhysicsEntities(Renderer* renderer)
{
	for each (QuadEntity* quad in m_quadEntities)
	{
		quad->Render(renderer);
	}
}

void PhysicsScene::RenderDiscPhysicsEntities(Renderer* renderer)
{
	for each (DiscEntity* disc in m_discEntities)
	{
		disc->Render(renderer);
	}
}

void PhysicsScene::RenderRectanglePhysicsEntities(Renderer* renderer)
{
	for each (RectangleEntity* rectangle in m_rectangleEntities)
	{
		rectangle->Render(renderer);
	}
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsConstDisc(DiscEntity& e1, const DiscEntity& e2, Vector2 normal)
{
	// Calculate relative velocity
	Vector2 relativeVelocity = e2.m_velocity - e1.m_velocity;

	// Calculate relative velocity in terms of the normal direction
	float velAlongNormal = DotProduct( relativeVelocity, normal );

	// Do not resolve if velocities are separating
	if(velAlongNormal > 0.f)
	{
		return;
	}

	// Calculate restitution
	float e = 1.f * e1.m_physicsMat.m_elasticity * e2.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velAlongNormal;
	impulseScalar /= (e1.m_massData.m_invMass + e2.m_massData.m_invMass);

	// Apply impulse
	Vector2 impulse = impulseScalar * normal;
	e1.m_velocity -= e1.m_massData.m_invMass * impulse;
	// no need to apply impulse on const disc

	// friction
	float f = 1.f * e1.m_physicsMat.m_frictionCoef * e2.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relativeVelocity - (normal * velAlongNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relativeVelocity, tangent) * f;
	impulseScalarTangent /= (e1.m_massData.m_invMass + e2.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	e1.m_velocity -= e1.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsConstQuad(DiscEntity& disc, const QuadEntity& quad, Vector2 normal)
{
	Vector2 relVel = quad.m_velocity - disc.m_velocity;

	float velNormal = DotProduct(relVel, normal);

	if (velNormal > 0.f)
	{
		return;
	}

	// restitution
	float e = 1.f * disc.m_physicsMat.m_elasticity * quad.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velNormal;
	impulseScalar /= (disc.m_massData.m_invMass + quad.m_massData.m_invMass);

	// Apply impulse
	Vector2 impulse = impulseScalar * normal;
	disc.m_velocity -= disc.m_massData.m_invMass * impulse;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsConstRec(DiscEntity& disc, const RectangleEntity& rec, Vector2 normal)
{
	Vector2 relVel = rec.m_velocity - disc.m_velocity;

	float velNormal = DotProduct(relVel, normal);

	if (velNormal > 0.f)
	{
		return;
	}

	// restitution
	float e = 1.f * disc.m_physicsMat.m_elasticity * rec.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velNormal;
	impulseScalar /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass);

	// Apply impulse
	Vector2 impulse = impulseScalar * normal;
	disc.m_velocity -= disc.m_massData.m_invMass * impulse;

	// friction
	float f = 1.f * rec.m_physicsMat.m_frictionCoef * disc.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relVel - (normal * velNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarTangent /= (rec.m_massData.m_invMass + disc.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	disc.m_velocity -= disc.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsDisc(DiscEntity& e1, DiscEntity& e2, Vector2 normal)
{
	// Calculate relative velocity
	Vector2 relativeVelocity = e2.m_velocity - e1.m_velocity;

	// Calculate relative velocity in terms of the normal direction
	float velAlongNormal = DotProduct( relativeVelocity, normal );

	// Do not resolve if velocities are separating
	if(velAlongNormal > 0)
	{
		return;
	}

	// Calculate restitution
	float e = 1.f * e1.m_physicsMat.m_elasticity * e2.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velAlongNormal;
	impulseScalar /= (e1.m_massData.m_invMass+ e2.m_massData.m_invMass);

	// apply impulse
	Vector2 impulse = impulseScalar * normal;
	e1.m_velocity -= e1.m_massData.m_invMass * impulse;
	e2.m_velocity += e2.m_massData.m_invMass * impulse;

	// friction
	float f = 1.f * e1.m_physicsMat.m_frictionCoef * e2.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relativeVelocity - (normal * velAlongNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relativeVelocity, tangent) * f;
	impulseScalarTangent /= (e1.m_massData.m_invMass + e2.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	e1.m_velocity -= e1.m_massData.m_invMass * impulseTangent;
	e2.m_velocity += e2.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsQuad(DiscEntity& disc, QuadEntity& quad, Vector2 normal)
{
	// Calculate relative velocity
	Vector2 relativeVelocity = quad.m_velocity - disc.m_velocity;

	// Calculate relative velocity in terms of the normal direction
	float velAlongNormal = DotProduct( relativeVelocity, normal );

	// Do not resolve if velocities are separating
	if(velAlongNormal > 0)
	{
		return;
	}

	// Calculate restitution
	float elasticity = 1.f * disc.m_physicsMat.m_elasticity * quad.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + elasticity) * velAlongNormal;
	impulseScalar /= (disc.m_massData.m_invMass + quad.m_massData.m_invMass);

	// apply impulse
	Vector2 impulse = impulseScalar * normal;
	disc.m_velocity -= disc.m_massData.m_invMass * impulse;
	quad.m_velocity += quad.m_massData.m_invMass * impulse;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsRec(DiscEntity& disc, RectangleEntity& rec, Vector2 normal)
{
	// Calculate relative velocity
	Vector2 relativeVelocity = rec.m_velocity - disc.m_velocity;

	// Calculate relative velocity in terms of the normal direction
	float velAlongNormal = DotProduct( relativeVelocity, normal );

	// Do not resolve if velocities are separating
	if(velAlongNormal > 0)
	{
		return;
	}

	// Calculate restitution
	float e = 1.f * disc.m_physicsMat.m_elasticity * rec.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velAlongNormal;
	impulseScalar /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass);

	// apply impulse
	Vector2 impulse = impulseScalar * normal;
	disc.m_velocity -= disc.m_massData.m_invMass * impulse;
	rec.m_velocity += rec.m_massData.m_invMass * impulse;

	// friction
	float f = 1.f * disc.m_physicsMat.m_frictionCoef * rec.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relativeVelocity - (normal * velAlongNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relativeVelocity, tangent) * f;
	impulseScalarTangent /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	disc.m_velocity -= disc.m_massData.m_invMass * impulseTangent;
	rec.m_velocity += rec.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsRecAngular(DiscEntity& disc, RectangleEntity& rec, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (rec.m_massData.m_invMass / (rec.m_massData.m_invMass + disc.m_massData.m_invMass));
	Vector2 weightedEnd = end * (disc.m_massData.m_invMass / (rec.m_massData.m_invMass + disc.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - disc.m_center);
	Vector2 toP2 = -(p - rec.m_center);

	Vector2 v1 = disc.m_velocity + Vector2(-1.f * disc.m_signedRotationVel * toP1.y, disc.m_signedRotationVel * toP1.x);
	Vector2 v2 = rec.m_velocity + Vector2(-1.f * rec.m_signedRotationVel * toP2.y, rec.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * disc.m_physicsMat.m_frictionCoef * rec.m_physicsMat.m_frictionCoef;
	float e = 1.f * disc.m_physicsMat.m_elasticity * rec.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * disc.m_massData.m_inertia
		+ cross2Nz * cross2Nz * rec.m_massData.m_inertia);

	// apply normal impulse
	disc.m_signedRotationVel -= cross1Nz * impulseScalarN * disc.m_massData.m_inertia;
	rec.m_signedRotationVel += cross2Nz * impulseScalarN * rec.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * disc.m_massData.m_inertia
		+ cross2Tz * cross2Tz * rec.m_massData.m_inertia);

	// apply tangent impulse
	disc.m_signedRotationVel -= cross1Tz * impulseScalarT * disc.m_massData.m_inertia;
	rec.m_signedRotationVel += cross2Tz * impulseScalarT * rec.m_massData.m_inertia;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsConstRecAngular(DiscEntity& disc, const RectangleEntity& rec, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (rec.m_massData.m_invMass / (rec.m_massData.m_invMass + disc.m_massData.m_invMass));
	Vector2 weightedEnd = end * (disc.m_massData.m_invMass / (rec.m_massData.m_invMass + disc.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - disc.m_center);
	Vector2 toP2 = -(p - rec.m_center);

	Vector2 v1 = disc.m_velocity + Vector2(-1.f * disc.m_signedRotationVel * toP1.y, disc.m_signedRotationVel * toP1.x);
	Vector2 v2 = rec.m_velocity + Vector2(-1.f * rec.m_signedRotationVel * toP2.y, rec.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * disc.m_physicsMat.m_frictionCoef * rec.m_physicsMat.m_frictionCoef;
	float e = 1.f * disc.m_physicsMat.m_elasticity * rec.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * disc.m_massData.m_inertia
		+ cross2Nz * cross2Nz * rec.m_massData.m_inertia);

	// apply normal impulse
	disc.m_signedRotationVel -= cross1Nz * impulseScalarN * disc.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (disc.m_massData.m_invMass + rec.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * disc.m_massData.m_inertia
		+ cross2Tz * cross2Tz * rec.m_massData.m_inertia);

	// apply tangent impulse
	disc.m_signedRotationVel -= cross1Tz * impulseScalarT * disc.m_massData.m_inertia;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsConstDiscAngular(DiscEntity& disc1, const DiscEntity& disc2, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (disc2.m_massData.m_invMass / (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass));
	Vector2 weightedEnd = end * (disc1.m_massData.m_invMass / (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - disc1.m_center);
	Vector2 toP2 = -(p - disc2.m_center);

	Vector2 v1 = disc1.m_velocity + Vector2(-1.f * disc1.m_signedRotationVel * toP1.y, disc1.m_signedRotationVel * toP1.x);
	Vector2 v2 = disc2.m_velocity + Vector2(-1.f * disc2.m_signedRotationVel * toP2.y, disc2.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * disc1.m_physicsMat.m_frictionCoef * disc2.m_physicsMat.m_frictionCoef;
	float e = 1.f * disc1.m_physicsMat.m_elasticity * disc2.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * disc1.m_massData.m_inertia
		+ cross2Nz * cross2Nz * disc2.m_massData.m_inertia);

	// apply normal impulse
	disc1.m_signedRotationVel -= cross1Nz * impulseScalarN * disc1.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * disc1.m_massData.m_inertia
		+ cross2Tz * cross2Tz * disc2.m_massData.m_inertia);

	// apply tangent impulse
	disc1.m_signedRotationVel -= cross1Tz * impulseScalarT * disc1.m_massData.m_inertia;
}

void PhysicsScene::ResolveCollisionPhysicsDiscVsDiscAngular(DiscEntity& disc1, DiscEntity& disc2, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (disc2.m_massData.m_invMass / (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass));
	Vector2 weightedEnd = end * (disc1.m_massData.m_invMass / (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - disc1.m_center);
	Vector2 toP2 = -(p - disc2.m_center);

	Vector2 v1 = disc1.m_velocity + Vector2(-1.f * disc1.m_signedRotationVel * toP1.y, disc1.m_signedRotationVel * toP1.x);
	Vector2 v2 = disc2.m_velocity + Vector2(-1.f * disc2.m_signedRotationVel * toP2.y, disc2.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * disc1.m_physicsMat.m_frictionCoef * disc2.m_physicsMat.m_frictionCoef;
	float e = 1.f * disc1.m_physicsMat.m_elasticity * disc2.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * disc1.m_massData.m_inertia
		+ cross2Nz * cross2Nz * disc2.m_massData.m_inertia);

	// apply normal impulse
	disc1.m_signedRotationVel -= cross1Nz * impulseScalarN * disc1.m_massData.m_inertia;
	disc2.m_signedRotationVel += cross2Nz * impulseScalarN * disc2.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (disc1.m_massData.m_invMass + disc2.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * disc1.m_massData.m_inertia
		+ cross2Tz * cross2Tz * disc2.m_massData.m_inertia);

	// apply tangent impulse
	disc1.m_signedRotationVel -= cross1Tz * impulseScalarT * disc1.m_massData.m_inertia;
	disc2.m_signedRotationVel += cross2Tz * impulseScalarT * disc2.m_massData.m_inertia;
}

void PhysicsScene::ResolveCollisionPhysicsQuadVsConstQuad(QuadEntity& quad1, const QuadEntity& quad2, Vector2 normal)
{
	Vector2 relVel = quad2.m_velocity - quad1.m_velocity;

	float velNormal = DotProduct(relVel, normal);

	if (velNormal > 0.f)
	{
		return;
	}

	// restitution
	float e = 1.f * quad1.m_physicsMat.m_elasticity * quad2.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velNormal;
	impulseScalar /= (quad1.m_massData.m_invMass + quad2.m_massData.m_invMass);

	// Apply impulse
	Vector2 impulse = impulseScalar * normal;
	quad1.m_velocity -= quad1.m_massData.m_invMass * impulse;
}

void PhysicsScene::ResolveCollisionPhysicsQuadVsQuad(QuadEntity& quad1, QuadEntity& quad2, Vector2 normal)
{
	// Calculate relative velocity
	Vector2 relativeVelocity = quad2.m_velocity - quad1.m_velocity;

	// Calculate relative velocity in terms of the normal direction
	float velAlongNormal = DotProduct( relativeVelocity, normal );

	// Do not resolve if velocities are separating
	if(velAlongNormal > 0)
	{
		return;
	}

	// Calculate restitution
	float elasticity = 1.f * quad1.m_physicsMat.m_elasticity * quad2.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + elasticity) * velAlongNormal;
	impulseScalar /= (quad1.m_massData.m_invMass + quad2.m_massData.m_invMass);

	// apply impulse
	Vector2 impulse = impulseScalar * normal;
	quad1.m_velocity -= quad1.m_massData.m_invMass * impulse;
	quad2.m_velocity += quad2.m_massData.m_invMass * impulse;
}

void PhysicsScene::ResolveCollisionPhysicsQuadVsConstDisc(QuadEntity& quad, const DiscEntity& disc, Vector2 normal)
{
	Vector2 relVel = disc.m_velocity - quad.m_velocity;

	float velNormal = DotProduct(relVel, normal);

	if (velNormal > 0.f)
	{
		return;
	}

	// restitution
	float e = 1.f * disc.m_physicsMat.m_elasticity * quad.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velNormal;
	impulseScalar /= (quad.m_massData.m_invMass + disc.m_massData.m_invMass);

	// Apply impulse
	Vector2 impulse = impulseScalar * normal;
	quad.m_velocity -= quad.m_massData.m_invMass * impulse;
}

void PhysicsScene::ResolveCollisionPhysicsRecVsConstDisc(RectangleEntity& rec, const DiscEntity& disc, Vector2 normal)
{
	Vector2 relVel = disc.m_velocity - rec.m_velocity;

	float velNormal = DotProduct(relVel, normal);

	if (velNormal > 0.f)
	{
		return;
	}

	// restitution
	float e = 1.f * disc.m_physicsMat.m_elasticity * rec.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velNormal;
	impulseScalar /= (rec.m_massData.m_invMass + disc.m_massData.m_invMass);

	// Apply impulse
	Vector2 impulse = impulseScalar * normal;
	rec.m_velocity -= rec.m_massData.m_invMass * impulse;

	// friction
	float f = 1.f * rec.m_physicsMat.m_frictionCoef * disc.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relVel - (normal * velNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarTangent /= (rec.m_massData.m_invMass + disc.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	rec.m_velocity -= rec.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsRecVsConstRec(RectangleEntity& rec1, const RectangleEntity& rec2, Vector2 normal)
{
	Vector2 relVel = rec2.m_velocity - rec1.m_velocity;

	float velNormal = DotProduct(relVel, normal);

	if (velNormal > 0.f)
	{
		return;
	}

	// restitution
	float e = 1.f * rec1.m_physicsMat.m_elasticity * rec2.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1.f + e) * velNormal;
	impulseScalar /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass);

	// Apply impulse in normal direction
	Vector2 impulse = impulseScalar * normal;
	rec1.m_velocity -= rec1.m_massData.m_invMass * impulse;

	// friction
	float f = 1.f * rec1.m_physicsMat.m_frictionCoef * rec2.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relVel - (normal * velNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarTangent /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	rec1.m_velocity -= rec1.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsRecVsRec(RectangleEntity& rec1, RectangleEntity& rec2, Vector2 normal)
{
	// Calculate relative velocity
	Vector2 relativeVelocity = rec2.m_velocity - rec1.m_velocity;

	// Calculate relative velocity in terms of the normal direction
	float velAlongNormal = DotProduct( relativeVelocity, normal );

	// Do not resolve if velocities are separating
	if(velAlongNormal > 0)
	{
		return;
	}

	// Calculate restitution
	float e = 1.f * rec1.m_physicsMat.m_elasticity * rec2.m_physicsMat.m_elasticity;

	// Calculate impulse scalar
	float impulseScalar = -(1 + e) * velAlongNormal;
	impulseScalar /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass);

	// apply impulse
	Vector2 impulse = impulseScalar * normal;
	rec1.m_velocity -= rec1.m_massData.m_invMass * impulse;
	rec2.m_velocity += rec2.m_massData.m_invMass * impulse;

	// friction
	float f = 1.f * rec1.m_physicsMat.m_frictionCoef * rec2.m_physicsMat.m_frictionCoef;

	// tangent
	Vector2 tangent = -((relativeVelocity - (normal * velAlongNormal)).GetNormalized());
	float impulseScalarTangent = -(1.f + e) * DotProduct(relativeVelocity, tangent) * f;
	impulseScalarTangent /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass);

	// cap friction
	if (impulseScalarTangent > impulseScalar)
	{
		impulseScalarTangent = impulseScalar;
	}

	// apply tangent impulse
	Vector2 impulseTangent = impulseScalarTangent * tangent;
	rec1.m_velocity -= rec1.m_massData.m_invMass * impulseTangent;
	rec2.m_velocity += rec2.m_massData.m_invMass * impulseTangent;
}

void PhysicsScene::ResolveCollisionPhysicsRecVsRecAngular(RectangleEntity& rec1, RectangleEntity& rec2, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (rec2.m_massData.m_invMass / (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass));
	Vector2 weightedEnd = end * (rec1.m_massData.m_invMass / (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - rec1.m_center);
	Vector2 toP2 = -(p - rec2.m_center);

	Vector2 v1 = rec1.m_velocity + Vector2(-1.f * rec1.m_signedRotationVel * toP1.y, rec1.m_signedRotationVel * toP1.x);
	Vector2 v2 = rec2.m_velocity + Vector2(-1.f * rec2.m_signedRotationVel * toP2.y, rec2.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * rec1.m_physicsMat.m_frictionCoef * rec2.m_physicsMat.m_frictionCoef;
	float e = 1.f * rec1.m_physicsMat.m_elasticity * rec2.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * rec1.m_massData.m_inertia
		+ cross2Nz * cross2Nz * rec2.m_massData.m_inertia);

	// apply normal impulse
	rec1.m_signedRotationVel -= cross1Nz * impulseScalarN * rec1.m_massData.m_inertia;
	rec2.m_signedRotationVel += cross2Nz * impulseScalarN * rec2.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * rec1.m_massData.m_inertia
		+ cross2Tz * cross2Tz * rec2.m_massData.m_inertia);

	// apply tangent impulse
	rec1.m_signedRotationVel -= cross1Tz * impulseScalarT * rec1.m_massData.m_inertia;
	rec2.m_signedRotationVel += cross2Tz * impulseScalarT * rec2.m_massData.m_inertia;
}

void PhysicsScene::ResolveCollisionPhysicsRecVsConstRecAngular(RectangleEntity& rec1, const RectangleEntity& rec2, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (rec2.m_massData.m_invMass / (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass));
	Vector2 weightedEnd = end * (rec1.m_massData.m_invMass / (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - rec1.m_center);
	Vector2 toP2 = -(p - rec2.m_center);

	Vector2 v1 = rec1.m_velocity + Vector2(-1.f * rec1.m_signedRotationVel * toP1.y, rec1.m_signedRotationVel * toP1.x);
	Vector2 v2 = rec2.m_velocity + Vector2(-1.f * rec2.m_signedRotationVel * toP2.y, rec2.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * rec1.m_physicsMat.m_frictionCoef * rec2.m_physicsMat.m_frictionCoef;
	float e = 1.f * rec1.m_physicsMat.m_elasticity * rec2.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * rec1.m_massData.m_inertia
		+ cross2Nz * cross2Nz * rec2.m_massData.m_inertia);

	// apply normal impulse
	rec1.m_signedRotationVel -= cross1Nz * impulseScalarN * rec1.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (rec1.m_massData.m_invMass + rec2.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * rec1.m_massData.m_inertia
		+ cross2Tz * cross2Tz * rec2.m_massData.m_inertia);

	// apply tangent impulse
	rec1.m_signedRotationVel -= cross1Tz * impulseScalarT * rec1.m_massData.m_inertia;
}

void PhysicsScene::ResolveCollisionPhysicsRecVsConstDiscAngular(RectangleEntity& rec, const DiscEntity& disc, Vector2 normal, const Vector2& start, const Vector2& end)
{
	Vector2 weightedStart = start * (disc.m_massData.m_invMass / (rec.m_massData.m_invMass + disc.m_massData.m_invMass));
	Vector2 weightedEnd = end * (rec.m_massData.m_invMass / (rec.m_massData.m_invMass + disc.m_massData.m_invMass));
	Vector2 p = weightedStart + weightedEnd;

	Vector2 toP1 = -(p - rec.m_center);
	Vector2 toP2 = -(p - disc.m_center);

	Vector2 v1 = rec.m_velocity + Vector2(-1.f * rec.m_signedRotationVel * toP1.y, rec.m_signedRotationVel * toP1.x);
	Vector2 v2 = disc.m_velocity + Vector2(-1.f * disc.m_signedRotationVel * toP2.y, disc.m_signedRotationVel * toP2.x);
	Vector2 relVel = v2 - v1;
	float relVelInNormal = DotProduct(relVel, normal);

	float f = 1.f * rec.m_physicsMat.m_frictionCoef * disc.m_physicsMat.m_frictionCoef;
	float e = 1.f * rec.m_physicsMat.m_elasticity * disc.m_physicsMat.m_elasticity;

	Vector3 toP1V3 = toP1.ToVector3(0.f);
	Vector3 toP2V3 = toP2.ToVector3(0.f);
	Vector3 normal3 = normal.ToVector3(0.f);
	float cross1Nz = toP1V3.Cross(normal3).z;
	float cross2Nz = toP2V3.Cross(normal3).z;

	float impulseScalarN = -(1.f + e) * relVelInNormal;
	impulseScalarN /= (rec.m_massData.m_invMass + disc.m_massData.m_invMass 
		+ cross1Nz * cross1Nz * rec.m_massData.m_inertia
		+ cross2Nz * cross2Nz * disc.m_massData.m_inertia);

	// apply normal impulse
	rec.m_signedRotationVel -= cross1Nz * impulseScalarN * rec.m_massData.m_inertia;

	// impulse along tangent
	Vector2 tangent = -((relVel - (normal * relVelInNormal)).GetNormalized());
	Vector3 tangent3 = tangent.ToVector3(0.f);
	float cross1Tz = toP1V3.Cross(tangent3).z;
	float cross2Tz = toP2V3.Cross(tangent3).z;

	float impulseScalarT = -(1.f + e) * DotProduct(relVel, tangent) * f;
	impulseScalarT /= (rec.m_massData.m_invMass + disc.m_massData.m_invMass 
		+ cross1Tz * cross1Tz * rec.m_massData.m_inertia
		+ cross2Tz * cross2Tz * disc.m_massData.m_inertia);

	// apply tangent impulse
	rec.m_signedRotationVel -= cross1Tz * impulseScalarT * rec.m_massData.m_inertia;
}

void PhysicsScene::ProcessCorrectivePhysics()
{
	ProcessDiscsCorrective();
	ProcessRectanglesCorrective();
	ProcessQuadsCorrective();
}

void PhysicsScene::ProcessDiscsCorrective()
{
	// disc vs disc
	for (uint disc1Idx = 0; disc1Idx < m_discEntities.size(); ++disc1Idx)
	{
		DiscEntity* discEntity1 = m_discEntities[disc1Idx];

		for (uint disc2Idx = disc1Idx + 1; disc2Idx < m_discEntities.size(); ++disc2Idx)
		{
			DiscEntity* discEntity2 = m_discEntities[disc2Idx];

			//bool broadPhasePassed = BroadPhaseCirclePrune(discEntity1->m_boundCircle, discEntity2->m_boundCircle);
			bool broadPhasePassed = BroadPhaseCirclePrune(*discEntity1, *discEntity2);
			if (!broadPhasePassed)
			{
				continue;				// detect next pair
			}
			else
			{
				// some visuals showing entities passed the broadphase
				discEntity1->m_passedBroadphase = true;
				discEntity2->m_passedBroadphase = true;
			}

			if (discEntity1->m_const)
			{
				if (discEntity2->m_const)
				{
					continue;
				}
				else
				{
					PhysicsDiscVsConstDiscCorrective(*discEntity2, *discEntity1);
				}
			}
			else
			{
				if (discEntity2->m_const)
				{
					PhysicsDiscVsConstDiscCorrective(*discEntity1, *discEntity2);
				}
				else
				{
					PhysicsDiscVsPhysicsDiscCorrective(*discEntity1, *discEntity2);
				}
			}
		}
	}

	// disc vs quad
	/*
	for (uint discIdx = 0; discIdx < m_sphereEntities.size(); ++discIdx)
	{
	DiscEntity* disc = m_sphereEntities[discIdx];
	for (uint quadIdx = 0; quadIdx < m_quadEntities.size(); ++quadIdx)
	{
	QuadEntity* quad = m_quadEntities[quadIdx];
	if (disc->m_const)
	{
	if (quad->m_const)
	{
	continue;
	}
	else
	{
	// non const quad onto const disc, quad will handle this
	}
	}
	else
	{
	if (quad->m_const)
	{
	PhysicsDiscVsConstantQuadCorrective(*disc, *quad);
	}
	else
	{
	PhysicsDiscVsPhysicsQuadCorrective(*disc, *quad);
	}
	}
	}
	}
	*/

	// disc vs rec
	for (uint discIdx = 0; discIdx < m_discEntities.size(); ++discIdx)
	{
		DiscEntity* disc = m_discEntities[discIdx];

		for (uint recIdx = 0; recIdx < m_rectangleEntities.size(); ++recIdx)
		{
			RectangleEntity* rec = m_rectangleEntities[recIdx];

			//bool broadPhasePassed = BroadPhaseCirclePrune(disc->m_boundCircle, rec->m_boundCircle);
			bool broadPhasePassed = BroadPhaseCirclePrune(*disc, *rec);
			if (!broadPhasePassed)
			{
				continue;
			}
			else
			{
				disc->m_passedBroadphase = true;
				rec->m_passedBroadphase = true;
			}

			if (disc->m_const)
			{
				if (rec->m_const)
				{
					continue;
				}
				else
				{
					// non const rec onto const disc, rec will handle this

				}
			}
			else
			{
				if (rec->m_const)
				{
					PhysicsDiscVsConstRecCorrective(*disc, *rec);
				}
				else
				{
					PhysicsDiscVsPhysicsRecCorrective(*disc, *rec);
				}
			}
		}
	}

	/*
	// disc vs const capsule
	for (uint capsuleIdx = 0; capsuleIdx < m_capsuleEntities.size(); ++capsuleIdx)
	{
		CapsuleEntity* capsuleEntity = m_capsuleEntities[capsuleIdx];

		for (uint discIdx = 0; discIdx < m_sphereEntities.size(); ++discIdx)
		{
			DiscEntity* discEntity = m_sphereEntities[discIdx];

			if (discEntity->m_const)
			{
				continue;
			}
			else
			{
				// by default all capsules are const for now
				PhysicsDiscVsConstantCapsuleCorrective(*discEntity, *capsuleEntity);
			}
		}
	}
	*/
}

void PhysicsScene::PhysicsDiscVsPhysicsDiscCorrective(DiscEntity& disc1, DiscEntity& disc2)
{
	Manifold* manifold = new Manifold(&disc1, &disc2);

	bool collided = DiscVsDiscManifold(manifold);

	if (collided)
	{
		Vector2 disc1Push = -manifold->m_normal * manifold->m_penetration / 2.f;
		Vector2 disc2Push = manifold->m_normal * manifold->m_penetration / 2.f;

		disc1.TranslateEntity(disc1Push);
		disc2.TranslateEntity(disc2Push);

		Vector2 toDisc2Normal = manifold->m_normal;
		ResolveCollisionPhysicsDiscVsDisc(disc1, disc2, toDisc2Normal);

		ResolveCollisionPhysicsDiscVsDiscAngular(disc1, disc2, toDisc2Normal,
			manifold->m_start, manifold->m_end);
	}

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsDiscVsConstDiscCorrective(DiscEntity& moveDisc, const DiscEntity& constantDisc)
{
	DiscEntity* copySphereEntity = new DiscEntity(constantDisc);

	Manifold* manifold = new Manifold(&moveDisc, copySphereEntity);

	bool collided = DiscVsDiscManifold(manifold);

	if (collided)
	{
		Vector2 moveDiscPush = -manifold->m_normal * manifold->m_penetration;

		moveDisc.TranslateEntity(moveDiscPush);

		// impulse resolution
		Vector2 toConstDiscNormal = manifold->m_normal;
		ResolveCollisionPhysicsDiscVsConstDisc(moveDisc, constantDisc, toConstDiscNormal);

		ResolveCollisionPhysicsDiscVsConstDiscAngular(moveDisc, constantDisc, toConstDiscNormal,
			manifold->m_start, manifold->m_end);
	}

	delete copySphereEntity;
	copySphereEntity = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsDiscVsPhysicsQuadCorrective(DiscEntity& disc, QuadEntity& quad)
{
	Manifold* manifold = new Manifold(&quad, &disc);

	bool collided = AABB2VsDisc2Manifold(manifold);

	if (collided)
	{
		Vector2 discPush = -manifold->m_normal * manifold->m_penetration / 2.f;
		Vector2 quadPush = manifold->m_normal * manifold->m_penetration / 2.f;

		disc.TranslateEntity(discPush);
		quad.TranslateEntity(quadPush);

		Vector2 toQuadNormal = manifold->m_normal;
		ResolveCollisionPhysicsDiscVsQuad(disc, quad, toQuadNormal);
	}

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsDiscVsConstQuadCorrective(DiscEntity& disc, const QuadEntity& quad)
{
	QuadEntity* copyQuadEntity = new QuadEntity(quad);

	Manifold* manifold = new Manifold(copyQuadEntity, &disc);

	bool collided = AABB2VsDisc2Manifold(manifold);

	if (collided)
	{
		Vector2 moveDiscPush = -manifold->m_normal * manifold->m_penetration;

		disc.TranslateEntity(moveDiscPush);

		Vector2 toQuadNormal = manifold->m_normal;
		ResolveCollisionPhysicsDiscVsConstQuad(disc, quad, toQuadNormal);
	}

	delete copyQuadEntity;
	copyQuadEntity = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsDiscVsPhysicsRecCorrective(DiscEntity& disc, RectangleEntity& rec)
{
	Manifold* manifold = new Manifold(&rec, &disc);

	bool collided = OBB2VsDiscManifold(manifold);

	if (collided)
	{
		Vector2 discPush = -manifold->m_normal * manifold->m_penetration / 2.f;
		Vector2 recPush = manifold->m_normal * manifold->m_penetration / 2.f;

		disc.TranslateEntity(discPush);
		rec.TranslateEntity(recPush);

		Vector2 toRecNormal = manifold->m_normal;
		ResolveCollisionPhysicsDiscVsRec(disc, rec, toRecNormal);

		ResolveCollisionPhysicsDiscVsRecAngular(disc, rec, toRecNormal, manifold->m_start, manifold->m_end);
	}

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsDiscVsConstRecCorrective(DiscEntity& disc, const RectangleEntity& rec)
{
	RectangleEntity* copyRec = new RectangleEntity(rec);

	Manifold* manifold = new Manifold(copyRec, &disc);

	bool collided = OBB2VsDiscManifold(manifold);

	if (collided)
	{
		Vector2 moveDiscPush = -manifold->m_normal * manifold->m_penetration;

		disc.TranslateEntity(moveDiscPush);

		Vector2 toRecNormal = manifold->m_normal;
		ResolveCollisionPhysicsDiscVsConstRec(disc, rec, toRecNormal);

		ResolveCollisionPhysicsDiscVsConstRecAngular(disc, rec, toRecNormal, manifold->m_start, manifold->m_end);
	}

	delete copyRec;
	copyRec = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::ProcessQuadsCorrective()
{
	// quad vs quad
	for (uint quad1Idx = 0; quad1Idx < m_quadEntities.size(); ++quad1Idx)
	{
		QuadEntity* quad1 = m_quadEntities[quad1Idx];

		for (uint quad2Idx = quad1Idx + 1; quad2Idx < m_quadEntities.size(); ++quad2Idx)
		{
			QuadEntity* quad2 = m_quadEntities[quad2Idx];

			if (quad1->m_const)
			{
				if (quad2->m_const)
				{
					continue;
				}
				else
				{
					PhysicsQuadVsConstQuadCorrective(*quad2, *quad1);
				}
			}
			else
			{
				if (quad2->m_const)
				{
					PhysicsQuadVsConstQuadCorrective(*quad1, *quad2);
				}
				else
				{
					PhysicsQuadVsPhysicsQuadCorrective(*quad1, *quad2);
				}
			}
		}
	}

	/*
	// quad vs disc
	for (uint quadIdx = 0; quadIdx < m_quadEntities.size(); ++quadIdx)
	{
	QuadEntity* quad = m_quadEntities[quadIdx];
	for (uint discIdx = 0; discIdx < m_sphereEntities.size(); ++discIdx)
	{
	DiscEntity* disc = m_sphereEntities[discIdx];
	if (quad->m_const)
	{
	if (disc->m_const)
	{
	continue;
	}
	else
	{
	// non const disc onto const quad, handled by disc
	}
	}
	else
	{
	if (disc->m_const)
	{
	PhysicsQuadVsConstDiscCorrective(*quad, *disc);
	}
	else
	{
	// handled by disc

	}
	}
	}
	}
	// quad vs const capsule
	for (uint capsuleIdx = 0; capsuleIdx < m_capsuleEntities.size(); ++capsuleIdx)
	{
	CapsuleEntity* capsuleEntity = m_capsuleEntities[capsuleIdx];
	for (uint quadIdx = 0; quadIdx < m_quadEntities.size(); ++quadIdx)
	{
	QuadEntity* quadEntity = m_quadEntities[quadIdx];
	if (quadEntity->m_const)
	{
	continue;
	}
	else
	{
	// by default all capsules are const for now
	PhysicsQuadVsConstCapsuleCorrective(*quadEntity, *capsuleEntity);
	}
	}
	}
	*/
}

void PhysicsScene::PhysicsQuadVsConstQuadCorrective(QuadEntity& quad1, const QuadEntity& quad2)
{
	QuadEntity* copyQuadEntity = new QuadEntity(quad2);

	Manifold* manifold = new Manifold(&quad1, copyQuadEntity);

	bool collided = AABB2VsAABB2Manifold(manifold);

	if (collided)
	{
		Vector2 quad1Push = -manifold->m_normal * manifold->m_penetration;

		quad1.TranslateEntity(quad1Push);

		//Vector2 toQuad2Normal = manifold->m_normal;
		//ResolveCollisionPhysicsQuadVsConstQuad(quad1, quad2, toQuad2Normal);
	}

	delete copyQuadEntity;
	copyQuadEntity = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsQuadVsPhysicsQuadCorrective(QuadEntity& quad1, QuadEntity& quad2)
{
	Manifold* manifold = new Manifold(&quad1, &quad2);

	bool collided = AABB2VsAABB2Manifold(manifold);

	if (collided)
	{
		//Vector2 quad1Push = -manifold->m_normal * manifold->m_penetration / 2.f;
		//Vector2 quad2Push = manifold->m_normal * manifold->m_penetration / 2.f;

		//quad1.TranslateEntity(quad1Push);
		//quad2.TranslateEntity(quad2Push);

		//Vector2 toQuad2Normal = manifold->m_normal;
		//ResolveCollisionPhysicsQuadVsQuad(quad1, quad2, toQuad2Normal);
	}

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsQuadVsConstDiscCorrective(QuadEntity& quad, const DiscEntity& disc)
{
	DiscEntity* copySphereEntity = new DiscEntity(disc);

	Manifold* manifold = new Manifold(&quad, copySphereEntity);

	bool collided = AABB2VsDisc2Manifold(manifold);

	if (collided)
	{
		Vector2 quadPush = manifold->m_normal * manifold->m_penetration;		// since collided disc to quad

		quad.TranslateEntity(quadPush);

		Vector2 toDiscNormal = -manifold->m_normal;
		ResolveCollisionPhysicsQuadVsConstDisc(quad, disc, toDiscNormal);
	}

	delete copySphereEntity;
	copySphereEntity = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::ProcessRectanglesCorrective()
{
	// rec vs rec
	for (uint recIdx1 = 0; recIdx1 < m_rectangleEntities.size(); ++recIdx1)
	{
		RectangleEntity* rec1 = m_rectangleEntities[recIdx1];

		for (uint recIdx2 = recIdx1 + 1; recIdx2 < m_rectangleEntities.size(); ++recIdx2)
		{
			RectangleEntity* rec2 = m_rectangleEntities[recIdx2];

			//bool broadPhasePassed = BroadPhaseCirclePrune(rec1->m_boundCircle, rec2->m_boundCircle);
			bool broadPhasePassed = BroadPhaseCirclePrune(*rec1, *rec2);
			if (!broadPhasePassed)
			{
				continue;
			}
			else
			{
				rec1->m_passedBroadphase = true;
				rec2->m_passedBroadphase = true;
			}

			if (rec1->m_const)
			{
				if (rec2->m_const)
				{
					continue;
				}
				else
				{
					PhysicsRecVsConstRecCorrective(*rec2, *rec1);
				}
			}
			else
			{
				if (rec2->m_const)
				{
					PhysicsRecVsConstRecCorrective(*rec1, *rec2);
				}
				else
				{
					PhysicsRecVsPhysicsRecCorrective(*rec1, *rec2);
				}
			}
		}
	}

	// rec vs disc
	for (uint recIdx = 0; recIdx < m_rectangleEntities.size(); ++recIdx)
	{
		RectangleEntity* rec = m_rectangleEntities[recIdx];

		for (uint discIdx = 0; discIdx < m_discEntities.size(); ++discIdx)
		{
			DiscEntity* disc = m_discEntities[discIdx];

			//bool broadPhasePassed = BroadPhaseCirclePrune(rec->m_boundCircle, disc->m_boundCircle);
			bool broadPhasePassed = BroadPhaseCirclePrune(*rec, *disc);
			if (!broadPhasePassed)
			{
				continue;
			}
			else
			{
				rec->m_passedBroadphase = true;
				disc->m_passedBroadphase = true;
			}

			if (rec->m_const)
			{
				if (disc->m_const)
				{
					continue;
				}
				else
				{
					// non const disc onto const quad, handled by disc

				}
			}
			else
			{
				if (disc->m_const)
				{
					PhysicsRecVsConstDiscCorrective(*rec, *disc);
				}
				else
				{
					// handled by disc

				}
			}
		}
	}

	/*
	// rec vs const capsule
	for (uint capsuleIdx = 0; capsuleIdx < m_capsuleEntities.size(); ++capsuleIdx)
	{
		CapsuleEntity* capsuleEntity = m_capsuleEntities[capsuleIdx];

		for (uint recIdx = 0; recIdx < m_rectangleEntities.size(); ++recIdx)
		{
			RectangleEntity* recEntity = m_rectangleEntities[recIdx];

			if (recEntity->m_const)
			{
				continue;
			}
			else
			{
				// by default all capsules are const for now
				PhysicsRecVsConstCapsuleCorrective(*recEntity, *capsuleEntity);
			}
		}
	}
	*/
}

void PhysicsScene::PhysicsRecVsConstRecCorrective(RectangleEntity& rec1, const RectangleEntity& rec2)
{
	RectangleEntity* copyRecEntity = new RectangleEntity(rec2);

	Manifold* manifold = new Manifold(&rec1, copyRecEntity);

	bool collided = OBB2VsOBB2Manifold(manifold);

	if (collided)
	{
		Vector2 rec1Push = -manifold->m_normal * manifold->m_penetration;

		rec1.TranslateEntity(rec1Push);

		Vector2 toRec2Normal = manifold->m_normal;
		ResolveCollisionPhysicsRecVsConstRec(rec1, rec2, toRec2Normal);

		ResolveCollisionPhysicsRecVsConstRecAngular(rec1, rec2, toRec2Normal, manifold->m_start, manifold->m_end);
	}

	delete copyRecEntity;
	copyRecEntity = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsRecVsPhysicsRecCorrective(RectangleEntity& rec1, RectangleEntity& rec2)
{
	Manifold* manifold = new Manifold(&rec1, &rec2);

	bool collided = OBB2VsOBB2Manifold(manifold);

	if (collided)
	{
		Vector2 rec1Push = -manifold->m_normal * manifold->m_penetration / 2.f;
		Vector2 rec2Push = manifold->m_normal * manifold->m_penetration / 2.f;

		rec1.TranslateEntity(rec1Push);
		rec2.TranslateEntity(rec2Push);

		Vector2 toRec2Normal = manifold->m_normal;
		ResolveCollisionPhysicsRecVsRec(rec1, rec2, toRec2Normal);

		// resolve angular impulse
		ResolveCollisionPhysicsRecVsRecAngular(rec1, rec2, toRec2Normal, manifold->m_start, manifold->m_end);
	}

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::PhysicsRecVsConstDiscCorrective(RectangleEntity& rec, const DiscEntity& disc)
{
	DiscEntity* copySphereEntity = new DiscEntity(disc);

	Manifold* manifold = new Manifold(&rec, copySphereEntity);

	bool collided = OBB2VsDiscManifold(manifold);

	if (collided)
	{
		Vector2 recPush = manifold->m_normal * manifold->m_penetration;		// collided disc to rec

		rec.TranslateEntity(recPush);

		Vector2 toDiscNormal = -manifold->m_normal;
		ResolveCollisionPhysicsRecVsConstDisc(rec, disc, toDiscNormal);

		ResolveCollisionPhysicsRecVsConstDiscAngular(rec, disc, toDiscNormal, manifold->m_start, manifold->m_end);
	}

	delete copySphereEntity;
	copySphereEntity = nullptr;

	delete manifold;
	manifold = nullptr;
}

void PhysicsScene::ProcessPreventativePhysics(float deltaTime)
{
	// disc vs disc
	for (uint disc1Idx = 0; disc1Idx < m_discEntities.size(); ++disc1Idx)
	{
		DiscEntity* discEntity1 = m_discEntities[disc1Idx];

		for (uint disc2Idx = disc1Idx + 1; disc2Idx < m_discEntities.size(); ++disc2Idx)
		{
			DiscEntity* discEntity2 = m_discEntities[disc2Idx];

			if (discEntity1->m_const)
			{
				if (discEntity2->m_const)
				{
					continue;
				}
				else
				{
					PhysicsDiscVsConstDiscPreventative(*discEntity2, *discEntity1, deltaTime);
				}
			}
			else
			{
				if (discEntity2->m_const)
				{
					PhysicsDiscVsConstDiscPreventative(*discEntity1, *discEntity2, deltaTime);
				}
				else
				{
					PhysicsDiscVsPhysicsDiscPreventative(*discEntity1, *discEntity2, deltaTime);
				}
			}
		}
	}

	/*
	// disc vs const capsule
	for (uint capsuleIdx = 0; capsuleIdx < m_capsuleEntities.size(); ++capsuleIdx)
	{
		CapsuleEntity* capsuleEntity = m_capsuleEntities[capsuleIdx];

		for (uint discIdx = 0; discIdx < m_sphereEntities.size(); ++discIdx)
		{
			DiscEntity* discEntity = m_sphereEntities[discIdx];

			if (discEntity->m_const)
			{
				continue;
			}
			else
			{
				// by default all capsules are const for now
				PhysicsDiscVsConstantCapsulePreventative(*discEntity, *capsuleEntity, deltaTime);
			}
		}
	}
	*/
}

void PhysicsScene::PhysicsDiscVsPhysicsDiscPreventative(DiscEntity& disc1, DiscEntity& disc2, float deltaTime)
{
	Vector2 collideOverlap;

	Disc2 primitive1 = disc1.GenerateDisc2();
	Disc2 primitive2 = disc2.GenerateDisc2();
	Vector2 vel1 = disc1.m_velocity;
	Vector2 vel2 = disc2.m_velocity;

	primitive1.center += deltaTime * vel1;
	primitive2.center += deltaTime * vel2;

	bool collided = DoDiscsOverlap(primitive1, primitive2);

	if (collided)
	{
		if (disc1.m_kinematic)
		{
			disc1.m_center -= deltaTime * vel1;
		}
		if (disc2.m_kinematic)
		{
			disc2.m_center -= deltaTime * vel2;
		}

		// do overlap next frame, reflect now
		ReflectDiscVsDisc(disc1, disc2);
	}
}

void PhysicsScene::PhysicsDiscVsConstDiscPreventative(DiscEntity& moveDisc, const DiscEntity& constantDisc, float deltaTime)
{
	Vector2 collideOverlap;

	DiscEntity* copySphereEntity = new DiscEntity(constantDisc);
	Disc2 constPrimitive = copySphereEntity->GenerateDisc2();

	Disc2 primitive = moveDisc.GenerateDisc2();
	Vector2 vel = moveDisc.m_velocity;

	primitive += deltaTime * vel;

	bool collided = DoDiscsOverlap(primitive, constPrimitive);

	if (collided)
	{
		if (moveDisc.m_kinematic)
		{
			moveDisc.m_center -= deltaTime * vel;
		}

		ReflectDiscVsConstDisc(moveDisc, constantDisc);
	}

	delete copySphereEntity;
	copySphereEntity = nullptr;
}

void PhysicsScene::ProcessContinuousPhysics(float deltaTime)
{
	// this is applied only in minkowski state for now
	// meaning that we only process quads for now
	ProcessQuadsContinuous(deltaTime);
}


void PhysicsScene::ProcessQuadsContinuous(float deltaTime)
{
	// we only care quad vs quad for now
	for (uint quad1Idx = 0; quad1Idx < m_quadEntities.size(); ++quad1Idx)
	{
		QuadEntity* quad1 = m_quadEntities[quad1Idx];

		for (uint quad2Idx = quad1Idx + 1; quad2Idx < m_quadEntities.size(); ++quad2Idx)
		{
			QuadEntity* quad2 = m_quadEntities[quad2Idx];

			if (quad1->m_const)
			{
				if (quad2->m_const)
				{
					continue;
				}
				else
				{
					PhysicsQuadVsConstQuadContinuous(*quad2, *quad1, deltaTime);
				}
			}
			else
			{
				if (quad2->m_const)
				{
					PhysicsQuadVsConstQuadContinuous(*quad1, *quad2, deltaTime);
				}
				else
				{
					PhysicsQuadVsPhysicsQuadContinuous(*quad1, *quad2, deltaTime);
				}
			}
		}
	}
}

void PhysicsScene::PhysicsQuadVsConstQuadContinuous(QuadEntity& quad1, const QuadEntity& quad2, float deltaTime)
{
	AABB2 q1 = quad1.m_primitive;
	AABB2 q2 = quad2.m_primitive;

	AABB2 minkowskiAABB = MinkowskiAABBVsAABB(q1, q2);

	bool colliding = minkowskiAABB.IsPointInside(Vector2::ZERO);

	if (colliding)
	{
		Vector2 penetration = minkowskiAABB.GetClosestOnBoundPoint(Vector2::ZERO);
		quad1.m_center -= penetration;
	}
	else
	{
		if (!quad1.m_kinematic)
		{
			Vector2 relativeMotion = (quad2.m_velocity - quad1.m_velocity) * deltaTime;

			float h = minkowskiAABB.GetRayIntersection(Vector2::ZERO, relativeMotion);

			if(h < INFINITY)
			{
				quad1.m_continued = true;

				quad1.m_center += quad1.m_velocity * deltaTime * h;

				Vector2 normal = relativeMotion.GetNormalized();
				Vector2 tangent = Vector2(-normal.y, normal.x);
				quad1.m_velocity = DotProduct(quad1.m_velocity, tangent) * tangent;
			}
		}
	}
}

void PhysicsScene::PhysicsQuadVsPhysicsQuadContinuous(QuadEntity&, QuadEntity&, float)
{

}

void PhysicsScene::ProcessPairsCorrective()
{
	for each (sCollisionPair* pair in m_pairs)
	{
		Entity* e1 = pair->m_e1;
		Entity* e2 = pair->m_e2;

		RectangleEntity* r1 = dynamic_cast<RectangleEntity*>(e1);
		DiscEntity* d1 = dynamic_cast<DiscEntity*>(e1);
		RectangleEntity* r2 = dynamic_cast<RectangleEntity*>(e2);
		DiscEntity* d2 = dynamic_cast<DiscEntity*>(e2);

		if (r1 != nullptr && r2 != nullptr)
		{
			// rec vs rec
			bool const1 = r1->m_const;
			bool const2 = r2->m_const;

			if (!const1 && !const2)
			{
				// both rec are not const
				PhysicsRecVsPhysicsRecCorrective(*r1, *r2);
			}
			else if (const1 && !const2)
			{
				PhysicsRecVsConstRecCorrective(*r2, *r1);
			}
			else if (const2 && !const1)
			{
				PhysicsRecVsConstRecCorrective(*r1, *r2);
			}
			else
			{
				// else both rec are const, no need to process
			}
		}
		else if (d1 != nullptr && d2 != nullptr)
		{
			// disc vs disc
			bool const1 = d1->m_const;
			bool const2 = d2->m_const;

			if (!const1 && !const2)
			{
				// both discs are not const
				PhysicsDiscVsPhysicsDiscCorrective(*d1, *d2);
			}
			else if (const1 && !const2)
			{
				PhysicsDiscVsConstDiscCorrective(*d2, *d1);
			}
			else if (const2 && !const1)
			{
				PhysicsDiscVsConstDiscCorrective(*d1, *d2);
			}
			else
			{
				// else both rec are const, no need to process
			}
		}
		else if (r1 != nullptr && d2 != nullptr)
		{
			// e1 is a rec and e2 is a disc
			// rec vs disc
			bool recConst = r1->m_const;
			bool discConst = d2->m_const;

			if (!recConst && !discConst)
			{
				// both rec and disc are not const
				PhysicsDiscVsPhysicsRecCorrective(*d2, *r1);
			}
			else if (recConst && !discConst)
			{
				// rec is const but disc is not
				PhysicsDiscVsConstRecCorrective(*d2, *r1);
			}
			else if (discConst && !recConst)
			{
				// disc is const but rec is not
				PhysicsRecVsConstDiscCorrective(*r1, *d2);
			}
			else
			{
				// both are const, no need to process
			}
		}
		else if (d1 != nullptr && r2 != nullptr)
		{
			// e1 is a disc and e2 is a rec
			// disc vs rec
			bool discConst = d1->m_const;
			bool recConst = r2->m_const;

			if (!discConst && !recConst)
			{
				// both are not const
				PhysicsDiscVsPhysicsRecCorrective(*d1, *r2);
			}
			else if (discConst && !recConst)
			{
				// disc is const and rec is not
				PhysicsRecVsConstDiscCorrective(*r2, *d1);
			}
			else if (recConst && !discConst)
			{
				// rec is const and disc is not
				PhysicsDiscVsConstRecCorrective(*d1, *r2);
			}
			else
			{
				// both are const no need to process
			}
		}
		else
		{
			// this should not happen
		}
	}
}

void PhysicsScene::ReflectDiscVsConstDisc(DiscEntity& moveDisc, const DiscEntity& constDisc)
{
	// after correcting, reflect velocity
	if (!moveDisc.m_kinematic)
	{
		float elasticity = 1.f * moveDisc.m_physicsMat.m_elasticity * constDisc.m_physicsMat.m_elasticity;
		//float friction = 1.f * moveDisc.m_frictionCoef * constDisc.m_frictionCoef;

		Vector2 correctedCenter = moveDisc.m_center;
		Vector2 constCenter = constDisc.m_center;
		Vector2 reflectNormal = (correctedCenter - constCenter).GetNormalized();
		Vector2 moveDiscVelocity = moveDisc.m_velocity;
		float dot = DotProduct(moveDiscVelocity, reflectNormal);
		Vector2 reflectedVelocity = moveDiscVelocity - 2.f * dot * reflectNormal;		// d - 2(d.n)n

																						// elasticity affect vertical component
		Vector2 vertical = DotProduct(reflectedVelocity, reflectNormal) * reflectNormal;
		Vector2 horizontal = reflectedVelocity - vertical;
		vertical = vertical * elasticity;
		//horizontal = horizontal * (1.f - friction);
		reflectedVelocity = vertical + horizontal;

		moveDisc.m_velocity = reflectedVelocity;
	}
}

void PhysicsScene::ReflectDiscVsDisc(DiscEntity& disc1, DiscEntity& disc2)
{
	float elasticity = 1.f * disc1.m_physicsMat.m_elasticity * disc2.m_physicsMat.m_elasticity;
	//float friction = 1.f * disc1.m_frictionCoef * disc2.m_frictionCoef;

	Vector2 center1 = disc1.m_center;
	Vector2 center2 = disc2.m_center;

	Vector2 reflectedNormal1 = (center1 - center2).GetNormalized();
	Vector2 reflectedNormal2 = -reflectedNormal1;

	// reflection for 1
	if (!disc1.m_kinematic)
	{
		Vector2 vel1 = disc1.m_velocity;
		float dot1 = DotProduct(vel1, reflectedNormal1);
		Vector2 reflect1 = vel1 - 2.f * dot1 * reflectedNormal1;

		Vector2 vertical = DotProduct(reflect1, reflectedNormal1) * reflectedNormal1;
		Vector2 horizontal = reflect1 - vertical;
		vertical = vertical * elasticity;
		//horizontal = horizontal * (1.f - friction);
		reflect1 = vertical + horizontal;

		disc1.m_velocity = reflect1;
	}

	// reflection for 2
	if (!disc2.m_kinematic)
	{
		Vector2 vel2 = disc2.m_velocity;
		float dot2 = DotProduct(vel2, reflectedNormal2);
		Vector2 reflect2 = vel2 - 2.f * dot2 * reflectedNormal2;

		Vector2 vertical = DotProduct(reflect2, reflectedNormal2) * reflectedNormal2;
		Vector2 horizontal = reflect2 - vertical;
		vertical = vertical * elasticity;
		//horizontal = horizontal * (1.f - friction);
		reflect2 = vertical + horizontal;

		disc2.m_velocity = reflect2;
	}
}

void PhysicsScene::ProcessSAP()
{
	// for all entities, sweep and prune along x axis
	SortAxisList(AXIS_X);
	// entities in axisList sorted

	// for all sorted entities, extract collision pairs
	ProcessActiveList(AXIS_X);
	// pair list filled

	ProcessPairsCorrective();
}

void PhysicsScene::ProcessQuadtree()
{
	m_tree->ClearTree();
	for each (DiscEntity* disc in m_discEntities)
	{
		m_tree->InsertTree(disc);
	}
	//for each (RectangleEntity* rec in m_rectangleEntities)
	//{
	//	m_tree->InsertTree(rec);
	//}

	std::vector<Entity*> objs;
	for each (DiscEntity* disc in m_discEntities)
	{
		objs.clear();
		m_tree->RetrieveEntity(objs, disc);

		for each (Entity* ent in objs)
		{
			DiscEntity* discEnt = dynamic_cast<DiscEntity*>(ent);
			//RectangleEntity* recEnt = dynamic_cast<RectangleEntity*>(ent);
			if (discEnt != nullptr)
			{
				if (disc != discEnt)
				{
					disc->m_passedBroadphase = true;
					discEnt->m_passedBroadphase = true;

					// both non-const
					if (!disc->m_const && !discEnt->m_const)
					{
						PhysicsDiscVsPhysicsDiscCorrective(*disc, *discEnt);
					}
					// one const, one non-const
					else if (disc->m_const && !discEnt->m_const)
					{
						PhysicsDiscVsConstDiscCorrective(*discEnt, *disc);
					}
					else if (!disc->m_const && discEnt->m_const)
					{
						PhysicsDiscVsConstDiscCorrective(*disc, *discEnt);
					}
					// both const
					else
					{
						// do nothing
					}
				}
			}
			/*
			else if (recEnt != nullptr)
			{
				disc->m_passedBroadphase = true;
				recEnt->m_passedBroadphase = true;

				// both non-const
				if (!disc->m_const && !recEnt->m_const)
				{
					PhysicsDiscVsPhysicsRecCorrective(*disc, *recEnt);
				}
				// one const, one non-const
				else if (disc->m_const && !recEnt->m_const)
				{
					PhysicsRecVsConstDiscCorrective(*recEnt, *disc);
				}
				else if (!disc->m_const && recEnt->m_const)
				{
					PhysicsDiscVsConstRecCorrective(*disc, *recEnt);
				}
				// both const
				else
				{
					// do nothing
				}
			}
			*/
		}
	}
	/*
	for each (RectangleEntity* rec in m_rectangleEntities)
	{
		objs.clear();
		m_tree->RetrieveEntity(objs, rec);

		for each (Entity* ent in objs)
		{
			DiscEntity* discEnt = dynamic_cast<DiscEntity*>(ent);
			RectangleEntity* recEnt = dynamic_cast<RectangleEntity*>(ent);
			if (discEnt != nullptr)
			{
				rec->m_passedBroadphase = true;
				discEnt->m_passedBroadphase = true;

				// both non-const
				if (!rec->m_const && !discEnt->m_const)
				{
					PhysicsDiscVsPhysicsRecCorrective(*discEnt, *rec);
				}
				// one const, one non-const
				else if (rec->m_const && !discEnt->m_const)
				{
					PhysicsDiscVsConstRecCorrective(*discEnt, *rec);
				}
				else if (!rec->m_const && discEnt->m_const)
				{
					PhysicsRecVsConstDiscCorrective(*rec, *discEnt);
				}
				// both const
				else
				{
					// do nothing
				}
			}
			else if (recEnt != nullptr)
			{
				if (rec != recEnt)
				{
					rec->m_passedBroadphase = true;
					recEnt->m_passedBroadphase = true;

					// both non-const
					if (!rec->m_const && !recEnt->m_const)
					{
						PhysicsRecVsPhysicsRecCorrective(*rec, *recEnt);
					}
					// one const, one non-const
					else if (rec->m_const && !recEnt->m_const)
					{
						PhysicsRecVsConstRecCorrective(*recEnt, *rec);
					}
					else if (!rec->m_const && recEnt->m_const)
					{
						PhysicsRecVsConstRecCorrective(*rec, *recEnt);
					}
					// both const
					else
					{
						// do nothing
					}
				}
			}
		}
	}
	*/
}