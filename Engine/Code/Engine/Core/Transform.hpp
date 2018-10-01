#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix44.hpp"

#define MAT_LOCAL_IDENTITY Matrix44::IDENTITY

struct sTransform
{
	sTransform()
		: m_position(Vector3::ZERO)
		, m_euler(Vector3::ZERO)
		, m_scale(Vector3::ONE) {}

	sTransform(Vector3 pos, Vector3 euler, Vector3 scale)
		: m_position(pos), m_euler(euler), m_scale(scale)
	{
		
	}

	Vector3  m_position; 
	Vector3  m_euler; 
	Vector3  m_scale; 

	void		SetPosition( Vector3 pos ); 
	void		SetScale( Vector3 s ); 
	void		SetRotationEuler( Vector3 euler ); 

	void		Translate( Vector3 offset ); 
	void		Rotate( Vector3 euler ); 
	
	Vector3		GetEulerAngles() const {return m_euler;}
	Vector3		GetPosition() const {return m_position;}
	Vector3		GetScale() const {return m_scale;}

	Matrix44	GetMatrixEulerTranspose() const;

	// basis transform
	// scale
	Matrix44 ToWorld() const;
	Matrix44 ToLocalOrthogonal() const;	
	Matrix44 ToLocalGeneral() const;	

	// non-scale
	Matrix44 ToWorldNonScale() const;
	Matrix44 ToLocalOrtho() const;  
	Matrix44 ToLocalGen() const;	
};

class Transform 
{
public:
	Transform();
	Transform(Vector3 pos, Vector3 euler, Vector3 scale);
	~Transform();

	// local mat
	Matrix44	GetLocalMatrix() const; 
	Matrix44    GetLocalRotationMatrix() const;
	Vector3		GetLocalPosition() const { return m_localTransform.m_position; }
	Vector3		GetLocalRotation() const {return m_localTransform.m_euler;}
	Vector3		GetLocalScale() const {return m_localTransform.m_scale;}
	Vector3		GetLocalForward() const;
	Vector3		GetLocalUp() const;
	Vector3		GetLocalRight() const;

	void		SetLocalScale( Vector3 s ); 
	void		SetLocalRotation( Vector3 euler ); 
	void		SetLocalPosition( Vector3 pos ); 

	Matrix44	GetWorldMatrix() const;
	Vector3		GetWorldPosition() const;
	Vector3		GetWorldScale() const;
	Vector3		GetWorldForward() const;
	Vector3		GetWorldUp() const;
	Vector3		GetWorldRight() const;

	void		TranslateLocal( Vector3 offset ); 
	void		RotateLocal( Vector3 euler ); 

	// world mat
	void		SetParentTransform(Transform* parentTransform);

	Matrix44    GetTRMatrix() const;
	Matrix44	GetLocalMatrixEulerTranspose() const;
	Matrix44    GetWorldMatrixEulerTranspose() const;

	// scale
	static Vector3 TransformLocalToWorldPos(Vector3 local, Transform transform);
	static Vector3 TransformWorldToLocalPosOrthogonal(const Vector3& world,
		const Transform& transform);	// assumes orthogonal bases
	static Vector3 TransformWorldToLocalPosGeneral(const Vector3& world,
		const Transform& transform);	// no assumption on bases

	// non-scale
	static Vector3 LocalToWorldPos(Vector3 local, Transform transform);
	static Vector3 WorldToLocalOrthogonal(const Vector3& world, 
		const Transform& transform);	
	static Vector3 WorldToLocalGeneral(const Vector3& world,
		const Transform& transform);	
	static void TransformRotationAtoBCoord(Matrix44& rotation, const Transform& A, const Transform& B);

	// dir
	static Vector3 TransformDirToWorld(Vector3 dir_local, Transform transform);
	static Vector3 TransformDirToLocal(Vector3 dir_world, Transform transform);

public:
	sTransform m_localTransform; 
	Transform* m_parentTransform = nullptr;
};