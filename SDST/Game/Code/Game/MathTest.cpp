//#include "Engine/Physics/3D/PHYSX/PhysXProcessor.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Transform.hpp"
#include "Engine/Core/Quaternion.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/MathTest.hpp"

void MathTest::RunMathTest()
{
	VoidMathTest();
	QuaternionTest();
	MatrixTest();
	PlaneTest();
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

	Vector3 i_correct = Vector3(0.8627092f, 0.4924039f, -0.1152006f);
	Vector3 j_correct = Vector3(-0.1152006f, 0.4131759f, 0.9033352f);
	Vector3 k_correct = Vector3(0.4924039f, -0.7660444f, 0.4131759f);
	Matrix33 mat3_correct = Matrix33(i_correct, j_correct, k_correct);

	bool pass = IsCloseEnoughMat3(mat3, mat3_correct);

	ASSERT_OR_DIE(pass, "euler to rotation matrix3 fails");
	DebuggerPrintf("euler to rotation matrix3 succeeds\n");

	// from matrix to quaternion
	Quaternion q = Quaternion::FromMatrix(mat3);
	Quaternion q_correct = Quaternion(0.8199178f, Vector3(0.5090082f, 0.1852638f, 0.1852638f));

	pass = IsCloseEnoughQuaternion(q, q_correct);

	ASSERT_OR_DIE(pass, "matrix to quaternion fails");
	DebuggerPrintf("matrix to quaternion succeeds\n");

	euler = Vector3(34.f, 5.f, 23.f);
	mat3 = Matrix33::FromEuler(euler);

	i_correct = Vector3(0.9360451f, 0.3239308f, 0.1374353f);
	j_correct = Vector3(-0.3443817f, 0.7631331f, 0.5468355f);
	k_correct = Vector3(0.0722554f, -0.5591929f, 0.8258829f);
	mat3_correct = Matrix33(i_correct, j_correct, k_correct);

	pass = IsCloseEnoughMat3(mat3, mat3_correct);

	ASSERT_OR_DIE(pass, "euler to rotation matrix3 fails");
	DebuggerPrintf("euler to rotation matrix3 succeeds\n");
}

void MathTest::PlaneTest()
{
	Plane p1 = Plane(Vector3(0.f, 0.f, 1.f), 1.f);
	Plane p2 = Plane(Vector3(0.f, 0.f, -1.f), 1.f);
	Plane p3 = Plane(Vector3(-1.f, 0.f, 0.f), 1.f);
	Plane p4 = Plane(Vector3(1.f, 0.f, 0.f), 1.f);
	Plane p5 = Plane(Vector3(0.f, 1.f, 0.f), 1.f);
	Plane p6 = Plane(Vector3(0.f, -1.f, 0.f), 1.f);

	Vector3 intersection;
	bool i1 = ComputePlaneIntersectionPoint(p1, p2, p3, intersection);
	ASSERT_OR_DIE(i1 == false, "There is no intersection for parallel planes");
	DebuggerPrintf("There is no intersection for parallel planes\n");

	bool i2 = ComputePlaneIntersectionPoint(p2, p3, p5, intersection);
	ASSERT_OR_DIE(i2 == true, "There should be an intersection for unparallel planes");
	ASSERT_OR_DIE(IsCloseEnoughVec3(intersection, Vector3(-1.f, 1.f, -1.f)), 
		"Unmatch of intersection point");
	
	bool i3 = ComputePlaneIntersectionPoint(p6, p4, p1, intersection);
	ASSERT_OR_DIE(i3 == true, "There should be an intersection for unparallel planes");
	ASSERT_OR_DIE(IsCloseEnoughVec3(intersection, Vector3(1.f, -1.f, 1.f)),
		"Unmatch of intersection point");
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
