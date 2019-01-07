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