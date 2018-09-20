#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix44.hpp"

class Quaternion
{
public:
	float m_real;
	Vector3 m_imaginary;

	const static Quaternion IDENTITY;

public:
	Quaternion();
	Quaternion(float real, float x, float y, float z);
	Quaternion(float real, Vector3 imaginary);
	~Quaternion();
	Quaternion(const Quaternion& copy);
	
	Quaternion& operator=(const Quaternion& copy);
	Quaternion operator+(const Quaternion& rhs) const;
	Quaternion operator-(const Quaternion& rhs) const;
	Quaternion operator*(const Quaternion& rhs) const;
	Quaternion operator*(const float scalar) const;
	void operator+=(const Quaternion& rhs);
	void operator-=(const Quaternion& rhs);
	void operator*=(const Quaternion& rhs);
	void operator*=(const float scalar);

	float GetNorm() const;
	void Normalize();
	void ConvertToUnitNormQuaternion();
	void RotateByVector(const Vector3& vec);
	void AddScaledVector(const Vector3& vec, float scale);

	Quaternion GetConjugate() const;
	Quaternion GetInverse() const;

	static Matrix44 GetMatrixRotation(const Quaternion& q);
	static Matrix44 GetMatrixWithPosition(const Quaternion& q, const Vector3& pos);
};