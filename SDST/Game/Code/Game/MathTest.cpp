#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Transform.hpp"
#include "Game/MathTest.hpp"

void MathTest::RunMathTest()
{
	VoidMathTest();
	QuaternionTest();
	MatrixTest();
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

void MathTest::MatrixTest()
{
	// mat3 inverse
	Vector3 i = Vector3(-1.f, 4.f, 3.f);
	Vector3 j = Vector3(2.f, -9.f, -5.f);
	Vector3 k = Vector3(-6.f, 7.f, -3.f);
	Matrix33 mat3 = Matrix33(i, j, k);

	Matrix33 inv = mat3.Invert();
	const Vector3& inv_i = inv.GetI();
	const Vector3& inv_j = inv.GetJ();
	const Vector3& inv_k = inv.GetK();

	bool match_i = (inv_i == Vector3(-31.f/19.f, -33.f/38.f, -7.f/38.f));
	bool match_j = (inv_j == Vector3(-18.f/19.f, -21.f/38.f, -1.f/38.f));
	bool match_k = (inv_k == Vector3(20.f/19.f, 17.f/38.f, -1.f/38.f));

	ASSERT_OR_DIE(match_i, "inverse 33 i does not match");
	ASSERT_OR_DIE(match_j, "inverse 33 j does not match");
	ASSERT_OR_DIE(match_k, "inverse 33 k does not match");

	DebuggerPrintf("inverse matrix matches\n");
}

bool IsCloseEnoughFloats(const float& f1, const float& f2)
{
	return (abs(f1 - f2) < 0.01f);
}
