#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Renderer.hpp"

struct QHFace
{
	int vert_num;
	std::vector<Vector3> verts;
	Vector3 normal;

	QHFace(){}
	QHFace(int num, Vector3* sample);
	~QHFace(){}

	Vector3 GetFaceCentroid() const;
	Mesh* CreateFaceNormalMesh() const;
};

class QuickHull
{
public:
	QuickHull(){}
	QuickHull(uint num, const Vector3& min, const Vector3& max);
	~QuickHull();

	// verts
	std::vector<Vector3> m_verts;

	// face
	std::vector<QHFace> m_faces;
	std::vector<Mesh*> m_faceMeshes;
	std::vector<Mesh*> m_normalMeshes;

public:
	void GeneratePointSet(uint num, const Vector3& min, const Vector3& max);
	void GenerateInitialFace();
	void GenerateInitialHull();
	void GenerateFaceNorms(Vector3& norm1, Vector3& norm2, const QHFace& face);
	void GenerateOutboundNorm(const Vector3& external, QHFace& face);

	void RemovePoint(const Vector3& pt);			// remove point from verts collection

	void RenderHull(Renderer* renderer);
	void RenderNormals(Renderer* renderer);

	void CreateNormalMeshes();
	void FlushNormalMeshes();
	void CreateFaceMesh(const QHFace& face);
};