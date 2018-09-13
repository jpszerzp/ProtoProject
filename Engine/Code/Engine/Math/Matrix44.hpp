#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix33.hpp"

class Matrix44
{
public:
	float Ix, Iy, Iz, Iw,   Jx, Jy, Jz, Jw,   Kx, Ky, Kz, Kw,   Tx, Ty, Tz, Tw;
	static const Matrix44 IDENTITY;

public:
	Matrix44();
	explicit Matrix44( const float* entries );
	explicit Matrix44( const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation = Vector2(0.f,0.f) );
	explicit Matrix44( const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& t );
	static Matrix44 FromBasis(const Vector3& right, const Vector3& up, const Vector3& forward);
	static Matrix44 FromBasisTranslation(const Vector3& right, const Vector3& up, const Vector3& forward, const Vector3& translation);
	~Matrix44();

	// basics
	Vector2 TransformPosition2D(const Vector2& position2D);
	Vector2 TransformDisplacement2D(const Vector2& displacement2D);
	Vector3 TransformDisplacement3D(const Vector3& displacement3D);
	Vector3 TransformDisplacementInverse3D(const Vector3& displacement3D);
	void	SetValues(float entries[16]);

	// producers
	static Matrix44 MakeRotationDegrees2D(const float deg);
	static Matrix44 MakeRotationDegrees3D(Vector3 euler);
	static Matrix44 MakeRotationDegrees3DTranspose(Vector3 euler);
	static Vector3  DecomposeMatrixIntoEuler(Matrix44 mat4);

	static Matrix44 MakeTranslation2D(const Vector2& translation);
	static Matrix44 MakeTranslation3D(const Vector3& translation);

	static Matrix44 MakeScaleUniform2D(const float xyScale);
	static Matrix44 MakeScale2D(const float xScale, const float yScale);

	static Matrix44 MakeScaleUniform3D(const float xyzScale);
	static Matrix44 MakeScale3D(const float xScale, const float yScale, const float zScale);

	// append identity
	void RotateDegrees2D(const float deg);
	void Translate2D(const Vector2& translation);
	void ScaleUniform2D(const float xyScale);
	void Scale2D(const float xScale, const float yScale);
	void Scale3D(const float xScale, const float yScale, const float zScale);

	// append general
	void Append(const Matrix44& mat);

	// set identity
	void SetIdentity();

	// projection
	static Matrix44 MakeOrtho2D(const Vector2& bl, const Vector2& tr);
	static Matrix44 MakeOrtho3D(const Vector2& bl, const Vector2& tr, float theFar, float theNear);
	static Matrix44 LookAtLocal(Vector3 position, Vector3 target, Vector3 up);
	//static Matrix44 LookAt(Vector3 position, Vector3 target, Vector3 up);

	// inverse
	Matrix44 Invert();
	bool Invert(const float m[16], float invOut[16]);

	// transpose
	Matrix44 Transpose();

	void Print();

	Matrix44 operator*(const Matrix44& rhs) const;
	Vector4 operator*(const Vector4& rhs) const;

	Vector3 GetForward() const;
	Vector3 GetUp() const;
	Vector3 GetRight() const;
	Vector3 GetTranslation() const;

	const float* GetValues() const;

	Matrix33 ExtractMat3() const;

	Matrix44 RotateToward(const Matrix44& rotateTarget, float maxAngleToMove) const;
};

//bool CloseEnough(const float& a, const float& b, const float& epsilon = std::numeric_limits<float>::epsilon());
//Vector3 EulerAngles(const Matrix33& R);