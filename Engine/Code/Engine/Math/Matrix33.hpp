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

	Vector3 operator*(const Vector3& rhs) const;
	void operator*(const float rhs);
	const float operator[](const int idx) const;

	float GetDeterminant() const;

	// inverse
	Matrix33 Invert() const;
	void SetInverse(const Matrix33& mat);

	// transpose
	Matrix33 Transpose() const;
};