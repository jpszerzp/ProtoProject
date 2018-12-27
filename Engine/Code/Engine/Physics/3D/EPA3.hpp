#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Line3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Mesh.hpp"

#include <queue> 
#include <set>

struct sEPAFace
{
	float m_toOrigin;
	Vector3 m_normal;
	Vector3 m_verts[3];
	Mesh* m_mesh;

	sEPAFace(const Vector3& v1, const Vector3& v2, const Vector3& v3);
	~sEPAFace();

	//float DistToPoint(const Vector3& pt);
	
	float ComputeAndSetDistOrigin();

	void Draw(Renderer* renderer) const;
};


struct sEPAFaceCompare 
{
	sEPAFaceCompare(){}
	bool operator() (const sEPAFace* f1, const sEPAFace* f2) const
	{
		if (f1->m_toOrigin > f2->m_toOrigin)
			return true;
		return false;
	}
};

enum eEPAStat
{
	EPA_DELETE_GJK_REF,
	EPA_CREATE_SIMPLEX,
	EPA_FIND_FACE,
	EPA_DELETE_VISIBLE,
};

/*
 * EPA simplex is different in the way that it is not limited to tetrahedron
 * unlike GJK simplex. We are ready to deal with any number of faces until reaching threshold.
 */
struct sEPASimplex
{
	std::vector<sEPAFace*> m_unordered_faces;
	std::priority_queue<sEPAFace*, std::vector<sEPAFace*>, sEPAFaceCompare> m_faces;

	sEPASimplex(){}
	sEPASimplex(const std::set<Vector3>& verts);
	~sEPASimplex();
	
	void Draw(Renderer* renderer);

	sEPAFace* SelectClosestFaceToOrigin();
};

Vector3 EPA_FindSupp(QuickHull* hull, const Line3& dir);
void EPA_GenerateFaceNormal(Vector3& norm1, Vector3& norm2, sEPAFace* face);
void EPA_GenerateOutboundFaceNormal(const Vector3& external, sEPAFace* face);

//extern eEPAStat epa_stat;
