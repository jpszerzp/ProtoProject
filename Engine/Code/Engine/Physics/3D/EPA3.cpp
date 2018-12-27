#include "Engine/Physics/3D/EPA3.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"

#include <assert.h>

Vector3 EPA_FindSupp(QuickHull* hull, const Line3& dir)
{
	const std::vector<Vector3>& verts = hull->m_vertices;

	Vector3 supp;
	float ext= -INFINITY;

	for each (const Vector3& vert in verts)
	{
		// project to direction, get the biggest ext
		float this_ext;
		ProjectPointToLineExt(vert, dir, this_ext);

		if (this_ext > ext)
		{
			ext = this_ext;
			supp = vert;
		}
	}

	return supp;
}

void EPA_GenerateFaceNormal(Vector3& norm1, Vector3& norm2, sEPAFace* face)
{
	const Vector3& v1 = face->m_verts[0];
	const Vector3& v2 = face->m_verts[1];
	const Vector3& v3 = face->m_verts[2];

	Vector3 side1 = v2 - v1;
	Vector3 side2 = v3 - v1;

	norm1 = side1.Cross(side2).GetNormalized();
	norm2 = side2.Cross(side1).GetNormalized();
}

void EPA_GenerateOutboundFaceNormal(const Vector3& external, sEPAFace* face)
{
	Vector3 norm1;
	Vector3 norm2;
	EPA_GenerateFaceNormal(norm1, norm2, face);

	// decide on initial face normal based on v4
	Vector3 towardExternal = external - face->m_verts[0];
	float ext1 = DotProduct(towardExternal, norm1);
	Vector3 norm = (ext1 < 0.f) ? norm1 : norm2;
	face->m_normal = norm;
}

//eEPAStat epa_stat = EPA_CREATE_SIMPLEX;

sEPAFace::sEPAFace(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	m_verts[0] = v1;
	m_verts[1] = v2;
	m_verts[2] = v3;

	m_mesh = Mesh::CreateTriangleImmediate(VERT_PCU, Rgba::YELLOW, v1, v2, v3);
}

sEPAFace::~sEPAFace()
{
	if (m_mesh != nullptr)
	{
		delete m_mesh;
		m_mesh = nullptr;
	}
}

//float sEPAFace::DistToPoint(const Vector3& pt)
//{
//	return 0.f;
//}

float sEPAFace::ComputeAndSetDistOrigin()
{
	Vector3 toOrigin = Vector3::ZERO - m_verts[0];
	m_toOrigin = abs(DotProduct(toOrigin, m_normal.GetNormalized()));
	return m_toOrigin;
}

void sEPAFace::Draw(Renderer* renderer) const
{
	DrawTriangle(m_mesh);
}

sEPASimplex::sEPASimplex(const std::set<Vector3>& verts)
{
	// tetra
	if (verts.size() == 4)
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(verts);
		const Vector3& v1 = v[0];
		const Vector3& v2 = v[1];
		const Vector3& v3 = v[2];
		const Vector3& v4 = v[3];

		sEPAFace* f1 = new sEPAFace(v1, v2, v3);

		EPA_GenerateOutboundFaceNormal(v4, f1);

		sEPAFace* f2 = new sEPAFace(v2, v1, v4);
		sEPAFace* f3 = new sEPAFace(v1, v3, v4);
		sEPAFace* f4 = new sEPAFace(v2, v4, v3);

		EPA_GenerateOutboundFaceNormal(v3, f2);
		EPA_GenerateOutboundFaceNormal(v2, f3);
		EPA_GenerateOutboundFaceNormal(v1, f4);

		// compute dist to origin
		f1->ComputeAndSetDistOrigin();
		f2->ComputeAndSetDistOrigin();
		f3->ComputeAndSetDistOrigin();
		f4->ComputeAndSetDistOrigin();

		m_faces.push(f1);
		m_faces.push(f2);
		m_faces.push(f3);
		m_faces.push(f4);

		m_unordered_faces.push_back(f1);
		m_unordered_faces.push_back(f2);
		m_unordered_faces.push_back(f3);
		m_unordered_faces.push_back(f4);
	}
	// triangle
	else if (verts.size() == 3)
	{

	}
	// line
	else
	{

	}
}

sEPASimplex::~sEPASimplex()
{
	DeleteVector(m_unordered_faces);
}

void sEPASimplex::Draw(Renderer* renderer)
{
	for (sEPAFace* face : m_unordered_faces)
		face->Draw(renderer);
}

sEPAFace* sEPASimplex::SelectClosestFaceToOrigin()
{
	sEPAFace* res = m_faces.top();
	//m_faces.pop();
	return res;
}
