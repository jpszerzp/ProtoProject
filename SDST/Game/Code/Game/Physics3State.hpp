#pragma once

#include "Engine/Math/QuickHull.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Physics/3D/ContactResolver.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Physics/3D/BVH3.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Physics/3D/CCDResolver.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Cube.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Core/Primitive/Box.hpp"
#include "Game/GameState.hpp"
#include "Game/Ballistics.hpp"
#include "Game/Fireworks.hpp"
#include "Game/Spring.hpp"
#include "Game/Links.hpp"
#include "Game/WrapAround3.hpp"

#include <set>
#include <list>

enum eHullGenerationStep
{
	HULL_GEN_FORM_CONFLICTS,
	HULL_GEN_FORM_EYE,
	HULL_GEN_FORM_HORIZON_START,
	HULL_GEN_FORM_HORIZON_PROECSS,
	HULL_GEN_DELETE_OLD_FACES,
	HULL_GEN_FORM_NEW_FACES,
	HULL_GEN_ASSIGN_ORPHANS,
	HULL_GEN_TOPO_ERRORS,
	HULL_GEN_FINISH_RESET,
	HULL_GEN_COMPLETE
};

class Physics3State : public GameState
{
public:
	Physics3State();
	~Physics3State();

	Sphere* InitializePhysSphere(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme scheme = DISCRETE, bool bp = true);
	Cube* InitializePhysCube(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);
	Point* InitializePhysPoint(Vector3 pos, Vector3 rot, float size, 
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);
	Quad* InitializePhysQuad(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid, eDynamicScheme scheme = DISCRETE);
	Box* InitializePhysBox(Vector3 pos, Vector3 rot, Vector3 scale,
		Rgba tint, eMoveStatus moveStat, eBodyIdentity bid);

	Fireworks* SetupFireworks(float age, Vector3 pos, Vector3 inheritVel, Vector3 maxVel, Vector3 minVel, bool lastRound = false);
	Ballistics* SetupBallistics(eBallisticsType type, Vector3 pos, bool frozen, Rgba color);
	Spring* SetupSpring(Point* end1, Point* end2, float coef, float rl);
	AnchorSpring* SetupAnchorSpring(Point* end1, Point* end2, float coef, float rl);
	Rod* SetupRod(float length, Point* p1, Point* p2);

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override;

	// pipeline of update
	void UpdateInput(float deltaTime);
	void UpdateGameobjects(float deltaTime);
	void UpdateDebugDraw(float deltaTime);
	void RespawnFireworks();

	// update of GO
	void UpdateForceRegistry(float deltaTime);
	void UpdateGameobjectsCore(float deltaTime);
	void UpdateContactGeneration();
	void UpdateCore();
	void UpdateContactResolution(float deltaTime);
	void UpdateBVH();

	void Render(Renderer* renderer) override;
	void RenderGameobjects(Renderer* renderer);
	void RenderBVH(Renderer* renderer);
	void RenderModelSamples(Renderer* renderer);

	// scene tests
	void WrapAroundTestGeneral();

	void SwapHullStatusMesh(const std::string& str);

public:
	GravityRigidForceGenerator* m_gravity;

	Spring* m_spring;
	AnchorSpring* m_anchorSpring;
	Rod* m_rod;
	GeneralRigidAnchorSpring* m_rigidAnchorSpring;

	ParticleForceRegistry* m_particleRegistry = nullptr;
	RigidForceRegistry* m_rigidRegistry = nullptr;

	// entities
	std::vector<Sphere*> m_spheres;
	std::vector<Cube*>	 m_cubes;
	std::vector<Quad*>	 m_quads;
	std::vector<Point*>  m_points;
	std::vector<Fireworks*>  m_fw_points;
	std::vector<Box*>	 m_boxes;
	std::vector<GameObject*> m_rigid_gos;		// for convenience of BVH

	// continuous convenience
	Sphere* m_ball_ccd_test_discrete = nullptr;
	Sphere* m_ball_ccd_test_continuous = nullptr;
	Quad* m_quad_ccd_test = nullptr;
	std::vector<Vector3> m_inspection;
	int m_insepction_count = 0;
	// we do need some structure to observe continuous objects conveniently
	std::vector<Sphere*> m_ccd_spheres;
	std::vector<Quad*> m_ccd_planes;

	ContactResolver* m_allResolver;		
	ContactResolver* m_iterResolver;
	ContactResolver* m_coherentResolver;
	
	// BVH
	bool m_broadPhase = false;
	std::vector<BVHContact> m_bvhContacts;
	BVHNode<BoundingSphere>* m_bvh_node;
	int m_nodeCount = 0;			// temp: later will flush all GO into tree at once
	Mesh* m_bp_title = nullptr;
	Mesh* m_bp_status = nullptr;

	// QH
	QuickHull* m_qh = nullptr;
	eHullGenerationStep m_genStep = HULL_GEN_FORM_CONFLICTS;
	Mesh* m_hull_title = nullptr;
	Mesh* m_hull_status = nullptr;
	float m_textHeight = 0.f;
	Vector2 m_titleMin = Vector2::ZERO;
	Vector2 m_statusMin = Vector2::ZERO;
	
	WrapAround* m_wraparound_continuous;
	WrapAround* m_wraparound_general;
	WrapAround* m_wraparound_verlet;
	int m_wrap_pos_it_general = 0;

	// assimp test
	AssimpLoader* m_assimp_0 = nullptr;
	std::set<Vector3> m_modelPoints;
	std::vector<Mesh*> m_modelPointMeshes;
};