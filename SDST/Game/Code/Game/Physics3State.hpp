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
	void UpdateGameobjects(float deltaTime);
	void UpdateDebug(float deltaTime);
	void UpdateDebugDraw(float deltaTime);
	void UpdateWrapArounds();
	void UpdateUI();
	void UpdateDelete();
	void UpdateForceRegistries(float dt);

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
	CollisionSphere* WrapAroundTestSphere(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, 
		bool register_g, const Vector3& position, const Vector3& rot, const Vector3& scale, float mass = 1.f, 
		const std::string& fp = "default", const std::string& tx = "Data/Images/perspective_test.png");
	CollisionBox* WrapAroundTestBox(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	CollisionConvexObject* WrapAroundTestConvex(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	CollisionPoint* WrapAroundTestPoint(WrapAround* wpa, bool give_ang_vel, bool give_lin_vel, bool register_g, const Vector3& position,
		const Vector3& rot, const Vector3& scale, const bool& awake = true, const bool& sleepable = false);
	Spring* SetupSpring(CollisionPoint* end1, CollisionPoint* end2, float coef, float rl);
	AnchorSpring* SetupAnchorSpring(CollisionPoint* end1, CollisionPoint* end2, float coef, float rl);

	void SpawnStack(const Vector3& origin, uint sideLength, uint stackHeight);
	void SpawnRandomBox(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void SpawnRandomSphere(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void SpawnRandomConvex(WrapAround* wpa, uint num, const Vector3& min, const Vector3& max);
	void ShootSphere(WrapAround* wpa);
	void ShootBox(WrapAround* wpa);

	void ResetCollisionCornerCase(const Vector3& pos1, const Vector3& pos2,
		const Vector3& rot1, const Vector3& rot2);
	std::pair<PhysXObject*, PhysXObject*> ResetCollisionCornerCasePhysX(const Vector3& pos1, 
		const Vector3& pos2, const Vector3& rot1, const Vector3& rot2);

public:
	const static uint MAX_CONTACT_NUM = 256;

	// ui
	bool m_cc_1_on = false;
	bool m_cc_2_on = false;
	float m_txt_height;
	Mesh* m_fps_text;
	Mesh* m_my_count;
	Mesh* m_slp_count;
	Mesh* m_lin_vel_1;
	Mesh* m_ang_vel_1;
	Mesh* m_lin_vel_2;
	Mesh* m_ang_vel_2;
	Mesh* m_lin_vel_3;
	Mesh* m_ang_vel_3;
	Mesh* m_lin_vel_4;
	Mesh* m_ang_vel_4;
	Mesh* m_sep_vel;
	Mesh* m_sep_cmp;
	Mesh* m_cmp_lin_vel_13_amt;
	Mesh* m_cmp_lin_vel_13_dir;
	Mesh* m_cmp_ang_vel_13_amt;
	Mesh* m_cmp_ang_vel_13_dir;
	Mesh* m_cmp_lin_vel_24_amt;
	Mesh* m_cmp_lin_vel_24_dir;
	Mesh* m_cmp_ang_vel_24_amt;
	Mesh* m_cmp_ang_vel_24_dir;

	// refactor
	std::vector<CollisionSphere*> m_spheres;
	std::vector<CollisionBox*> m_boxes;
	std::vector<CollisionPlane*> m_planes;
	std::vector<CollisionConvexObject*> m_convex_objs;
	std::vector<CollisionPoint*> m_points;

	Collision m_storage[MAX_CONTACT_NUM];
	CollisionKeep m_keep;
	CollisionSolver m_solver;

	WrapAround* m_wraparound_plane;
	WrapAround* m_wraparound_demo_0;
	WrapAround* m_wraparound_demo_1;
	WrapAround* m_wraparound_verlet;
	WrapAround* m_wraparound_ccd;

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
	const Vector3 CORNER_CASE_POS_PP_2 = Vector3(35.0005f, 315.f, 5.f);
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
	const Vector3 CORNER_CASE_LIN_VEL_PP_1 = Vector3(3.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_1 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FF_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_2 = Vector3(-3.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_2 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FF_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_3 = Vector3(3.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_3 = Vector3(3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FF_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FP_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_FE_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PP_4 = Vector3(-3.f);
	const Vector3 CORNER_CASE_LIN_VEL_EE_4 = Vector3(-3.f, 0.f, 0.f);
	const Vector3 CORNER_CASE_LIN_VEL_PE_4 = Vector3(-3.f, 0.f, 0.f);

	const static Vector3 ORIGIN_CAMERA;
	Vector3 m_corner_case_demo_pos;

	// stack
	std::vector<CollisionBox*> m_my_stack;

	CollisionBox* m_corner_case_1 = nullptr;
	CollisionBox* m_corner_case_2 = nullptr;
	std::vector<CollisionBox*> m_corner_boxes;

	// force
	ParticleForceRegistry* m_particleRegistry = nullptr;
	RigidForceRegistry* m_rigidRegistry = nullptr;

	// utility
	std::vector<Vector3> m_inspection;
	int m_insepction_count = 0;

	// springs
	Spring* m_spring;
	AnchorSpring* m_anchorSpring;
	GeneralRigidAnchorSpring* m_rigidAnchorSpring;
	CollisionPoint* m_spr_anchor;
	CollisionSphere* m_spr_attach;

	// continuous
	Vector3 m_discrete_ball_pos;
	Vector3 m_ccd_ball_pos;
	CollisionSphere* m_discrete_ball;
	CollisionSphere* m_ccd_ball;
	CollisionPlane*  m_omni_plane;
};