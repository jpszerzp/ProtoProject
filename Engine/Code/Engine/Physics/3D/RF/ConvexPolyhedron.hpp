#pragma once

#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

#include <deque>
#include <unordered_map>

struct ConvexPolygon
{
	Vector3 m_normal;
	std::vector<int> m_vert_idx;
	//std::deque<int> m_sorted_vert_idx;

	// unsorted indices
	ConvexPolygon(const std::vector<int>& idx, const Vector3& normal)
		: m_vert_idx(idx), m_normal(normal){}
	~ConvexPolygon(){}

	//bool operator==(ConvexPolygon& other_polygon);

	int GetVertNum() const { return m_vert_idx.size(); }
};

class ConvexPolyhedron
{
private:
	ConvexHull* m_hull;

	std::vector<Vector3> m_verts;

	//std::vector<IntVector2> m_edges;

	//std::map<Plane, ConvexPolygon> m_plane_face;
	//std::vector<ConvexPolygon> m_faces;

	MeshBuilder m_cpu_mesh;
	Mesh* m_gpu_mesh = nullptr;

public:
	ConvexPolyhedron(ConvexHull* hull);
	~ConvexPolyhedron();

	void Render(Renderer* r);

	Vector3 ComputeIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3);

	//Plane GetPlaneFromConvexPolygon(ConvexPolygon& polygon);
	void SortVerticesCCW(ConvexPolygon& polygon);

	void AppendPolygonMesh(ConvexPolygon& polygon);
};

Vector3 GetPolygonCentroid(const std::vector<Vector3>& verts, const ConvexPolygon& polygon);
int GetPolygonRandomVert(const std::vector<Vector3>& verts, const ConvexPolygon& polygon, Vector3& pos);