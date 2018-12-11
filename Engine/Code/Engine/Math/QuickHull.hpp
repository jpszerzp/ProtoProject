#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/HalfEdge.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <stack> 
#include <deque>
#include <algorithm>
#include <set>

enum eHullStep
{
	CONFLICT,
	EYE,
	HORIZON_START,
	HORIZON_PROCESS,
	OLD_FACE,
	NEW_FACE,
	ORPHAN,
	TOPO_ERROR,
	RESET,
	COMPLETE
};


enum eQHFeature
{
	QH_FACE,
	QH_EDGE,
	QH_VERT,
	QH_NONE
};


class QHFeature
{
private:
	eQHFeature m_id;

public:
	QHFeature(){}
	virtual ~QHFeature(){}

	eQHFeature GetFeatureID() const { return m_id; }
	void SetFeatureID(eQHFeature id) { m_id = id; }
	virtual void ConstructFeatureID(){};
};

class QHVert : public QHFeature
{
public:
	Vector3 vert;
	Mesh* vertMesh = nullptr;
	Rgba color;

public:
	QHVert() { ConstructFeatureID(); }
	explicit QHVert(const Vector3& pt);
	QHVert(const Vector3& pt, const Rgba& color);
	~QHVert();

	// effectively changing the mesh
	void ChangeColor(const Rgba& color);
	
	const Vector3& GetVertRef() const { return vert; }
	void ConstructFeatureID() override;

	void DrawVert(Renderer* renderer) const;
};

class QHEdge : public QHFeature
{
public:
	Vector3 v1;
	Vector3 v2;

	Mesh* edgeMesh = nullptr;

	Rgba color;

public:
	QHEdge() { ConstructFeatureID(); }
	QHEdge(const Vector3& vert1, const Vector3& vert2);
	QHEdge(const Vector3& v1, const Vector3& v2, Rgba color);
	~QHEdge();

	const Vector3& GetVert1Edge() const { return v1; }
	const Vector3& GetVert2Edge() const { return v2; }

	void ConstructFeatureID() override;
};

class QHFace : public QHFeature
{
public:
	int vert_num;
	std::vector<Vector3> verts;			// size of 3 or 4 size, vertices of face (triangle or quad)
	std::vector<QHVert*> conflicts;		// conflicting point list of the face		

	// normal
	Vector3 normal;
	Mesh* normMesh = nullptr;
	Rgba normColor;

	Mesh* faceMesh = nullptr;

	// half edge structure
	HalfEdge* m_entry = nullptr;		// linked list entry for half edges

public:
	QHFace(){ ConstructFeatureID(); }
	QHFace(const Vector3& v1, const Vector3& v2, const Vector3& v3);
	//QHFace(HalfEdge* onHorizon, HalfEdge* horizon_next, HalfEdge* horizon_prev);
	QHFace(int num, Vector3* sample);
	QHFace(HalfEdge* he, const Vector3& head, const Vector3& eyePos);
	~QHFace();

	void AddConflictPoint(QHVert* pt, QuickHull* hull);
	QHVert* GetFarthestConflictPoint(float& dist) const;
	QHVert* GetFarthestConflictPoint() const;
	bool FindTwinAgainstFace(QHFace* face);
	bool FindEdgeTwinAgainstFace(QHFace* face, HalfEdge* entry);

	void SetParentHalfEdge();

	void VerifyHalfEdgeNext();
	void VerifyHalfEdgeParent();
	void VerifyHalfEdgeTwin();

	const Vector3& GetVert1() const { return verts[0]; }
	const Vector3& GetVert2() const { return verts[1]; }
	const Vector3& GetVert3() const { return verts[2]; }
	const Vector3& GetVert4() const;
	Vector3 GetFaceCentroid() const;
	void CreateFaceNormalMesh(const Rgba& color);
	void FlushFaceNormalMesh();
	void FlushFaceMesh();
	void GenerateEdges();

	bool IsTriangle() const { return vert_num == 3; }
	bool IsPolygon() const { return vert_num == 4; }

	bool UpdateSharedEdge(QHFace* other);
	bool CheckAndSetEdgeShared(HalfEdge* he);

	void ConstructFeatureID() override;

	void DrawFaceAndNormal(Renderer* renderer);
	void DrawFace(Renderer* renderer);
};

class QuickHull
{
public:
	QuickHull(){}
	QuickHull(uint num, const Vector3& min, const Vector3& max, bool auto_gen = false);
	~QuickHull();

	// global list of verts that have a chance to sit on surface of hull
	// INVARIANT: m_verts = sum_of(face.conflicts)
	std::vector<QHVert*> m_verts;

	// face
	std::vector<QHFace*> m_faces;
	std::vector<QHFace*> m_newFaces;

	std::tuple<QHFace*, QHVert*> m_eyePair;

	// QH utils
	std::deque<QHFace*> m_exploredFaces;
	std::deque<QHFace*> m_visitedFaces;
	std::deque<HalfEdge*> m_horizon;		
	std::deque<std::tuple<Vector3, Vector3, HalfEdge*>> m_horizon_infos;
	bool m_render_horizon = false;

	// test
	HalfEdge* m_start_he = nullptr;
	HalfEdge* m_current_he = nullptr;			
	QHFace* m_otherFace = nullptr;

	// orphans
	std::deque<QHVert*> m_orphans;

	// anchor point that is "inside" the hull generated in future rounds
	// always valid for use to generate normals of new faces
	Vector3 m_anchor = Vector3::INVALID;

	//bool m_hull_complete = false;
	bool m_auto_gen = false;
	eHullStep m_gen_step;
	int m_vertCount = 0;

public:
	bool AddConflictPointInitial(QHVert* vert);
	bool AddConflictPointGeneral(QHVert* vert, std::vector<QHFace*>& faces);
	//void AddHorizonMesh(HalfEdge* horizon);
	bool AddToFinalizedFaceInitial(QHFeature* feature, const Vector3& closest, QHVert* vert);
	bool AddToFinalizedFaceGeneral(QHFeature* feature, const Vector3& closest, QHVert* vert, std::vector<QHFace*>& faces);
	void AddHorizonInfo(HalfEdge* he);
	void AddFace(QHFace* face) { m_faces.push_back(face); }
	void AddNewFace(QHFace* face) { m_newFaces.push_back(face); }

	void GeneratePointSet(uint num, const Vector3& min, const Vector3& max);
	void GenerateInitialFace();
	void GenerateInitialHull();
	void GenerateFaceNorms(Vector3& norm1, Vector3& norm2, const QHFace& face);
	void GenerateOutboundNorm(const Vector3& external, QHFace& face);

	void RemovePointGlobal(const Vector3& pt);			// remove point from verts collection
	bool PointOutBoundFace(const Vector3& pt, const QHFace& face);
	QHFeature* FindClosestFeatureInitial(const Vector3& pt, float& dist, Vector3& closest);
	QHFeature* FindClosestFeatureGeneral(const Vector3& pt, float& dist, Vector3& closest, std::vector<QHFace*>& faces);
	QHFace* FindFaceGivenPtsInitial(const Vector3& v1, const Vector3& v2, const Vector3& v3, bool& found);
	QHFace* FindFaceGivenPtsGeneral(const Vector3& v1, const Vector3& v2, const Vector3& v3, bool& found, const std::vector<QHFace*>& new_faces);
	const std::vector<QHFace*> FindFaceGivenSharedEdgeInitial(const QHEdge& edge, bool& found);
	const std::vector<QHFace*> FindFaceGivenSharedEdgeGeneral(const QHEdge& edge, bool& found, const std::vector<QHFace*>& new_faces);
	const std::vector<QHFace*> FindFaceGivenSharedVertInitial(const QHVert& vert, bool& found);
	QHVert* GetVert(int idx) { return m_verts[idx]; }
	size_t GetVertNum() const { return m_verts.size(); }
	std::tuple<QHFace*, QHVert*> GetFarthestConflictPair(float& dist) const;
	std::tuple<QHFace*, QHVert*> GetFarthestConflictPair() const;
	std::set<Vector3> GetPointSet() const;
	bool HasVisitedFace(QHFace* face);
	bool IsLastVisitedFace(QHFace* face);
	bool ReachStartHalfEdge();
	QHFace* PeekVisitedFrontier();
	void RemoveVisitedFrontier();
	HalfEdge* PeekHorizonFrontier();
	void RemoveHorizonFrontier();

	void ChangeCurrentHalfEdgeOldFace();
	void ChangeCurrentHalfEdgeNewFace();
	void ChangeCurrentHalfEdgeMesh();
	void ChangeOtherFace();

	void UpdateHull();

	void RenderHull(Renderer* renderer);
	void RenderFacesAndNormals(Renderer* renderer);
	void RenderFaces(Renderer* renderer);
	void RenderVerts(Renderer* renderer);
	void RenderHorizon(Renderer* renderer);
	//void RenderAnchor(Renderer* renderer);
	//void RenderCurrentHalfEdge(Renderer* renderer);

	void CreateAllNormalMeshes();
	void FlushNormalMeshes();
	void CreateFaceMesh(QHFace& face, Rgba color = Rgba::WHITE);
	void CreateFaceNormalMesh(QHFace& face);
};