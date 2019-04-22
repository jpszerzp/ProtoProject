#pragma once

//#include "Engine/Physics/3D/PHYSX/PhysXProcessor.hpp"

#include "Engine/Math/QuickHull.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Physics/3D/ContactResolver.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Physics/3D/SphereRB3.hpp"
#include "Engine/Physics/3D/CCDResolver.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"
#include "Engine/Physics/3D/RF/CollisionSolver.hpp"
#include "Engine/Physics/3D/PHYSX/PhysAllocator.hpp"
#include "Engine/Physics/3D/PHYSX/PhysErrorCallback.hpp"
#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"
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

#pragma comment(lib, "PhysX_64.lib")
#pragma comment(lib, "PhysXCommon_64.lib")
#pragma comment(lib, "PhysXCooking_64.lib")
#pragma comment(lib, "PhysXFoundation_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")
#pragma comment(lib, "PhysXPvdSDK_static_64.lib")

#include <set>
#include <list>

enum eFeatureCornerCase
{
	FCC_FF,
	FCC_FP,
	FCC_PP,
	FCC_EE,
	FCC_PE,
	FCC_FE,
	FCC_NUM,
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
	void UpdateForceRegistries(float dt);
	void UpdateGameobjects(float deltaTime);
	void UpdateDebug(float deltaTime);
	void UpdateDebugDraw(float deltaTime);
	void UpdateWrapArounds();
	void UpdateUI();
	void UpdateDeletePhysx();
	void UpdateDelete();
	void MyPhysicsUpdate(float deltaTime);

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
	CollisionSphere* WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g,
		const Vector3& position, const Vector3& rot, const Vector3& scale, 
		const std::string& fp = "default", const std::string& tx = "Data/Images/perspective_test.png");
	CollisionBox* WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	CollisionConvexObject* WrapAroundTestConvex(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	CollisionPoint* WrapAroundTestPoint(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	Spring* SetupSpring(CollisionPoint* end1, CollisionPoint* end2, float coef, float rl);

	void SpawnStack(const Vector3& origin, uint sideLength, uint stackHeight);
	void SpawnRandomBox(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void SpawnRandomSphere(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void SpawnRandomConvex(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void ShootSphere(WrapAround* wpa);
	void ShootBox(WrapAround* wpa);

	// Physx
	void PhysxStartup();
	void PhysxShutdown(bool interactive);
	void PhysxUpdate(bool interactive, float deltaTime);
	void InitPhysxScene(bool interactive);
	void SpawnPhysxStack(const Vector3& origin, uint sideLength, uint stackHeight);
	void PhysxRender(Renderer* renderer);
	PhysXObject* SpawnPhysxBox(const Vector3& pos);

	void ResetCollisionCornerCase(const Vector3& pos1, const Vector3& pos2, const Vector3& rot1, const Vector3& rot2);
	std::pair<PhysXObject*, PhysXObject*> ResetCollisionCornerCasePhysX(const Vector3& pos1, const Vector3& pos2, const Vector3& rot1, const Vector3& rot2);
	void SetupMyCornerCase();

public:
	const static uint MAX_CONTACT_NUM = 256;

	// ui
	bool m_debug_ui = false;
	Mesh* m_time_ui;
	Mesh* m_motion_ui;
	Mesh* m_vel_ui;
	Mesh* m_mass_ui;
	std::vector<Mesh*> m_tensor_ui;
	Mesh* m_pos_ui;
	Mesh* m_orient_ui;
	Mesh* m_base_acc_ui;
	Mesh* m_net_acc_ui;
	Mesh* m_net_force_ui;
	Mesh* m_net_torque_ui;
	Mesh* m_lin_damp_ui;
	Mesh* m_ang_damp_ui;
	Mesh* m_slow_ui;
	std::vector<Mesh*> m_world_inv_tensor_ui;

	// storages
	std::vector<CollisionSphere*> m_spheres;
	std::vector<CollisionBox*> m_boxes;
	std::vector<CollisionPlane*> m_planes;
	std::vector<CollisionConvexObject*> m_convex_objs;
	std::vector<CollisionPoint*> m_points;

	// springs
	Spring* m_spring;

	// force
	ParticleForceRegistry* m_particleRegistry = nullptr;
	RigidForceRegistry* m_rigidRegistry = nullptr;

	// debug
	CollisionPrimitive* m_focus;

	// collisions
	Collision m_storage[MAX_CONTACT_NUM];
	CollisionKeep m_keep;
	CollisionSolver m_solver;

	WrapAround* m_wraparound_plane;
	WrapAround* m_wraparound_demo_0;
	WrapAround* m_wraparound_demo_1;
	WrapAround* m_wraparound_verlet;
	WrapAround* m_wraparound_ccd;

	// corner case demo
	CollisionBox* m_corner_case_1 = nullptr;
	CollisionBox* m_corner_case_2 = nullptr;
	PhysXObject* m_corner_case_3 = nullptr;
	PhysXObject* m_corner_case_4 = nullptr;

	bool m_ff_test = true;
	bool m_fp_test = false;
	bool m_pp_test = false;
	bool m_ee_test = false;
	bool m_pe_test = false;
	bool m_fe_test = false;
	eFeatureCornerCase m_phys_corner_case = FCC_NUM;

	const Vector3 CORNER_CASE_POS_FF_1 = Vector3(25.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FP_1 = Vector3(25.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PP_1 = Vector3(25.f, 305.f, -5.f);
	const Vector3 CORNER_CASE_POS_EE_1 = Vector3(25.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PE_1 = Vector3(25.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FE_1 = Vector3(25.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FF_2 = Vector3(35.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FP_2 = Vector3(35.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PP_2 = Vector3(35.f, 315.f, 5.f);
	const Vector3 CORNER_CASE_POS_EE_2 = Vector3(35.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PE_2 = Vector3(35.f, 305.9625f, 0.f);
	const Vector3 CORNER_CASE_POS_FE_2 = Vector3(35.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FF_3 = Vector3(65.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FP_3 = Vector3(65.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FE_3 = Vector3(65.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PP_3 = Vector3(65.f, 305.f, -5.f);
	const Vector3 CORNER_CASE_POS_EE_3 = Vector3(65.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PE_3 = Vector3(65.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FF_4 = Vector3(75.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FP_4 = Vector3(75.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_FE_4 = Vector3(75.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PP_4 = Vector3(75.f, 315.f, 5.f);
	const Vector3 CORNER_CASE_POS_EE_4 = Vector3(75.f, 305.f, 0.f);
	const Vector3 CORNER_CASE_POS_PE_4 = Vector3(75.f, 305.9625f, 0.f);

	const Vector3 CORNER_CASE_ORIENT_FF_1 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_FP_1 = Vector3(45.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_PP_1 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_EE_1 = Vector3(45.f, 90.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_PE_1 = Vector3(0.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_FE_1 = Vector3(0.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_FF_2 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_FP_2 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_PP_2 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_EE_2 = Vector3(0.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_PE_2 = Vector3(0.f);
	const Vector3 CORNER_CASE_ORIENT_FE_2 = Vector3(0.f);
	const Vector3 CORNER_CASE_ORIENT_FF_3 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_FP_3 = Vector3(45.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_FE_3 = Vector3(0.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_PP_3 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_EE_3 = Vector3(45.f, 90.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_PE_3 = Vector3(0.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_FF_4 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_FP_4 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_FE_4 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_PP_4 = Vector3::ZERO;
	const Vector3 CORNER_CASE_ORIENT_EE_4 = Vector3(0.f, 45.f, 0.f);
	const Vector3 CORNER_CASE_ORIENT_PE_4 = Vector3::ZERO;

	const Vector3 CORNER_CASE_LIN_VEL_FF_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_1 = Vector3(1.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FF_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_2 = Vector3(-1.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FF_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_3 = Vector3(1.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FF_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_4 = Vector3(-1.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_4 = Vector3(-3.f, 0.f, 0.f);

	const static Vector3 ORIGIN_CAMERA;

	// physx
	PxFoundation* m_foundation = nullptr;
	PxPvd* m_pvd = nullptr;
	PxPhysics* m_physics = nullptr;
	PxDefaultCpuDispatcher* m_physx_dispatcher = NULL;
	PxScene* m_physx_scene = NULL;
	PxMaterial* m_physx_mat = NULL;
	std::vector<PhysXObject*> m_physx_objs;

	// stacks 
	std::vector<PhysXObject*> m_physx_stack;
	std::vector<CollisionBox*> m_my_stack;

	// utility
	std::vector<Vector3> m_inspection;
	int m_insepction_count = 0;

	// continuous
	Vector3 m_discrete_ball_pos;
	Vector3 m_ccd_ball_pos;
	CollisionSphere* m_discrete_ball;
	CollisionSphere* m_ccd_ball;
	CollisionPlane*  m_omni_plane;
};