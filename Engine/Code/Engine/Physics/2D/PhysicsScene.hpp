#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Quadtree.hpp"
#include "Engine/Physics/2D/DiscEntity.hpp"
#include "Engine/Physics/2D/RectangleEntity.hpp"
#include "Engine/Physics/2D/QuadEntity.hpp"
#include "Engine/Physics/PhysicsPhase.hpp"

#include <vector>

#define GRAVITY_CONST 9.8f

struct sCollisionPair
{
	Entity* m_e1;
	Entity* m_e2;

	sCollisionPair(Entity* e1, Entity* e2);
	~sCollisionPair();
};

enum eProcessScheme
{
	PHYSICS_CORRECTIVE,
	PHYSICS_PREVENTATIVE,
	PHYSICS_CONTINUOUS,
	NUM_OF_SCHEME
};

enum eBroadPhaseScheme
{
	BP_BOUND,
	BP_SAP,
	BP_QUAD,
	NUM_BP
};

class PhysicsScene
{
public:
public:
	bool		   m_drawBP = false;
	bool		   m_gravityOn = true;
	Vector2		   m_graivty;

	eProcessScheme m_scheme;
	eBroadPhaseScheme m_bpScheme;

	std::vector<QuadEntity*>      m_quadEntities;
	std::vector<DiscEntity*>	  m_discEntities;
	//std::vector<CapsuleEntity*>   m_capsuleEntities;
	std::vector<RectangleEntity*> m_rectangleEntities;

	std::vector<Entity*>		  m_axisList;
	std::vector<Entity*>		  m_activeList;
	std::vector<sCollisionPair*>  m_pairs;

	Quadtree* m_tree;

public:
	void AddQuadPhysicsEntity(QuadEntity* quadEntity);
	void AddDiscPhysicsEntity(DiscEntity* discEntity);
	//void AddCapsulePhysicsEntity(CapsuleEntity* capsuleEntity);
	void AddRectanglePhysicsEntity(RectangleEntity* rectangleEntity);

	void ApplyGravity(Entity* entity);

	void FinalizeTransformDiscEntity();
	void FinalizeTransformQuadEntity();
	void FinalizeTransformCapsuleEntity();
	void FinalizeTransformRectangleEntity();

	void UpdateScene(float deltaTime);
	void UpdateQuadPhysicsEntities(float deltaTime);
	void UpdateDiscPhysicsEntities(float deltaTime);
	void UpdateCapsulePhysicsEntities(float deltaTime);
	void UpdateRectanglePhysicsEntities(float deltaTime);
	void UpdateDelete();
	void UpdateInput();

	void RenderScene(Renderer* renderer);
	void RenderQuadPhysicsEntities(Renderer* renderer);
	void RenderDiscPhysicsEntities(Renderer* renderer);
	void RenderCapsulePhysicsEntities(Renderer* renderer);
	void RenderRectanglePhysicsEntities(Renderer* renderer);

	// impulse resolution
	//void ResolveCollisionPhysicsDiscVsConstCapsule(DiscEntity& e1, const CapsuleEntity& e2, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsConstDisc(DiscEntity& e1, const DiscEntity& e2, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsConstQuad(DiscEntity& disc, const QuadEntity& quad, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsConstRec(DiscEntity& disc, const RectangleEntity& rec, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsDisc(DiscEntity& e1, DiscEntity& e2, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsQuad(DiscEntity& disc, QuadEntity& quad, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsRec(DiscEntity& disc, RectangleEntity& rec, Vector2 normal);
	void ResolveCollisionPhysicsDiscVsRecAngular(DiscEntity& disc, RectangleEntity& rec, Vector2 normal, const Vector2& start, const Vector2& end);
	void ResolveCollisionPhysicsDiscVsConstRecAngular(DiscEntity& disc, const RectangleEntity& rec, Vector2 normal, const Vector2& start, const Vector2& end);
	void ResolveCollisionPhysicsDiscVsConstDiscAngular(DiscEntity& disc1, const DiscEntity& disc2, Vector2 normal, const Vector2& start, const Vector2& end);
	void ResolveCollisionPhysicsDiscVsDiscAngular(DiscEntity& disc1, DiscEntity& disc2, Vector2 normal, const Vector2& start, const Vector2& end);

	void ResolveCollisionPhysicsQuadVsConstQuad(QuadEntity& quad1, const QuadEntity& quad2, Vector2 normal);
	void ResolveCollisionPhysicsQuadVsQuad(QuadEntity& quad1, QuadEntity& quad2, Vector2 normal);
	void ResolveCollisionPhysicsQuadVsConstDisc(QuadEntity& quad, const DiscEntity& disc, Vector2 normal);

	void ResolveCollisionPhysicsRecVsConstDisc(RectangleEntity& rec, const DiscEntity& disc, Vector2 normal);
	void ResolveCollisionPhysicsRecVsConstRec(RectangleEntity& rec1, const RectangleEntity& rec2, Vector2 normal);
	void ResolveCollisionPhysicsRecVsRec(RectangleEntity& rec1, RectangleEntity& rec2, Vector2 normal);
	void ResolveCollisionPhysicsRecVsRecAngular(RectangleEntity& rec1, RectangleEntity& rec2, Vector2 normal,
		const Vector2& start, const Vector2& end);
	void ResolveCollisionPhysicsRecVsConstRecAngular(RectangleEntity& rec1, const RectangleEntity& rec2, Vector2 normal,
		const Vector2& start, const Vector2& end);
	void ResolveCollisionPhysicsRecVsConstDiscAngular(RectangleEntity& rec, const DiscEntity& disc, 
		Vector2 normal, const Vector2& start, const Vector2& end);

	// corrective
	void ProcessCorrectivePhysics();
	void ProcessDiscsCorrective();
	void PhysicsDiscVsPhysicsDiscCorrective(DiscEntity& disc1, DiscEntity& disc2);
	void PhysicsDiscVsConstDiscCorrective(DiscEntity& moveDisc, const DiscEntity& constantDisc);
	void PhysicsDiscVsPhysicsQuadCorrective(DiscEntity& disc, QuadEntity& quad);
	void PhysicsDiscVsConstQuadCorrective(DiscEntity& disc, const QuadEntity& quad);
	//void PhysicsDiscVsConstCapsuleCorrective(DiscEntity& moveDisc, const CapsuleEntity& constantCapsule);
	void PhysicsDiscVsPhysicsRecCorrective(DiscEntity& disc, RectangleEntity& rec);
	void PhysicsDiscVsConstRecCorrective(DiscEntity& disc, const RectangleEntity& rec);

	void ProcessQuadsCorrective();
	void PhysicsQuadVsConstQuadCorrective(QuadEntity& quad1, const QuadEntity& quad2);
	void PhysicsQuadVsPhysicsQuadCorrective(QuadEntity& quad1, QuadEntity& quad2);
	void PhysicsQuadVsConstDiscCorrective(QuadEntity& quad, const DiscEntity& disc);
	//void PhysicsQuadVsConstCapsuleCorrective(QuadEntity& quad, const CapsuleEntity& capsule);

	void ProcessRectanglesCorrective();
	void PhysicsRecVsConstRecCorrective(RectangleEntity& rec1, const RectangleEntity& rec2);
	void PhysicsRecVsPhysicsRecCorrective(RectangleEntity& rec1, RectangleEntity& rec2);
	void PhysicsRecVsConstDiscCorrective(RectangleEntity& rec, const DiscEntity& disc);
	//void PhysicsRecVsConstCapsuleCorrective(RectangleEntity& rec, const CapsuleEntity& capsule);

	// preventative
	void ProcessPreventativePhysics(float deltaTime);
	void PhysicsDiscVsPhysicsDiscPreventative(DiscEntity& disc1, DiscEntity& disc2, float deltaTime);
	void PhysicsDiscVsConstDiscPreventative(DiscEntity& moveDisc, const DiscEntity& constantDisc, float deltaTime);
	//void PhysicsDiscVsConstCapsulePreventative(DiscEntity& moveDisc, const CapsuleEntity& constantCapsule, float deltaTime);

	// continuous
	void ProcessContinuousPhysics(float deltaTime);
	void ProcessQuadsContinuous(float deltaTime);
	void PhysicsQuadVsConstQuadContinuous(QuadEntity& quad1, const QuadEntity& quad2, float deltaTime);
	void PhysicsQuadVsPhysicsQuadContinuous(QuadEntity& quad1, QuadEntity& quad2, float deltaTime);

	// corrective collision pair process
	void ProcessPairsCorrective();

	// reflection
	void ReflectDiscVsConstDisc(DiscEntity& moveDisc, const DiscEntity& constDisc);
	void ReflectDiscVsDisc(DiscEntity& disc1, DiscEntity& disc2);
	//void ReflectDiscVsConstCapsule(DiscEntity& moveDisc, const CapsuleEntity& constCapsule, eCapsuleRegion region);

	PhysicsScene();
	~PhysicsScene();

	// SAP
	void SortAxisList(eSAPAxis axis);
	void ProcessActiveList(eSAPAxis axis);
	void ProcessSAP();

	// Quadtree
	void ProcessQuadtree();
};