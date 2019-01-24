#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const Matrix44 Matrix44::IDENTITY = Matrix44();

Matrix44::Matrix44()
{
	Ix = 1.f; 	Jx = 0.f;	Kx = 0.f;	Tx = 0.f;
	Iy = 0.f;	Jy = 1.f;	Ky = 0.f;	Ty = 0.f;
	Iz = 0.f;	Jz = 0.f;	Kz = 1.f;	Tz = 0.f;
	Iw = 0.f;	Jw = 0.f;	Kw = 0.f;	Tw = 1.f;
}


Matrix44::Matrix44(const float* entries)
{
	Ix = entries[0]; 	Jx = entries[4];	Kx = entries[8];	Tx = entries[12];
	Iy = entries[1];	Jy = entries[5];	Ky = entries[9];	Ty = entries[13];
	Iz = entries[2];	Jz = entries[6];	Kz = entries[10];	Tz = entries[14];
	Iw = entries[3];	Jw = entries[7];	Kw = entries[11];	Tw = entries[15];
}


Matrix44::Matrix44(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation)
{
	Ix = iBasis.x; 	Jx = jBasis.x;	Kx = 0.f;	Tx = translation.x;
	Iy = iBasis.y;	Jy = jBasis.y;	Ky = 0.f;	Ty = translation.y;
	Iz = 0.f;		Jz = 0.f;		Kz = 1.f;	Tz = 0.f;
	Iw = 0.f;		Jw = 0.f;		Kw = 0.f;	Tw = 1.f;
}


Matrix44::Matrix44( const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& t )
{
	Ix = i.x; Jx = j.x; Kx = k.x; Tx = t.x;
	Iy = i.y; Jy = j.y; Ky = k.y; Ty = t.y;
	Iz = i.z; Jz = j.z; Kz = k.z; Tz = t.z;
	Iw = i.w; Jw = j.w; Kw = k.w; Tw = t.w;
}


Matrix44::~Matrix44()
{

}


Matrix44 Matrix44::FromBasis(const Vector3& right, const Vector3& up, const Vector3& forward)
{
	Matrix44 res;

	res.Ix = right.x;
	res.Iy = right.y;
	res.Iz = right.z;

	res.Jx = up.x;
	res.Jy = up.y;
	res.Jz = up.z;

	res.Kx = forward.x;
	res.Ky = forward.y;
	res.Kz = forward.z;

	return res;
}


Matrix44 Matrix44::FromBasisTranslation(const Vector3& right, const Vector3& up, const Vector3& forward, const Vector3& translation)
{
	Matrix44 res;

	res.Ix = right.x;
	res.Iy = right.y;
	res.Iz = right.z;

	res.Jx = up.x;
	res.Jy = up.y;
	res.Jz = up.z;

	res.Kx = forward.x;
	res.Ky = forward.y;
	res.Kz = forward.z;

	res.Tx = translation.x;
	res.Ty = translation.y;
	res.Tz = translation.z;

	return res;
}


Vector2 Matrix44::TransformPosition2D(const Vector2& position2D)
{
	float transformedX = Ix * position2D.x + Jx * position2D.y + Tx * 1.f;
	float transformedY = Iy * position2D.x + Jy * position2D.y + Ty * 1.f;
	return Vector2(transformedX, transformedY);
}


Vector2 Matrix44::TransformDisplacement2D(const Vector2& displacement2D)
{
	float transformedX = Ix * displacement2D.x + Jx * displacement2D.y + Tx * 0.f;
	float transformedY = Iy * displacement2D.x + Jy * displacement2D.y + Ty * 0.f;
	return Vector2(transformedX, transformedY);
}


Vector3 Matrix44::TransformDisplacement3D(const Vector3& displacement3D)
{
	float t_x = Ix * displacement3D.x + Jx * displacement3D.y + Kx * displacement3D.z;
	float t_y = Iy * displacement3D.x + Jy * displacement3D.y + Ky * displacement3D.z;
	float t_z = Iz * displacement3D.x + Jz * displacement3D.y + Kz * displacement3D.z;
	return Vector3(t_x, t_y, t_z);
}

Vector3 Matrix44::TransformDisplacementInverse3D(const Vector3& displacement3D)
{
	// Transform the given direction vector by the transformational inverse of this matrix.
	// assuming orthogonal: using transpose for inverse
	float t_x = displacement3D.x * Ix + 
		displacement3D.y * Iy + displacement3D.z * Iz;
	float t_y = displacement3D.x * Jx +
		displacement3D.y * Jy + displacement3D.z * Jz;
	float t_z = displacement3D.x + Kx +
		displacement3D.y * Ky + displacement3D.z * Kz;
	return Vector3(t_x, t_y, t_z);
}

void Matrix44::SetValues(float entries[16])
{
	Ix = entries[0]; 	Jx = entries[4];	Kx = entries[8];	Tx = entries[12];
	Iy = entries[1];	Jy = entries[5];	Ky = entries[9];	Ty = entries[13];
	Iz = entries[2];	Jz = entries[6];	Kz = entries[10];	Tz = entries[14];
	Iw = entries[3];	Jw = entries[7];	Kw = entries[11];	Tw = entries[15];
}


Matrix44 Matrix44::MakeRotationDegrees2D(const float deg)
{
	Matrix44 res;

	res.Ix = CosDegrees(deg);
	res.Iy = SinDegrees(deg);
	res.Jx = -SinDegrees(deg);
	res.Jy = CosDegrees(deg);

	return res;
}


Matrix44 Matrix44::MakeRotationDegrees3D(const Vector3& euler)
{
	Matrix44 res = IDENTITY;

	// yaw pitch roll - column major
	res.Ix = CosDegrees(euler.y) * CosDegrees(euler.z) + SinDegrees(euler.y) * SinDegrees(euler.x) * SinDegrees(euler.z);
	res.Iy = -CosDegrees(euler.y) * SinDegrees(euler.z) + SinDegrees(euler.y) * SinDegrees(euler.x) * CosDegrees(euler.z);
	res.Iz = SinDegrees(euler.y) * CosDegrees(euler.x);

	res.Jx = SinDegrees(euler.z) * CosDegrees(euler.x);
	res.Jy = CosDegrees(euler.z) * CosDegrees(euler.x);
	res.Jz = -SinDegrees(euler.x);

	res.Kx = -SinDegrees(euler.y) * CosDegrees(euler.z) + CosDegrees(euler.y) * SinDegrees(euler.x) * SinDegrees(euler.z);
	res.Ky = SinDegrees(euler.z) * SinDegrees(euler.y) + CosDegrees(euler.y) * SinDegrees(euler.x) * CosDegrees(euler.z);
	res.Kz = CosDegrees(euler.y) * CosDegrees(euler.x);

	return res;
}


Matrix44 Matrix44::MakeRotationDegrees3DTranspose(Vector3 euler)
{
	Matrix44 res = IDENTITY;

	res.Ix = CosDegrees(euler.y) * CosDegrees(euler.z) + SinDegrees(euler.y) * SinDegrees(euler.x) * SinDegrees(euler.z);
	res.Iy = SinDegrees(euler.z) * CosDegrees(euler.x);
	res.Iz = -SinDegrees(euler.y) * CosDegrees(euler.z) + CosDegrees(euler.y) * SinDegrees(euler.x) * SinDegrees(euler.z);

	res.Jx = -CosDegrees(euler.y) * SinDegrees(euler.z) + SinDegrees(euler.y) * SinDegrees(euler.x) * CosDegrees(euler.z);
	res.Jy = CosDegrees(euler.z) * CosDegrees(euler.x);
	res.Jz = SinDegrees(euler.z) * SinDegrees(euler.y) + CosDegrees(euler.y) * SinDegrees(euler.x) * CosDegrees(euler.z);

	res.Kx = SinDegrees(euler.y) * CosDegrees(euler.x);
	res.Ky = -SinDegrees(euler.x);
	res.Kz = CosDegrees(euler.y) * CosDegrees(euler.x);

	return res;
}


/*
bool CloseEnough(const float& a, const float& b, const float& epsilon = std::numeric_limits<float>::epsilon()) 
{
	return (epsilon > std::abs(a - b));
}


Vector3 EulerAngles(const Matrix33& R)
{
	//check for gimbal lock
	if (CloseEnough(R.entries[0][2], -1.0f)) {
		float x = 0.f; //gimbal lock, value of x doesn't matter
		float y = ConvertRadiansToDegrees( PI / 2.f );
		float z = x + ConvertRadiansToDegrees(atan2(R.entries[1][0], R.entries[2][0]));
		return Vector3(x, y, z);
	} 
	else if (CloseEnough(R.entries[0][2], 1.0f))
	{
		float x = 0.f;
		float y = ConvertRadiansToDegrees(-PI / 2.f);
		float z = -x + ConvertRadiansToDegrees(atan2(-R.entries[1][0], -R.entries[2][0]));
		return Vector3(x, y, z);
	}
	else
	{
		//two solutions exist
		float x1 = -ConvertRadiansToDegrees(asin(R.entries[0][2]));
		float x2 = 180.f - x1;

		float y1 = ConvertRadiansToDegrees(atan2(R.entries[1][2] / cos(x1), R.entries[2][2] / cos(x1)));
		float y2 = ConvertRadiansToDegrees(atan2(R.entries[1][2] / cos(x2), R.entries[2][2] / cos(x2)));

		float z1 = ConvertRadiansToDegrees(atan2(R.entries[0][1] / cos(x1), R.entries[0][0] / cos(x1)));
		float z2 = ConvertRadiansToDegrees(atan2(R.entries[0][1] / cos(x2), R.entries[0][0] / cos(x2)));

		//choose one solution to return
		//for example the "shortest" rotation
		if ((std::abs(x1) + std::abs(y1) + std::abs(z1)) <= (std::abs(x2) + std::abs(y2) + std::abs(z2)))
		{
			return Vector3(x1, y1, z1);
		} 
		else
		{
			return Vector3(x2, y2, z2);
		}
	}
}
*/


Vector3 Matrix44::DecomposeMatrixIntoEuler(Matrix44 mat)
{
	float head;
	float pitch;
	float bank;

	float sinePitch = -mat.Jz;
	if (sinePitch <= -1.f)
	{
		pitch = ConvertRadiansToDegrees(-PI / 2.f);		// -pi/2
	}
	else if (sinePitch >= 1.f)
	{
		pitch = ConvertRadiansToDegrees(PI / 2.f);		// pi/2
	}
	else
	{
		pitch = ConvertRadiansToDegrees(asin(sinePitch));
	}

	// gimbal lock

	if (sinePitch > 0.9999f)
	{
		// looking straight up or down
		// set bank to 0 and then set head
		bank = 0.f;
		head = ConvertRadiansToDegrees(atan2(-mat.Kx, mat.Ix));
	}
	else
	{
		// compute bank and head normally 
		head = ConvertRadiansToDegrees(atan2(mat.Iz, mat.Kz));
		bank = ConvertRadiansToDegrees(atan2(mat.Jx, mat.Jy));
	}

	Vector3 euler = Vector3(pitch, head, bank);
	return euler;
}

Matrix44 Matrix44::MakeTranslation2D(const Vector2& translation)
{
	Matrix44 res;

	res.Tx = translation.x;
	res.Ty = translation.y;

	return res;
}


Matrix44 Matrix44::MakeTranslation3D(const Vector3& translation)
{
	Matrix44 res;
	res.Tx = translation.x;
	res.Ty = translation.y;
	res.Tz = translation.z;
	return res;
}


Matrix44 Matrix44::MakeScaleUniform2D(const float xyScale)
{
	Matrix44 res;

	res.Ix *= xyScale;
	res.Jy *= xyScale;

	return res;
}


Matrix44 Matrix44::MakeScale2D(const float xScale, const float yScale)
{
	Matrix44 res;

	res.Ix *= xScale;
	res.Jy *= yScale;

	return res;
}


Matrix44 Matrix44::MakeScaleUniform3D(const float xyzScale)
{
	return MakeScale3D(xyzScale, xyzScale, xyzScale);
}


Matrix44 Matrix44::MakeScale3D(const float xScale, const float yScale, const float zScale)
{
	Matrix44 res;
	res.Ix *= xScale;
	res.Jy *= yScale;
	res.Kz *= zScale;
	return res;
}


void Matrix44::RotateDegrees2D(const float deg)
{
	this->Append(MakeRotationDegrees2D(deg));
}


void Matrix44::Translate2D(const Vector2& translation)
{
	this->Append(MakeTranslation2D(translation));
}


void Matrix44::ScaleUniform2D(const float xyScale)
{
	this->Append(MakeScaleUniform2D(xyScale));
}


void Matrix44::Scale2D(const float xScale, const float yScale)
{
	this->Append(MakeScale2D(xScale, yScale));
}


void Matrix44::Scale3D(const float xScale, const float yScale, const float zScale)
{
	this->Append(MakeScale3D(xScale, yScale, zScale));
}


void Matrix44::Append(const Matrix44& mat)
{
	float t_Ix = Ix;	float t_Jx = Jx;	float t_Kx = Kx;	float t_Tx = Tx;
	float t_Iy = Iy;	float t_Jy = Jy;	float t_Ky = Ky;	float t_Ty = Ty;
	float t_Iz = Iz;	float t_Jz = Jz;	float t_Kz = Kz;	float t_Tz = Tz;
	float t_Iw = Iw;	float t_Jw = Jw;	float t_Kw = Kw;	float t_Tw = Tw;

	Ix = t_Ix * mat.Ix + t_Jx * mat.Iy + t_Kx * mat.Iz + t_Tx * mat.Iw;		Jx = t_Ix * mat.Jx + t_Jx * mat.Jy + t_Kx * mat.Jz + t_Tx * mat.Jw;		Kx = t_Ix * mat.Kx + t_Jx * mat.Ky + t_Kx * mat.Kz + t_Tx * mat.Kw;		Tx = t_Ix * mat.Tx + t_Jx * mat.Ty + t_Kx * mat.Tz + t_Tx * mat.Tw;
	Iy = t_Iy * mat.Ix + t_Jy * mat.Iy + t_Ky * mat.Iz + t_Ty * mat.Iw;		Jy = t_Iy * mat.Jx + t_Jy * mat.Jy + t_Ky * mat.Jz + t_Ty * mat.Jw;		Ky = t_Iy * mat.Kx + t_Jy * mat.Ky + t_Ky * mat.Kz + t_Ty * mat.Kw;		Ty = t_Iy * mat.Tx + t_Jy * mat.Ty + t_Ky * mat.Tz + t_Ty * mat.Tw;
	Iz = t_Iz * mat.Ix + t_Jz * mat.Iy + t_Kz * mat.Iz + t_Tz * mat.Iw;		Jz = t_Iz * mat.Jx + t_Jz * mat.Jy + t_Kz * mat.Jz + t_Tz * mat.Jw;		Kz = t_Iz * mat.Kx + t_Jz * mat.Ky + t_Kz * mat.Kz + t_Tz * mat.Kw;		Tz = t_Iz * mat.Tx + t_Jz * mat.Ty + t_Kz * mat.Tz + t_Tz * mat.Tw;
	Iw = t_Iw * mat.Ix + t_Jw * mat.Iy + t_Kw * mat.Iz + t_Tw * mat.Iw;		Jw = t_Iw * mat.Jx + t_Jw * mat.Jy + t_Kw * mat.Jz + t_Tw * mat.Jw;		Kw = t_Iw * mat.Kx + t_Jw * mat.Ky + t_Kw * mat.Kz + t_Tw * mat.Kw;		Tw = t_Iw * mat.Tx + t_Jw * mat.Ty + t_Kw * mat.Tz + t_Tw * mat.Tw;
}


void Matrix44::SetIdentity()
{
	Ix = 1.f; 	Jx = 0.f;	Kx = 0.f;	Tx = 0.f;
	Iy = 0.f;	Jy = 1.f;	Ky = 0.f;	Ty = 0.f;
	Iz = 0.f;	Jz = 0.f;	Kz = 1.f;	Tz = 0.f;
	Iw = 0.f;	Jw = 0.f;	Kw = 0.f;	Tw = 1.f;
}


Matrix44 Matrix44::MakeOrtho2D(const Vector2& bl, const Vector2& tr)
{
	Matrix44 res;

	res.Ix = 2.f / (tr.x - bl.x);
	res.Jy = 2.f / (tr.y - bl.y);
	res.Kz = -2.f / (1.f - 0.f);
	res.Tx = -(tr.x + bl.x) / (tr.x - bl.x);
	res.Ty = -(tr.y + bl.y) / (tr.y - bl.y);
	res.Tz = -(1.f + 0.f) / (1.f - 0.f);

	return res;
}


Matrix44 Matrix44::MakeOrtho3D(const Vector2& bl, const Vector2& tr, float theFar, float theNear)
{
	Matrix44 res = Matrix44();

	// opengl
	res.Ix = 2.f / (tr.x - bl.x);
	res.Jy = 2.f / (tr.y - bl.y);
	res.Kz = -2.f / (theFar - theNear);
	res.Tx = -(tr.x + bl.x) / (tr.x - bl.x);
	res.Ty = -(tr.y + bl.y) / (tr.y - bl.y);
	res.Tz = -(theFar + theNear) / (theFar - theNear);

	return res;
}


//Matrix44 Matrix44::LookAt(Vector3 position, Vector3 target, Vector3 up)
//{
//	Vector3 forward = (target - position).GetNormalized(); 
//	Vector3 right = up.Cross(forward);
//	Vector3 newUp = forward.Cross(right); 
//
//	Matrix44 model = Matrix44::FromBasisTranslation( right.GetNormalized(), newUp.GetNormalized(), forward, position ); 
//	//Matrix44 tranlationMatrix = Matrix44::MakeTranslation3D(position);
//
//	/*
//	Matrix44 model = IDENTITY;
//	model.Append(tranlationMatrix);
//	model.Append(rotationMatrix);
//	*/
//
//	return model;
//}


Matrix44 Matrix44::LookAtLocal(Vector3 position, Vector3 target, Vector3 up)
{
	Vector3 forward = target - position;
	forward.NormalizeAndGetLength();   

	Vector3 right = up.Cross(forward);
	right.NormalizeAndGetLength();

	Vector3 upDir = forward.Cross(right); 

	Matrix44 matrix;

	float translationX = -right.x * position.x - right.y * position.y - right.z * position.z;
	float translationY = -upDir.x * position.x - upDir.y * position.y - upDir.z * position.z;
	float translationZ = -forward.x * position.x - forward.y * position.y - forward.z * position.z;
	/*
	float entries[16] = { right.x, upDir.x, forward.x, 0.f, 
		right.y, upDir.y, forward.y, 0.f, 
		right.z, upDir.z, forward.z, 0.f, 
		translationX, translationY, translationZ, 1.f
	};
	*/

	float entries[16] = { right.x, right.y, right.z, 0.f, 
		upDir.x, upDir.y, upDir.z, 0.f, 
		forward.x, forward.y, forward.z, 0.f, 
		translationX, translationY, translationZ, 1.f
	};

	matrix.SetValues(entries);
	return matrix;
}

Matrix44 Matrix44::operator*(const Matrix44& n) const
{
	Matrix44 res = Matrix44();

	float newIx = Ix * n.Ix + Jx * n.Iy + Kx * n.Iz + Tx * n.Iw;
	float newIy = Iy * n.Ix + Jy * n.Iy + Ky * n.Iz + Ty * n.Iw;
	float newIz = Iz * n.Ix + Jz * n.Iy + Kz * n.Iz + Tz * n.Iw;
	float newIw = Iw * n.Ix + Jw * n.Iy + Kw * n.Iz + Tw * n.Iw;

	float newJx = Ix * n.Jx + Jx * n.Jy + Kx * n.Jz + Tx * n.Jw;
	float newJy = Iy * n.Jx + Jy * n.Jy + Ky * n.Jz + Ty * n.Jw;
	float newJz = Iz * n.Jx + Jz * n.Jy + Kz * n.Jz + Tz * n.Jw;
	float newJw = Iw * n.Jx + Jw * n.Jy + Kw * n.Jz + Tw * n.Jw;

	float newKx = Ix * n.Kx + Jx * n.Ky + Kx * n.Kz + Tx * n.Kw;
	float newKy = Iy * n.Kx + Jy * n.Ky + Ky * n.Kz + Ty * n.Kw;
	float newKz = Iz * n.Kx + Jz * n.Ky + Kz * n.Kz + Tz * n.Kw;
	float newKw = Iw * n.Kx + Jw * n.Ky + Kw * n.Kz + Tw * n.Kw;

	float newTx = Ix * n.Tx + Jx * n.Ty + Kx * n.Tz + Tx * n.Tw;
	float newTy = Iy * n.Tx + Jy * n.Ty + Ky * n.Tz + Ty * n.Tw;
	float newTz = Iz * n.Tx + Jz * n.Ty + Kz * n.Tz + Tz * n.Tw;
	float newTw = Iw * n.Tx + Jw * n.Ty + Kw * n.Tz + Tw * n.Tw;

	float entries[16] = {newIx, newIy, newIz, newIw, newJx, newJy, newJz, newJw, newKx, newKy, newKz, newKw, newTx, newTy, newTz, newTw};

	res.SetValues(entries);
	return res;
}


Vector4 Matrix44::operator*(const Vector4& rhs) const
{
	Vector4 row1 = Vector4(Ix, Jx, Kx, Tx);
	Vector4 row2 = Vector4(Iy, Jy, Ky, Ty);
	Vector4 row3 = Vector4(Iz, Jz, Kz, Tz);
	Vector4 row4 = Vector4(Iw, Jw, Kw, Tw);

	float x = DotProduct(row1, rhs);
	float y = DotProduct(row2, rhs);
	float z = DotProduct(row3, rhs);
	float w = DotProduct(row4, rhs);

	return Vector4(x, y, z, w);
}


Vector3 Matrix44::operator*(const Vector3& rhs) const
{
	Vector3 row1 = Vector3(Ix, Jx, Kx);
	Vector3 row2 = Vector3(Iy, Jy, Ky);
	Vector3 row3 = Vector3(Iz, Jz, Kz);

	float x = DotProduct(row1, rhs);
	float y = DotProduct(row2, rhs);
	float z = DotProduct(row3, rhs);

	// also considers translation
	x += Tx;
	y += Ty;
	z += Tz;

	return Vector3(x, y, z);
}

Matrix44 Matrix44::Invert() const
{
	float inv[16];
	float det;
	float m[16];
	float data[16];

	memcpy(m, this, sizeof(Matrix44));

	inv[0] = 
		m[5]  * m[10] * m[15] - 
		m[5]  * m[11] * m[14] - 
		m[9]  * m[6]  * m[15] + 
		m[9]  * m[7]  * m[14] +
		m[13] * m[6]  * m[11] - 
		m[13] * m[7]  * m[10];

	inv[4] = 
		-m[4]  * m[10] * m[15] + 
		m[4]  * m[11] * m[14] + 
		m[8]  * m[6]  * m[15] - 
		m[8]  * m[7]  * m[14] - 
		m[12] * m[6]  * m[11] + 
		m[12] * m[7]  * m[10];

	inv[8] = 
		m[4]  * m[9] * m[15] - 
		m[4]  * m[11] * m[13] - 
		m[8]  * m[5] * m[15] + 
		m[8]  * m[7] * m[13] + 
		m[12] * m[5] * m[11] - 
		m[12] * m[7] * m[9];

	inv[12] = 
		-m[4]  * m[9] * m[14] + 
		m[4]  * m[10] * m[13] +
		m[8]  * m[5] * m[14] - 
		m[8]  * m[6] * m[13] - 
		m[12] * m[5] * m[10] + 
		m[12] * m[6] * m[9];

	inv[1] = 
		-m[1]  * m[10] * m[15] + 
		m[1]  * m[11] * m[14] + 
		m[9]  * m[2] * m[15] - 
		m[9]  * m[3] * m[14] - 
		m[13] * m[2] * m[11] + 
		m[13] * m[3] * m[10];

	inv[5] = 
		m[0]  * m[10] * m[15] - 
		m[0]  * m[11] * m[14] - 
		m[8]  * m[2] * m[15] + 
		m[8]  * m[3] * m[14] + 
		m[12] * m[2] * m[11] - 
		m[12] * m[3] * m[10];

	inv[9] = 
		-m[0]  * m[9] * m[15] + 
		m[0]  * m[11] * m[13] + 
		m[8]  * m[1] * m[15] - 
		m[8]  * m[3] * m[13] - 
		m[12] * m[1] * m[11] + 
		m[12] * m[3] * m[9];

	inv[13] = 
		m[0]  * m[9] * m[14] - 
		m[0]  * m[10] * m[13] - 
		m[8]  * m[1] * m[14] + 
		m[8]  * m[2] * m[13] + 
		m[12] * m[1] * m[10] - 
		m[12] * m[2] * m[9];

	inv[2] = 
		m[1]  * m[6] * m[15] - 
		m[1]  * m[7] * m[14] - 
		m[5]  * m[2] * m[15] + 
		m[5]  * m[3] * m[14] + 
		m[13] * m[2] * m[7] - 
		m[13] * m[3] * m[6];

	inv[6] = 
		-m[0]  * m[6] * m[15] + 
		m[0]  * m[7] * m[14] + 
		m[4]  * m[2] * m[15] - 
		m[4]  * m[3] * m[14] - 
		m[12] * m[2] * m[7] + 
		m[12] * m[3] * m[6];

	inv[10] = 
		m[0]  * m[5] * m[15] - 
		m[0]  * m[7] * m[13] - 
		m[4]  * m[1] * m[15] + 
		m[4]  * m[3] * m[13] + 
		m[12] * m[1] * m[7] - 
		m[12] * m[3] * m[5];

	inv[14] = 
		-m[0]  * m[5] * m[14] + 
		m[0]  * m[6] * m[13] + 
		m[4]  * m[1] * m[14] - 
		m[4]  * m[2] * m[13] - 
		m[12] * m[1] * m[6] + 
		m[12] * m[2] * m[5];

	inv[3] = 
		-m[1] * m[6] * m[11] + 
		m[1] * m[7] * m[10] + 
		m[5] * m[2] * m[11] - 
		m[5] * m[3] * m[10] - 
		m[9] * m[2] * m[7] + 
		m[9] * m[3] * m[6];

	inv[7] = 
		m[0] * m[6] * m[11] - 
		m[0] * m[7] * m[10] - 
		m[4] * m[2] * m[11] + 
		m[4] * m[3] * m[10] + 
		m[8] * m[2] * m[7] - 
		m[8] * m[3] * m[6];

	inv[11] = 
		-m[0] * m[5] * m[11] + 
		m[0] * m[7] * m[9] + 
		m[4] * m[1] * m[11] - 
		m[4] * m[3] * m[9] - 
		m[8] * m[1] * m[7] + 
		m[8] * m[3] * m[5];

	inv[15] = 
		m[0] * m[5] * m[10] - 
		m[0] * m[6] * m[9] - 
		m[4] * m[1] * m[10] + 
		m[4] * m[2] * m[9] + 
		m[8] * m[1] * m[6] - 
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	det = 1.f / det;

	for (uint i = 0; i < 16; i++) {
		data[i] = (float)(inv[i] * det);
	}

	Matrix44 res;
	res.SetValues(data);

	return res;
}


bool Matrix44::Invert(const float m[16], float invOut[16])
{
	float inv[16], det;
	int i;

	inv[0] = m[5]  * m[10] * m[15] - 
		m[5]  * m[11] * m[14] - 
		m[9]  * m[6]  * m[15] + 
		m[9]  * m[7]  * m[14] +
		m[13] * m[6]  * m[11] - 
		m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] + 
		m[4]  * m[11] * m[14] + 
		m[8]  * m[6]  * m[15] - 
		m[8]  * m[7]  * m[14] - 
		m[12] * m[6]  * m[11] + 
		m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9] * m[15] - 
		m[4]  * m[11] * m[13] - 
		m[8]  * m[5] * m[15] + 
		m[8]  * m[7] * m[13] + 
		m[12] * m[5] * m[11] - 
		m[12] * m[7] * m[9];

	inv[12] = -m[4]  * m[9] * m[14] + 
		m[4]  * m[10] * m[13] +
		m[8]  * m[5] * m[14] - 
		m[8]  * m[6] * m[13] - 
		m[12] * m[5] * m[10] + 
		m[12] * m[6] * m[9];

	inv[1] = -m[1]  * m[10] * m[15] + 
		m[1]  * m[11] * m[14] + 
		m[9]  * m[2] * m[15] - 
		m[9]  * m[3] * m[14] - 
		m[13] * m[2] * m[11] + 
		m[13] * m[3] * m[10];

	inv[5] = m[0]  * m[10] * m[15] - 
		m[0]  * m[11] * m[14] - 
		m[8]  * m[2] * m[15] + 
		m[8]  * m[3] * m[14] + 
		m[12] * m[2] * m[11] - 
		m[12] * m[3] * m[10];

	inv[9] = -m[0]  * m[9] * m[15] + 
		m[0]  * m[11] * m[13] + 
		m[8]  * m[1] * m[15] - 
		m[8]  * m[3] * m[13] - 
		m[12] * m[1] * m[11] + 
		m[12] * m[3] * m[9];

	inv[13] = m[0]  * m[9] * m[14] - 
		m[0]  * m[10] * m[13] - 
		m[8]  * m[1] * m[14] + 
		m[8]  * m[2] * m[13] + 
		m[12] * m[1] * m[10] - 
		m[12] * m[2] * m[9];

	inv[2] = m[1]  * m[6] * m[15] - 
		m[1]  * m[7] * m[14] - 
		m[5]  * m[2] * m[15] + 
		m[5]  * m[3] * m[14] + 
		m[13] * m[2] * m[7] - 
		m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] + 
		m[0]  * m[7] * m[14] + 
		m[4]  * m[2] * m[15] - 
		m[4]  * m[3] * m[14] - 
		m[12] * m[2] * m[7] + 
		m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] - 
		m[0]  * m[7] * m[13] - 
		m[4]  * m[1] * m[15] + 
		m[4]  * m[3] * m[13] + 
		m[12] * m[1] * m[7] - 
		m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] + 
		m[0]  * m[6] * m[13] + 
		m[4]  * m[1] * m[14] - 
		m[4]  * m[2] * m[13] - 
		m[12] * m[1] * m[6] + 
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] + 
		m[1] * m[7] * m[10] + 
		m[5] * m[2] * m[11] - 
		m[5] * m[3] * m[10] - 
		m[9] * m[2] * m[7] + 
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] - 
		m[0] * m[7] * m[10] - 
		m[4] * m[2] * m[11] + 
		m[4] * m[3] * m[10] + 
		m[8] * m[2] * m[7] - 
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] + 
		m[0] * m[7] * m[9] + 
		m[4] * m[1] * m[11] - 
		m[4] * m[3] * m[9] - 
		m[8] * m[1] * m[7] + 
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] - 
		m[0] * m[6] * m[9] - 
		m[4] * m[1] * m[10] + 
		m[4] * m[2] * m[9] + 
		m[8] * m[1] * m[6] - 
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
	{
		return false;
	}

	det = 1.f / det;

	for (i = 0; i < 16; i++)
	{
		invOut[i] = inv[i] * det;
	}

	return true;
}


Vector3 Matrix44::MultiplyInverse(const Vector3& vector) const
{
	Vector3 tmp = vector;
	tmp.x -= Tx;
	tmp.y -= Ty;
	tmp.z -= Tz;

	float x = tmp.x * Ix + tmp.y * Iy + tmp.z * Iz;
	float y = tmp.x * Jx + tmp.y * Jy + tmp.z * Jz;
	float z = tmp.x * Kx + tmp.y * Ky + tmp.z * Kz;

	return Vector3(x, y, z);
}

Matrix44 Matrix44::Transpose()
{
	Vector4 newI = Vector4(Ix, Jx, Kx, Tx);
	Vector4 newJ = Vector4(Iy, Jy, Ky, Ty);
	Vector4 newK = Vector4(Iz, Jz, Kz, Tz);
	Vector4 newT = Vector4(Iw, Jw, Kw, Tw);
	return Matrix44(newI, newJ, newK, newT);
}

void Matrix44::Print()
{
	DebuggerPrintf("Ix: %f, Iy: %f, Iz: %f, Iw: %f"
				   "Jx: %f, Jy: %f, Jz: %f, Jw: %f"
				   "Kx: %f, Ky: %f, Kz: %f, Kw: %f"
				   "Tx: %f, Ty: %f, Tz: %f, Tw: %f\n",
					Ix, Iy, Iz, Iw,
					Jx, Jy, Jz, Jw,
					Kx, Ky, Kz, Kw,
					Tx, Ty, Tz, Tw);
}


Vector3 Matrix44::GetForward() const
{
	Vector3 forward = Vector3(Kx, Ky, Kz);
	return forward;
}


Vector3 Matrix44::GetUp() const
{
	Vector3 up = Vector3(Jx, Jy, Jz);
	return up;
}


Vector3 Matrix44::GetRight() const
{
	Vector3 right = Vector3(Ix, Iy, Iz);
	return right;
}


Vector3 Matrix44::GetTranslation() const
{
	Vector3 translation = Vector3(Tx, Ty, Tz);
	return translation;
}


//const float* Matrix44::GetValues() const
//{
//	const float m[16] = {
//		Ix, Iy, Iz, Iw, Jx, Jy, Jz, Jw, Kx, Ky, Kz, Kw, Tx, Ty, Tz, Tw
//	};
//
//	return m;
//}

Matrix33 Matrix44::ExtractMat3() const
{
	Vector3 i = Vector3(Ix, Iy, Iz);
	Vector3 j = Vector3(Jx, Jy, Jz);
	Vector3 k = Vector3(Kx, Ky, Kz);

	return Matrix33(i, j, k);
}


Matrix44 Matrix44::RotateToward(const Matrix44&, float) const
{
	TODO("Implement this");
	return Matrix44::IDENTITY;
}