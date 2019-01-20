#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Transform.hpp"
#include "Engine/Core/Quaternion.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
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
	bool pass = rotationRes == rotationRes_true;
	ASSERT_OR_DIE(pass, "rotation: quaternion rotation of vector based on axis is wrong!");
	DebuggerPrintf("rotation: quaternion rotation of vector based on axis is correct\n");

	//// from euler to quaternion
	//Vector3 euler = Vector3(50.f);
	//Quaternion q = Quaternion::FromEuler(euler);
	//Quaternion q_correct = Quaternion(.669f, .509f, .185f, .509f);
	//pass = q == q_correct;
	//std::string err_msg = Stringf("q converted is: %f, %f, %f, %f; correct q is %f, %f, %f, %f",
	//	q.m_real, q.m_imaginary.x, q.m_imaginary.y, q.m_imaginary.z,
	//	q_correct.m_real, q_correct.m_imaginary.x, q_correct.m_imaginary.y, q_correct.m_imaginary.z);
	//ASSERT_RECOVERABLE(pass, err_msg);
	//DebuggerPrintf("quaternion from euler passed\n");
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

	// from euler to matrix
	Vector3 euler = Vector3(50.f);
	mat3 = Matrix33::FromEuler(euler);

	Vector3 i_correct = Vector3(0.8627092, 0.4924039, -0.1152006);
	Vector3 j_correct = Vector3(-0.1152006, 0.4131759, 0.9033352);
	Vector3 k_correct = Vector3(0.4924039, -0.7660444, 0.4131759);
	Matrix33 mat3_correct = Matrix33(i_correct, j_correct, k_correct);

	bool pass = IsCloseEnoughMat3(mat3, mat3_correct);

	ASSERT_OR_DIE(pass, "euler to rotation matrix3 fails");
	DebuggerPrintf("euler to rotation matrix3 succeeds\n");

	// from matrix to quaternion
	Quaternion q = Quaternion::FromMatrix(mat3);
	Quaternion q_correct = Quaternion(0.8199178 , Vector3(0.5090082, 0.1852638, 0.1852638));

	pass = IsCloseEnoughQuaternion(q, q_correct);

	ASSERT_OR_DIE(pass, "matrix to quaternion fails");
	DebuggerPrintf("matrix to quaternion succeeds\n");

	euler = Vector3(34.f, 5.f, 23.f);
	mat3 = Matrix33::FromEuler(euler);

	i_correct = Vector3(0.9360451, 0.3239308, 0.1374353);
	j_correct = Vector3(-0.3443817, 0.7631331, 0.5468355);
	k_correct = Vector3(0.0722554, -0.5591929, 0.8258829);
	mat3_correct = Matrix33(i_correct, j_correct, k_correct);

	pass = IsCloseEnoughMat3(mat3, mat3_correct);

	ASSERT_OR_DIE(pass, "euler to rotation matrix3 fails");
	DebuggerPrintf("euler to rotation matrix3 succeeds\n");
}

bool IsCloseEnoughFloats(const float& f1, const float& f2)
{
	return (abs(f1 - f2) < 0.01f);
}

bool IsCloseEnoughVec3(const Vector3& v1, const Vector3& v2)
{
	return IsCloseEnoughFloats(v1.x, v2.x) && IsCloseEnoughFloats(v1.y, v2.y) && IsCloseEnoughFloats(v1.z, v2.z);
}

bool IsCloseEnoughMat3(const Matrix33& m1, const Matrix33& m2)
{
	return IsCloseEnoughVec3(m1.GetI(), m2.GetI()) && IsCloseEnoughVec3(m1.GetJ(), m2.GetJ()) && IsCloseEnoughVec3(m1.GetK(), m2.GetK());
}

bool IsCloseEnoughQuaternion(const Quaternion& q1, const Quaternion& q2)
{
	return IsCloseEnoughFloats(q1.m_real, q2.m_real) && IsCloseEnoughVec3(q1.m_imaginary, q2.m_imaginary);
}
