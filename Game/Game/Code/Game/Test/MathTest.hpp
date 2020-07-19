#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix33.hpp"
#include "Engine/Core/Quaternion.hpp"

class MathTest
{
public:
	static void RunMathTest();

private:
	static void QuaternionTest();
	static void MatrixTest();
	static void VectorTest();
};

bool IsCloseEnoughFloats(const float& f1, const float& f2);
bool IsCloseEnoughVec3(const Vector3& v1, const Vector3& v2);
bool IsCloseEnoughMat3(const Matrix33& m1, const Matrix33& m2);
bool IsCloseEnoughQuaternion(const Quaternion& q1, const Quaternion& q2);