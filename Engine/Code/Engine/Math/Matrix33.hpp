#pragma once

#include "Engine/Math/Vector3.hpp"

class Matrix33
{
public:
	float Ix, Iy, Iz, Jx, Jy, Jz, Kx, Ky, Kz;
	static const Matrix33 IDENTITY;
	static const Matrix33 ZERO;

	Matrix33();
	~Matrix33(){}
	explicit Matrix33(float entry);
	explicit Matrix33(const float* entries);
	explicit Matrix33(const Vector3& i, const Vector3& j, const Vector3& k);

	Matrix33 operator*(const Matrix33& rhs) const;
	void operator*=(const Matrix33& rhs);
	void operator*=(const float scale);
	void operator+=(const Matrix33& rhs);
	Vector3 operator*(const Vector3& rhs) const;
	void operator*(const float rhs);
	const float operator[](const int idx) const;

	float GetDeterminant() const;
	float GetTrace() const;
	Vector3 GetI() const { return Vector3(Ix, Iy, Iz); }
	Vector3 GetJ() const { return Vector3(Jx, Jy, Jz); }
	Vector3 GetK() const { return Vector3(Kx, Ky, Kz); }

	void SetRight(Vector3 right);
	void SetUp(Vector3 up);
	void SetForward(Vector3 forward);
	void SetBasis(Vector3 right, Vector3 up, Vector3 forward);
	void SetSkewSymmetric(const Vector3& v);

	// inverse
	Matrix33 Invert() const;
	void SetInverse(const Matrix33& mat);

	// transpose
	Matrix33 Transpose() const;
	Vector3 MultiplyTranspose(const Vector3& v) const;

	static Matrix33 FromEuler(const Vector3& euler);
};