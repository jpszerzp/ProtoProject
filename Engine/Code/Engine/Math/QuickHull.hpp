#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/HalfEdge.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <stack> 
#include <deque>
#include <algorithm>


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
	std::vector<QHFace*> visibleFaces;

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
	//std::vector<QHEdge*> edges;

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
	QHFace(HalfEdge* he, const Vector3& pt);
	QHFace(int num, Vector3* sample);
	~QHFace()
	{
		FlushFaceNormalMesh();

		if (faceMesh != nullptr)
		{
			delete faceMesh;
			faceMesh = nullptr;
		}

		// conflict points are usually orphaned points that will be adopted
		// so we DO NOT delete them here
	}

	void AddConflictPoint(QHVert* pt);
	QHVert* GetFarthestConflictPoint(float& dist) const;
	bool FindTwinAgainstFace(QHFace* face);
	bool FindEdgeTwinAgainstFace(QHFace* face, HalfEdge* entry);

	void SetParentHalfEdge();

	const Vector3& GetVert1() const { return verts[0]; }
	const Vector3& GetVert2() const { return verts[1]; }
	const Vector3& GetVert3() const { return verts[2]; }
	const Vector3& GetVert4() const;
	Vector3 GetFaceCentroid() const;
	void CreateFaceNormalMesh(const Rgba& color);
	void FlushFaceNormalMesh();
	void GenerateEdges();

	bool IsTriangle() const { return vert_num == 3; }
	bool IsPolygon() const { return vert_num == 4; }

	bool ShareEdge(QHFace* other);
	bool IsEdgeShared(HalfEdge* he);

	void ConstructFeatureID() override;

	void DrawFace(Renderer* renderer);
};

class QuickHull
{
public:
	QuickHull(){}
	QuickHull(uint num, const Vector3& min, const Vector3& max);
	~QuickHull();

	// global list of verts that have a chance to sit on surface of hull
	// INVARIANT: m_verts = sum_of(face.conflicts)
	std::vector<QHVert*> m_verts;

	// face
	std::vector<QHFace*> m_faces;

	std::tuple<QHFace*, QHVert*> m_eyePair;

	std::deque<QHFace*> m_visibleFaces;
	std::deque<QHFace*> m_allFaces;
	std::deque<HalfEdge*> m_horizon;		std::vector<Mesh*> m_horizon_mesh;

	// test
	HalfEdge* test_start_he = nullptr;
	HalfEdge* test_he = nullptr;			Mesh* m_test_he_mesh = nullptr;
	HalfEdge* test_he_twin = nullptr;		
	QHFace* test_otherFace = nullptr;

	// orphans
	std::vector<QHVert*> m_orphans;
	QHVert* m_candidate = nullptr;

public:
	bool AddConflictPointInitial(QHVert* vert);
	bool AddConflictPointGeneral(QHVert* vert, std::vector<QHFace*>& faces);
	void AddHorizonMesh(HalfEdge* horizon);
	bool AddToFinalizedFace(QHFeature* feature, const Vector3& closest, QHVert* vert);

	void GeneratePointSet(uint num, const Vector3& min, const Vector3& max);
	void GenerateInitialFace();
	void GenerateInitialHull();
	void GenerateFaceNorms(Vector3& norm1, Vector3& norm2, const QHFace& face);
	void GenerateOutboundNorm(const Vector3& external, QHFace& face);

	void RemovePointGlobal(const Vector3& pt);			// remove point from verts collection
	bool PointOutBoundFace(const Vector3& pt, const QHFace& face);
	QHFeature* FindClosestFeatureInitial(const Vector3& pt, float& dist, Vector3& closest);
	QHFeature* FindClosestFeatureGeneral(const Vector3& pt, float& dist, Vector3& closest, std::vector<QHFace*>& faces);
	QHFace* FindFaceGivenPts(const Vector3& v1, const Vector3& v2, const Vector3& v3, bool& found);
	const std::vector<QHFace*> FindFaceGivenSharedEdge(const QHEdge& edge, bool& found);
	const std::vector<QHFace*> FindFaceGivenSharedVert(const QHVert& vert, bool& found);
	QHVert* GetVert(int idx) { return m_verts[idx]; }
	size_t GetVertNum() const { return m_verts.size(); }
	std::tuple<QHFace*, QHVert*> GetFarthestConflictPair(float& dist) const;

	void ChangeCurrentHalfEdge();

	void RenderHull(Renderer* renderer);
	void RenderFaces(Renderer* renderer);
	void RenderVerts(Renderer* renderer);
	void RenderHorizon(Renderer* renderer);
	void RenderCurrentHalfEdge(Renderer* renderer);

	void CreateNormalMeshes();
	void FlushNormalMeshes();
	void CreateFaceMesh(QHFace& face);
};