#pragma once

#include "Engine/Math/QuickHull.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Physics/3D/ContactResolver.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Physics/3D/CCDResolver.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"
#include "Engine/Physics/3D/RF/CollisionSolver.hpp"
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
	void PostConstruct();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override;

	// pipeline of update
	void UpdateInput(float deltaTime);
	void UpdateGameobjects(float deltaTime);
	void UpdateDebug(float deltaTime);
	void UpdateDebugDraw(float deltaTime);
	void UpdateWrapArounds();
	void UpdateUI();

	// update of GO
	void UpdateGameobjectsCore(float deltaTime);
	void UpdateGameobjectsDynamics(float deltaTime);
	void UpdateContacts(float deltaTime);
	void UpdateContactGeneration();
	void UpdateContactResolution(float deltaTime);

	void Render(Renderer* renderer) override;
	void RenderGameobjects(Renderer* renderer);
	void RenderWrapArounds(Renderer* renderer);
	void RenderForwardPath(Renderer* renderer);
	void RenderUI(Renderer* renderer);

	// scene tests
	CollisionSphere* WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale);
	CollisionBox* WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	CollisionConvexObject* WrapAroundTestConvex(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);

	void SpawnStack(const Vector3& origin, uint sideLength, uint stackHeight);
	void SpawnRandomBox(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void SpawnRandomSphere(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void SpawnRandomConvex(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void ShootSphere(WrapAround* wpa);
	void ShootBox(WrapAround* wpa);

public:
	const static uint MAX_CONTACT_NUM = 256;

	// ui
	Mesh* m_time_ui;
	Mesh* m_motion_ui;
	Mesh* m_vel_ui;
	Mesh* m_mass_ui;
	std::vector<Mesh*> m_tensor_ui;

	// refactor
	std::vector<CollisionSphere*> m_spheres;
	std::vector<CollisionBox*> m_boxes;
	std::vector<CollisionPlane*> m_planes;
	std::vector<CollisionConvexObject*> m_convex_objs;

	// debug
	CollisionPrimitive* m_focus;

	Collision m_storage[MAX_CONTACT_NUM];

	CollisionKeep m_keep;

	CollisionSolver m_solver;

	//WrapAround* m_wraparound_sphere;
	//WrapAround* m_wraparound_box;
	//WrapAround* m_wraparound_convex;
	WrapAround* m_wraparound_plane;
};