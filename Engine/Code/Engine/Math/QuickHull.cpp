#include "Engine/Math/QuickHull.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Renderable.hpp"

#include <cassert>

QHFace::QHFace(int num, Vector3* sample)
{
	vert_num = num;

	for (int i = 0; i < num; ++i)
	{
		Vector3 vertex = *sample;
		verts.push_back(vertex);
		sample++;
	}
}


QuickHull::QuickHull(uint num, const Vector3& min, const Vector3& max)
{
	GeneratePointSet(num, min, max);
	GenerateInitialFace();
	GenerateInitialHull();
}

QuickHull::~QuickHull()
{
	DeleteVector(m_faceMeshes);
}

void QuickHull::GeneratePointSet(uint num, const Vector3& min, const Vector3& max)
{
	for (uint i = 0; i < num; ++i)
	{
		float x = GetRandomFloatInRange(min.x, max.x);
		float y = GetRandomFloatInRange(min.y, max.y);
		float z = GetRandomFloatInRange(min.z, max.z);
		Vector3 pos = Vector3(x, y, z);
		m_verts.push_back(pos);

		DebugRenderPoint(10000.f, 5.f, pos, Rgba::WHITE, Rgba::WHITE, DEBUG_RENDER_USE_DEPTH);
	}
}

void QuickHull::GenerateInitialFace()
{
	// points of min and max extents along each axis
	float max = -INFINITY;
	Vector3 x_max;
	for (Vector3 pt : m_verts)
	{
		if (pt.x > max)
		{
			x_max = pt;
			max = pt.x;
		}
	}

	max = -INFINITY;
	Vector3 y_max;
	for (Vector3 pt : m_verts)
	{
		if (pt.y > max)
		{
			y_max = pt;
			max = pt.y;
		}
	}

	max = -INFINITY;
	Vector3 z_max;
	for (Vector3 pt : m_verts)
	{
		if (pt.z > max)
		{
			z_max = pt;
			max = pt.z;
		}
	}

	float min = INFINITY;
	Vector3 x_min;
	for (Vector3 pt : m_verts)
	{
		if (pt.x < min)
		{
			x_min = pt;
			min = pt.x;
		}
	}

	min = INFINITY;
	Vector3 y_min;
	for (Vector3 pt : m_verts)
	{
		if (pt.y < min)
		{
			y_min = pt;
			min = pt.y;
		}
	}

	min = INFINITY;
	Vector3 z_min;
	for (Vector3 pt : m_verts)
	{
		if (pt.z < min)
		{
			z_min = pt;
			min = pt.z;
		}
	}

	// given min/max points, choose the pair farthest apart
	Vector3 v1, v2;
	max = -INFINITY;
	float x_apart = (x_max - x_min).GetLength();
	float y_apart = (y_max - y_min).GetLength();
	float z_apart = (z_max - z_min).GetLength();
	if (x_apart > max)
	{
		v1 = x_max; v2 = x_min;
		max = x_apart;
	}
	if (y_apart > max)
	{
		v1 = y_max; v2 = y_min;
		max = y_apart;
	}
	if (z_apart > max)
	{
		v1 = z_max; v2 = z_min;
		max = z_apart;
	}

	// find the farthest point to line formed by v1 and v2
	max = -INFINITY;
	Vector3 v3;
	for (Vector3 pt : m_verts)
	{
		if (pt != v1 && pt != v2)
		{
			float dist = DistPointToEdge(pt, v1, v2);
			if (dist > max)
			{
				v3 = pt;
				max = dist;
			}
		}
	}

	// generate triangle face 
	Vector3 verts[3] = {v1, v2, v3};
	QHFace face = QHFace(3, verts);
	m_faces.push_back(face);
	m_initial = face;

	// delete these points from effective point set
	RemovePoint(v1);
	RemovePoint(v2);
	RemovePoint(v3);
	
	// create mesh
	CreateFaceMesh(face);
	// done
}

void QuickHull::GenerateInitialHull()
{
	// first triangle
	//GenerateInitialFace();
	assert(m_initial.vert_num == 3);
	Vector3 v1 = m_initial.verts[0];
	Vector3 v2 = m_initial.verts[1];
	Vector3 v3 = m_initial.verts[2];

	// say triangle has v1, v2, v3, now in verts there are no such points
	// to have initial hull, we need a v4
	float max = -INFINITY;
	Vector3 v4;
	for (Vector3 pt : m_verts)
	{
		float dist = DistPointToPlaneUnsigned(pt, v1, v2, v3);

		if (dist > max)
		{
			v4 = pt;
			max = dist;
		}
	}

	// build new faces
	Vector3 verts124[3] = {v1, v2, v4};
	Vector3 verts134[3] = {v1, v3, v4};
	Vector3 verts234[3] = {v2, v3, v4};
	QHFace face124 = QHFace(3, verts124);
	QHFace face134 = QHFace(3, verts134);
	QHFace face234 = QHFace(3, verts234);
	m_faces.push_back(face124);
	m_faces.push_back(face134);
	m_faces.push_back(face234);

	// remove v4 from verts
	RemovePoint(v4);

	// create mesh
	CreateFaceMesh(face124);
	CreateFaceMesh(face134);
	CreateFaceMesh(face234);
	// done
}


void QuickHull::RemovePoint(const Vector3& pt)
{
	for (std::vector<Vector3>::size_type idx = 0; idx < m_verts.size(); ++idx)
	{
		if (m_verts[idx] == pt)
		{
			// remove 
			std::vector<Vector3>::iterator it = m_verts.begin();
			m_verts.erase(it + idx);
			idx--;
		}
	}
}


void QuickHull::RenderHull(Renderer* renderer)
{
	for (Mesh* mesh : m_faceMeshes)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		glLineWidth(2.f);

		renderer->m_objectData.model = Matrix44::IDENTITY;

		renderer->m_currentShader->m_state.m_depthCompare = COMPARE_LESS;
		renderer->m_currentShader->m_state.m_cullMode = CULLMODE_BACK;
		renderer->m_currentShader->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;
		renderer->DrawMesh(mesh);
	}
}

void QuickHull::CreateFaceMesh(const QHFace& face)
{
	if (face.vert_num == 3)
	{
		const Vector3& v1 = face.verts[0];
		const Vector3& v2 = face.verts[1];
		const Vector3& v3 = face.verts[2];

		Mesh* mesh = Mesh::CreateTriangleImmediate(VERT_PCU, Rgba::WHITE, v1, v2, v3);
		m_faceMeshes.push_back(mesh);
	}
	else 
	{
		const Vector3& v1 = face.verts[0];
		const Vector3& v2 = face.verts[1];
		const Vector3& v3 = face.verts[2];
		const Vector3& v4 = face.verts[3];

		Mesh* mesh = Mesh::CreateQuadImmediate(VERT_PCU, v1, v2, v3, v4, Rgba::WHITE);
		m_faceMeshes.push_back(mesh);
	}
}