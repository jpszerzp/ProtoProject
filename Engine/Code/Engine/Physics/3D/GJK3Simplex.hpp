#pragma once

#include "Engine/Math/Vector3.hpp"

#include <set>


struct GJK3SimplexFace
{
	Vector3 m_verts[3];
	Vector3 m_normal;

	GJK3SimplexFace(){}
	GJK3SimplexFace(const Vector3& v1, const Vector3& v2, const Vector3& v3);
	~GJK3SimplexFace(){}

	bool IsPointBehindFace(const Vector3& pt) const;
};

struct GJK3Simplex
{
	GJK3Simplex(){}
	virtual ~GJK3Simplex(){}
};

struct GJK3SimplexTetra : GJK3Simplex
{
	GJK3SimplexFace m_faces[4];

	GJK3SimplexTetra(){}
	GJK3SimplexTetra(const std::set<Vector3>& verts);
	~GJK3SimplexTetra(){}

	bool IsPointInTetra(const Vector3& pt);
};

void GJK_GenerateFaceNormal(Vector3& norm1, Vector3& norm2, const GJK3SimplexFace& face);
void GJK_GenerateOutboundFaceNormal(const Vector3& external, GJK3SimplexFace& face);