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
	bool operator==(const Quaternion& rhs);

	float GetNorm() const;
	float GetNormSqr() const;
	void Normalize();
	void ConvertToUnitNormQuaternion();
	void RotateByVector(const Vector3& vec);
	void AddScaledVector(const Vector3& vec, float scale);

	Quaternion GetConjugate() const;
	Quaternion GetInverse() const;

	// from and to rotation matrix (note the subtlety between mat33 and mat44 here)
	// we can use extract function to retrieve a mat33 from mat44
	static Matrix44 GetMatrixRotation(const Quaternion& q);
	static Matrix44 GetMatrixWithPosition(const Quaternion& q, const Vector3& pos);
	static Quaternion FromMatrix(const Matrix33& rot);

	static Quaternion FromEuler(const Vector3& euler);
	Vector3 ToEuler() const;
};