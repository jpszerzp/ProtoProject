#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Line3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Mesh.hpp"

#include <queue> 
#include <set>

//struct sEPAVert
//{
//	Vector3 mksi_vert;
//
//	Vector3 a_vert;
//	Vector3 b_vert;
//
//	bool operator==(const sEPAVert& compare) const { return mksi_vert == compare.mksi_vert; }
//};

struct sEPAEdgeRef
{
	std::vector<Vector3> ends;
	int ref_count=0;

	sEPAEdgeRef(const Vector3& v1, const Vector3& v2);
	~sEPAEdgeRef(){}

	bool operator==(const sEPAEdgeRef& compare) const;
};

struct sEPAFace
{
	float m_toOrigin;
	Vector3 m_normal;
	Vector3 m_verts[3];
	//sEPAVert m_verts[3];
	Mesh* m_mesh = nullptr;
	//std::vector<sEPAEdgeRef*> m_edge_refs;

	//sEPASimplex* epa_simplex_ref = nullptr;

	sEPAFace(const Vector3& v1, const Vector3& v2, const Vector3& v3);
	//sEPAFace(const sEPAVert& v1, const sEPAVert& v2, const sEPAVert& v3);
	~sEPAFace();

	//float DistToPoint(const Vector3& pt);
	
	float ComputeAndSetDistOrigin();
	//Vector3 GetMKSIVertPos(int idx) const;

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
	EPA_FORM_NEW_FACE,
	EPA_COMPLETE,
	EPA_POST_COMPLETE,
};

/*
 * EPA simplex is different in the way that it is not limited to tetrahedron
 * unlike GJK simplex. We are ready to deal with any number of faces until reaching threshold.
 */
struct sEPASimplex
{
	std::vector<sEPAEdgeRef*> m_edge_refs;
	std::vector<sEPAFace*> m_unordered_faces;
	//std::priority_queue<sEPAFace*, std::vector<sEPAFace*>, sEPAFaceCompare> m_faces;

	sEPASimplex(){}
	//sEPASimplex(const std::set<sEPAVert>& verts);
	sEPASimplex(const std::set<Vector3>& verts);
	~sEPASimplex();
	
	void Draw(Renderer* renderer);

	sEPAFace* SelectClosestFaceToOrigin();
	bool DeleteVisibleFacesForPt(const Vector3& pt);
	bool IsFaceVisibleForPt(sEPAFace* face, const Vector3& pt);
	sEPAEdgeRef* IsEdgeExisted(sEPAEdgeRef* edge);
	sEPAEdgeRef* IsEdgeExisted(sEPAEdgeRef* edge, std::vector<sEPAEdgeRef*>& edge_set);
	void AddEdgeRef(sEPAEdgeRef* edge);
	void AddEdgeRefSafe(sEPAEdgeRef* edge, std::vector<sEPAEdgeRef*>& edge_set);
	void AddEdgeRefSafe(sEPAEdgeRef* edge);
	void RemoveEdgeRef(sEPAEdgeRef* edge);
	void FormNewFace(const Vector3& supp, sEPAEdgeRef* edge, const Vector3& external, std::vector<sEPAEdgeRef*>& new_edges);
	void AppendNewEdges(const std::vector<sEPAEdgeRef*>& new_edges);
};

Vector3 EPA_FindSupp(QuickHull* hull, const Line3& dir);
void EPA_GenerateFaceNormal(Vector3& norm1, Vector3& norm2, sEPAFace* face);
void EPA_GenerateOutboundFaceNormal(const Vector3& external, sEPAFace* face);

//extern eEPAStat epa_stat;