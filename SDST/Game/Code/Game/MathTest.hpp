#pragma once

class MathTest
{
public:
	static void RunMathTest();

private:
	static void VoidMathTest();

	// quaternion
	static void QuaternionTest();

	static void MatrixTest();
};

bool IsCloseEnoughFloats(const float& f1, const float& f2);
bool IsCloseEnoughVec3(const Vector3& v1, const Vector3& v2);
bool IsCloseEnoughMat3(const Matrix33& m1, const Matrix33& m2);