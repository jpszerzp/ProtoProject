#pragma once

#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

#include <deque>
#include <unordered_map>

struct ConvexPolygon
{
	Vector3 m_normal = Vector3::ZERO;
	std::vector<int> m_vert_idx;

	// unsorted indices
	ConvexPolygon(){}
	ConvexPolygon(const Vector3& normal)
		: m_normal(normal){}
	ConvexPolygon(const std::vector<int>& idx, const Vector3& normal)
		: m_vert_idx(idx), m_normal(normal){}
	~ConvexPolygon(){}

	int GetVertNum() const { return (int)m_vert_idx.size(); }
	Vector3 GetNormal() const { return m_normal; }

	void AddVertexIndex(const int& vert_idx) { m_vert_idx.push_back(vert_idx); }
};

// DEPRECATED, use CollisionConvexObject instead
class ConvexPolyhedron
{
private:
	ConvexHull* m_hull;

	std::vector<Vector3> m_verts;

	MeshBuilder m_cpu_mesh;
	Mesh* m_gpu_mesh = nullptr;

public:
	Vector3 pos = Vector3::ZERO;
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;

public:
	ConvexPolyhedron(ConvexHull* hull);
	~ConvexPolyhedron();

	void Render(Renderer* r);

	Vector3 ComputeIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3);

	void SortVerticesCCW(ConvexPolygon& polygon);

	void AppendPolygonMesh(ConvexPolygon& polygon);
};

//Vector3 GetPolygonCentroid(const std::vector<Vector3>& verts, const ConvexPolygon& polygon);
int GetPolygonFirstVert(const std::vector<Vector3>& verts, const ConvexPolygon& polygon, Vector3& pos);