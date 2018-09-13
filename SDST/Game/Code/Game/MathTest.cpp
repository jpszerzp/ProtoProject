#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Transform.hpp"
#include "Game/MathTest.hpp"

void MathTest::RunMathTest()
{
	VoidMathTest();
	QuaternionTest();
}

void MathTest::VoidMathTest()
{
	ASSERT_OR_DIE(true, "this test always passes");
	DebuggerPrintf("void test passes\n");
}

void MathTest::QuaternionTest()
{
	// rotation
	Vector3 rotatedVec3 = Vector3(0.f, 1.f, 0.f);
	Vector3 rotateAxis = Vector3(1.f, 0.f, 0.f);
	Vector3 rotationRes = rotatedVec3.RotateAboutAxisWithAngle(90.f, rotateAxis);
	Vector3 rotationRes_true = Vector3(0.f, 0.f, 1.f);
	bool pass_rotation = rotationRes == rotationRes_true;
	ASSERT_OR_DIE(pass_rotation, "rotation: quaternion rotation of vector based on axis is wrong!");
	DebuggerPrintf("rotation: quaternion rotation of vector based on axis is correct\n");
}

