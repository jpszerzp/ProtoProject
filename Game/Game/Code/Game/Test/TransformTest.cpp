#include "Game/Test/TransformTest.hpp"
#include "Engine/Core/Transform.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void TransformTest::RunTransformTest()
{
	LocalWorldTransformTest();
	TransformBasisTest();
}

void TransformTest::LocalWorldTransformTest()
{
	Transform base_0 = Transform();
	Vector3 local_0 = Vector3::ONE;			// (1, 1, 1)
	Vector3 world_0 = Transform::TransformLocalToWorldPos(local_0, base_0);
	Vector3 world_0_true = Vector3::ONE;
	bool pass_0 = world_0 == world_0_true;
	ASSERT_OR_DIE(pass_0, "0: local to world is wrong!");
	DebuggerPrintf("0: local to world is right\n");

	Vector3 pos_1 = Vector3(10.f, 10.f, 10.f);
	Vector3 rot_1 = Vector3::ZERO;
	Vector3 scale_1 = Vector3::ONE;
	Transform base_1 = Transform(pos_1, rot_1, scale_1);
	Vector3 local_1 = Vector3::ONE;			// (11, 11, 11)
	Vector3 world_1 = Transform::TransformLocalToWorldPos(local_1, base_1);
	Vector3 world_1_true = Vector3(11.f, 11.f, 11.f);
	bool pass_1 = world_1 == world_1_true;
	ASSERT_OR_DIE(pass_1, "1: local to world is wrong!");
	DebuggerPrintf("1: local to world is right\n");

	Vector3 pos_2 = Vector3(-10.f, 5.f, -20.f);
	Vector3 rot_2 = Vector3::ZERO;
	Vector3 scale_2 = Vector3::ONE;
	Transform base_2 = Transform(pos_2, rot_2, scale_2);
	Vector3 world_2 = Vector3(-10.f, 0.f, -20.f);
	Vector3 local_2 = Transform::TransformWorldToLocalPosOrthogonal(world_2, base_2);
	Vector3 local_2_true = Vector3(0.f, -5.f, 0.f);
	bool pass_2 = local_2 == local_2_true;
	ASSERT_OR_DIE(pass_2, "2: world to local is wrong!");
	DebuggerPrintf("2: world to local is right\n");

	Vector3 pos_3= Vector3(-10.f, 5.f, -20.f);
	Vector3 rot_3 = Vector3::ZERO;
	Vector3 scale_3 = Vector3::ONE;
	Transform base_3 = Transform(pos_3, rot_3, scale_3);
	Vector3 world_3 = Vector3(-10.f, 0.f, -20.f);
	Vector3 local_3 = Transform::TransformWorldToLocalPosGeneral(world_3, base_3);
	Vector3 local_3_true = Vector3(0.f, -5.f, 0.f);
	bool pass_3 = local_3 == local_3_true;
	ASSERT_OR_DIE(pass_3, "3: world to local is wrong!");
	DebuggerPrintf("3: world to local is right\n");
}

void TransformTest::TransformBasisTest()
{
	Vector3 pos = Vector3::ZERO;
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Transform t0 = Transform(pos, rot, scale);

	Vector3 wf0 = t0.GetWorldForward();
	Vector3 wu0 = t0.GetWorldUp();
	Vector3 wr0 = t0.GetWorldRight();

	Vector3 true_wf0 = Vector3(0.f, 0.f, 1.f);
	Vector3 true_wu0 = Vector3(0.f, 1.f, 0.f);
	Vector3 true_wr0 = Vector3(1.f, 0.f, 0.f);

	ASSERT_OR_DIE(wf0 == true_wf0, "0 - world forward does not match");
	DebuggerPrintf("0 - world forward matches\n");
	ASSERT_OR_DIE(wu0 == true_wu0, "0 - world upward does not match");
	DebuggerPrintf("0 - world upward matches\n");
	ASSERT_OR_DIE(wr0 == true_wr0, "0 - wolrd right does not match");
	DebuggerPrintf("0 - world rightward matches\n");

	Vector3 lf0 = t0.GetLocalForward();
	Vector3 lu0 = t0.GetLocalUp();
	Vector3 lr0 = t0.GetLocalRight();

	Vector3 true_lf0 = Vector3(0.f, 0.f, 1.f);
	Vector3 true_lu0 = Vector3(0.f, 1.f, 0.f);
	Vector3 true_lr0 = Vector3(1.f, 0.f, 0.f);

	ASSERT_OR_DIE(lf0 == true_lf0, "0 - local forward does not match");
	DebuggerPrintf("0 - local forward matches\n");
	ASSERT_OR_DIE(lu0 == true_lu0, "0 - local upward does not match");
	DebuggerPrintf("0 - local upward matches\n");
	ASSERT_OR_DIE(lr0 == true_lr0, "0 - local right does not match");
	DebuggerPrintf("0 - local rightward matches\n");

	// have some rotation
	pos = Vector3::ZERO;
	rot = Vector3(90.f, 0.f, 0.f);
	scale = Vector3::ONE;
	Transform t1 = Transform(pos, rot, scale);

	Vector3 wf1 = t1.GetWorldForward();
	Vector3 wu1 = t1.GetWorldUp();
	Vector3 wr1 = t1.GetWorldRight();

	Vector3 true_wf1 = Vector3(0.f, 1.f, 0.f);
	Vector3 true_wu1 = Vector3(0.f, 0.f, -1.f);
	Vector3 true_wr1 = Vector3(1.f, 0.f, 0.f);

	ASSERT_OR_DIE((wf1 == true_wf1) || Vector3::AreVectorsNearlyIdentical(wf1, true_wf1), "1 - world forward does not match");
	DebuggerPrintf("1 - world forward matches\n");
	ASSERT_OR_DIE((wu1 == true_wu1) || Vector3::AreVectorsNearlyIdentical(wu1, true_wu1), "1 - world upward does not match");
	DebuggerPrintf("1 - world upward matches\n");
	ASSERT_OR_DIE((wr1 == true_wr1) || Vector3::AreVectorsNearlyIdentical(wr1, true_wr1), "1 - wolrd right does not match");
	DebuggerPrintf("1 - world rightward matches\n");

	// GetWorldMatrix used
	Matrix44 worldMat1 = t1.GetWorldMatrix();
	wf1 = worldMat1.GetForward();
	wu1 = worldMat1.GetUp();
	wr1 = worldMat1.GetRight();

	ASSERT_OR_DIE((wf1 == true_wf1) || Vector3::AreVectorsNearlyIdentical(wf1, true_wf1), "1,GetWorldMatrix - world forward does not match");
	DebuggerPrintf("1,GetWorldMatrix - world forward matches\n");
	ASSERT_OR_DIE((wu1 == true_wu1) || Vector3::AreVectorsNearlyIdentical(wu1, true_wu1), "1,GetWorldMatrix - world upward does not match");
	DebuggerPrintf("1,GetWorldMatrix - world upward matches\n");
	ASSERT_OR_DIE((wr1 == true_wr1) || Vector3::AreVectorsNearlyIdentical(wr1, true_wr1), "1,GetWorldMatrix - wolrd right does not match");
	DebuggerPrintf("1,GetWorldMatrix - world rightward matches\n");

	Vector3 lf1 = t1.GetLocalForward();
	Vector3 lu1 = t1.GetLocalUp();
	Vector3 lr1 = t1.GetLocalRight();

	Vector3 true_lf1 = Vector3(0.f, 0.f, 1.f);
	Vector3 true_lu1 = Vector3(0.f, 1.f, 0.f);
	Vector3 true_lr1 = Vector3(1.f, 0.f, 0.f);

	ASSERT_OR_DIE(lf1 == true_lf1, "1 - local forward does not match");
	DebuggerPrintf("1 - local forward matches\n");
	ASSERT_OR_DIE(lu1 == true_lu1, "1 - local upward does not match");
	DebuggerPrintf("1 - local upward matches\n");
	ASSERT_OR_DIE(lr1 == true_lr1, "1 - local right does not match");
	DebuggerPrintf("1 - local rightward matches\n");

	// GetLocalMatrix used 
	Matrix44 localMat1 = t1.GetLocalMatrix();
	lf1 = localMat1.GetForward();
	lu1 = localMat1.GetUp();
	lr1 = localMat1.GetRight();

	ASSERT_OR_DIE(lf1 == true_lf1, "1,GetLocalMatrix - local forward does not match");
	DebuggerPrintf("1,GetLocalMatrix - local forward matches\n");
	ASSERT_OR_DIE(lu1 == true_lu1, "1,GetLocalMatrix - local upward does not match");
	DebuggerPrintf("1,GetLocalMatrix - local upward matches\n");
	ASSERT_OR_DIE(lr1 == true_lr1, "1,GetLocalMatrix - local right does not match");
	DebuggerPrintf("1,GetLocalMatrix - local rightward matches\n");
}
