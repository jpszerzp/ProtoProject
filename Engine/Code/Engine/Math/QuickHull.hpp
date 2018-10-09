#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Renderer.hpp"

struct QHFace
{
	int vert_num;
	std::vector<Vector3> verts;

	QHFace(){}
	QHFace(int num, Vector3* sample);
	~QHFace(){}
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
	QHFace m_initial;

public:
	void GeneratePointSet(uint num, const Vector3& min, const Vector3& max);
	void GenerateInitialFace();
	void GenerateInitialHull();

	void RemovePoint(const Vector3& pt);			// remove point from verts collection

	void RenderHull(Renderer* renderer);

	void CreateFaceMesh(const QHFace& face);
};