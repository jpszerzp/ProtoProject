#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix33.hpp"
#include "Engine/Math/Primitive3.hpp"
#include "Engine/Core/Transform.hpp"

#include <vector>

enum eContactFeature
{
	V1, V2, V3, V4, V5, V6, V7, V8,
	F1, F2, F3, F4, F5, F6,
	E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12,
	FEATURE_NO_CARE		// The no care is for box-plane, the feature of plane specifically, cuz plane has "no" feature
};

class Entity3;

struct OBB3Vert
{
	Vector3 m_vert;
	eContactFeature m_feature;		// could only be Vs 

	OBB3Vert(){}
	OBB3Vert(const Vector3& vert, eContactFeature feature) : m_vert(vert), m_feature(feature) {}
};

struct OBB3Edge
{
	OBB3Vert m_end1;
	OBB3Vert m_end2;
	eContactFeature m_feature;

	OBB3Edge(){}
	OBB3Edge(const OBB3Vert& v1, const OBB3Vert& v2, eContactFeature feature) : m_end1(v1), m_end2(v2), m_feature(feature) {}

	const bool operator<(const OBB3Edge& compared) const;

	Vector3 ToVec3() const;
};

struct OBB3Face
{
	Vector3 m_normal;
	Vector3 m_center;
	eContactFeature m_feature;

	OBB3Face(){}
	OBB3Face(const Vector3& normal, const Vector3& center, eContactFeature feature) : m_normal(normal), m_center(center), m_feature(feature) {}
};

class OBB3
{
private:
	Entity3* m_entity = nullptr;

public:
	Vector3 m_center;			// OBB center point
	Vector3 m_forward;
	Vector3 m_up;
	Vector3 m_right;
	Vector3 m_halfExt;			// halfway extends of OBB along basis 
	std::vector<OBB3Face> m_faces;
	std::vector<OBB3Edge> m_edges;
	std::vector<OBB3Vert> m_verts;

public:
	OBB3(){}
	OBB3(Vector3 center, Vector3 forward, Vector3 up, Vector3 right, Vector3 halfExt);
	~OBB3(){}

	float GetDiagonalHalf() const { return m_halfExt.GetLength(); }
	float GetDiagonalHalfSquared() const { return m_halfExt.GetLengthSquared(); }
	float GetFullExtX() const { return m_halfExt.x * 2.f; }
	float GetFullExtY() const { return m_halfExt.y * 2.f; }
	float GetFullExtZ() const { return m_halfExt.z * 2.f; }
	Vector3 GetHalfExt() const { return m_halfExt; }
	Vector3 GetHalfExtCopy() const { return m_halfExt; }
	Vector3 GetHalfExtX() const { return m_right * m_halfExt.x; }
	Vector3 GetHalfExtY() const { return m_up * m_halfExt.y; }
	Vector3 GetHalfExtZ() const { return m_forward * m_halfExt.z; }
	Vector3 GetCenter() const { return m_center; }
	OBB3Vert GetFTLVert() const { return m_verts[0]; }
	OBB3Vert GetFBLVert() const { return m_verts[1]; }
	OBB3Vert GetFBRVert() const { return m_verts[2]; }
	OBB3Vert GetFTRVert() const { return m_verts[3]; }
	OBB3Vert GetBTLVert() const { return m_verts[4]; }
	OBB3Vert GetBBLVert() const { return m_verts[5]; }
	OBB3Vert GetBBRVert() const { return m_verts[6]; }
	OBB3Vert GetBTRVert() const { return m_verts[7]; }
	Vector3 GetFTL() const { return m_verts[0].m_vert; }
	Vector3 GetFBL() const { return m_verts[1].m_vert; }
	Vector3 GetFBR() const { return m_verts[2].m_vert; }
	Vector3 GetFTR() const { return m_verts[3].m_vert; }
	Vector3 GetBTL() const { return m_verts[4].m_vert; }
	Vector3 GetBBL() const { return m_verts[5].m_vert; }
	Vector3 GetBBR() const { return m_verts[6].m_vert; }
	Vector3 GetBTR() const { return m_verts[7].m_vert; }
	Vector3 GetFTLExt() const;
	Vector3 GetFBLExt() const;
	Vector3 GetFBRExt() const;
	Vector3 GetFTRExt() const;
	Vector3 GetBTLExt() const;
	Vector3 GetBBLExt() const;
	Vector3 GetBBRExt() const;
	Vector3 GetBTRExt() const;
	float	GetFTLExtAlong(const Vector3& along) const;
	float	GetFBLExtAlong(const Vector3& along) const;
	float	GetFBRExtAlong(const Vector3& along) const;
	float	GetFTRExtAlong(const Vector3& along) const;
	float	GetBTLExtAlong(const Vector3& along) const;
	float	GetBBLExtAlong(const Vector3& along) const;
	float	GetBBRExtAlong(const Vector3& along) const;
	float	GetBTRExtAlong(const Vector3& along) const; 
	Entity3* GetEntity() const { return m_entity; }
	Vector3 GetForward() const { return m_forward; }
	Vector3 GetUp() const { return m_up; }
	Vector3 GetRight() const { return m_right; }
	std::vector<Vector3> GetVertices() const;
	std::vector<Vector3> GetBase() const;

	void SetCenter(const Vector3& center);
	void SetEntity(Entity3* ent) { m_entity = ent; }
	void SetForward(const Vector3& forward) { m_forward = forward; }
	void SetUp(const Vector3& up) { m_up = up; }
	void SetRight(const Vector3& right) { m_right = right; }

	void UpdateFace();
	void UpdateVertAndEdge();
};