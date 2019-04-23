#include "Engine/Physics/3D/EPA3.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <assert.h>
//#include <algorithm>

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

	//const Vector3& v1 = face->GetMKSIVertPos(0);
	//const Vector3& v2 = face->GetMKSIVertPos(1);
	//const Vector3& v3 = face->GetMKSIVertPos(2);

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
	//Vector3 towardExternal = external - face->GetMKSIVertPos(0);
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

//sEPAFace::sEPAFace(const sEPAVert& v1, const sEPAVert& v2, const sEPAVert& v3)
//{
//	m_verts[0] = v1;
//	m_verts[1] = v2;
//	m_verts[2] = v3;
//
//	const Vector3& vert1 = v1.mksi_vert;
//	const Vector3& vert2 = v2.mksi_vert;
//	const Vector3& vert3 = v3.mksi_vert;
//	m_mesh = Mesh::CreateTriangleImmediate(VERT_PCU, Rgba::YELLOW, vert1, vert2, vert3);
//}

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
	//Vector3 toOrigin = Vector3::ZERO - m_verts[0].mksi_vert;
	m_toOrigin = abs(DotProduct(toOrigin, m_normal.GetNormalized()));
	return m_toOrigin;
}

//Vector3 sEPAFace::GetMKSIVertPos(int idx) const
//{
//	return m_verts[idx].mksi_vert;
//}

void sEPAFace::Draw(Renderer*) const
{
	if (m_mesh != nullptr)
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
		// in this case, edge_ref_12 and edge_ref_21 should be equivalent
		sEPAEdgeRef* edge_ref_12 = new sEPAEdgeRef(v1, v2);
		sEPAEdgeRef* edge_ref_23 = new sEPAEdgeRef(v2, v3);
		sEPAEdgeRef* edge_ref_13 = new sEPAEdgeRef(v1, v3);
		AddEdgeRef(edge_ref_12);
		AddEdgeRef(edge_ref_23);
		AddEdgeRef(edge_ref_13);

		EPA_GenerateOutboundFaceNormal(v4, f1);

		sEPAFace* f2 = new sEPAFace(v2, v1, v4);
		//sEPAEdgeRef* edge_ref_21 = new sEPAEdgeRef(v2, v1);
		sEPAEdgeRef* edge_ref_14 = new sEPAEdgeRef(v1, v4);
		sEPAEdgeRef* edge_ref_24 = new sEPAEdgeRef(v2, v4);
		//AddEdgeRef(edge_ref_21);
		AddEdgeRef(edge_ref_12);
		AddEdgeRef(edge_ref_14);
		AddEdgeRef(edge_ref_24);

		sEPAFace* f3 = new sEPAFace(v1, v3, v4);
		//sEPAEdgeRef* edge_ref_13 = new sEPAEdgeRef(v1, v3);
		//sEPAEdgeRef* edge_ref_14 = new sEPAEdgeRef(v1, v4);
		sEPAEdgeRef* edge_ref_34 = new sEPAEdgeRef(v3, v4);
		AddEdgeRef(edge_ref_13);
		AddEdgeRef(edge_ref_14);
		AddEdgeRef(edge_ref_34);

		sEPAFace* f4 = new sEPAFace(v2, v4, v3);
		//sEPAEdgeRef* edge_ref_24 = new sEPAEdgeRef(v2, v4);
		//sEPAEdgeRef* edge_ref_23 = new sEPAEdgeRef(v2, v3);
		//sEPAEdgeRef* edge_ref_43 = new sEPAEdgeRef(v4, v3);
		AddEdgeRef(edge_ref_24);
		AddEdgeRef(edge_ref_23);
		//AddEdgeRef(edge_ref_43);
		AddEdgeRef(edge_ref_34);

		EPA_GenerateOutboundFaceNormal(v3, f2);
		EPA_GenerateOutboundFaceNormal(v2, f3);
		EPA_GenerateOutboundFaceNormal(v1, f4);

		// compute dist to origin
		f1->ComputeAndSetDistOrigin();
		f2->ComputeAndSetDistOrigin();
		f3->ComputeAndSetDistOrigin();
		f4->ComputeAndSetDistOrigin();

		//m_faces.push(f1);
		//m_faces.push(f2);
		//m_faces.push(f3);
		//m_faces.push(f4);

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
	//sEPAFace* res = m_faces.top();
	////m_faces.pop();
	//return res;

	float min = INFINITY;
	sEPAFace* res = nullptr;

	for each (sEPAFace* face in m_unordered_faces)
	{
		float this_dist = face->m_toOrigin;

		if (this_dist < min)
		{
			min = this_dist;
			res = face;
		}
	}
	
	return res;
}

bool sEPASimplex::DeleteVisibleFacesForPt(const Vector3& pt)
{
	bool res = false;

	for (std::vector<sEPAFace*>::size_type idx = 0; idx < m_unordered_faces.size(); ++idx)
	{
		sEPAFace* face = m_unordered_faces[idx];
		if (IsFaceVisibleForPt(face, pt))
		{
			std::vector<sEPAFace*>::iterator it = m_unordered_faces.begin() + idx;
			m_unordered_faces.erase(it);

			// by deleting this face, delete edge ref or its reference count
			const Vector3& v1 = face->m_verts[0];
			const Vector3& v2 = face->m_verts[1];
			const Vector3& v3 = face->m_verts[2];
			sEPAEdgeRef* edge_ref12 = new sEPAEdgeRef(v1, v2);
			sEPAEdgeRef* edge_ref13 = new sEPAEdgeRef(v1, v3);
			sEPAEdgeRef* edge_ref23 = new sEPAEdgeRef(v2, v3);
			RemoveEdgeRef(edge_ref12);
			RemoveEdgeRef(edge_ref13);
			RemoveEdgeRef(edge_ref23);

			delete face;
			face = nullptr;		// optional

			idx--;

			res = true;
		}
	}

	return res;
}

bool sEPASimplex::IsFaceVisibleForPt(sEPAFace* face, const Vector3& pt)
{
	const Vector3& normal = face->m_normal;
	const Vector3& vert = face->m_verts[0];
	float ext = DotProduct((pt - vert), normal);
	return (ext > 0.f);
}

sEPAEdgeRef* sEPASimplex::IsEdgeExisted(sEPAEdgeRef* edge)
{
	for (sEPAEdgeRef* my_edge : m_edge_refs)
	{
		const Vector3& one_end = edge->ends[0];
		const Vector3& the_other_end = edge->ends[1];

		bool one_end_found = (std::find(my_edge->ends.begin(), my_edge->ends.end(), one_end) != my_edge->ends.end());
		bool the_other_end_found = (std::find(my_edge->ends.begin(), my_edge->ends.end(), the_other_end) != my_edge->ends.end());

		if (one_end_found && the_other_end_found)
			return my_edge;

		//if (my_edge == edge)
		//	return my_edge;
	}

	return nullptr;
}

sEPAEdgeRef* sEPASimplex::IsEdgeExisted(sEPAEdgeRef* edge, std::vector<sEPAEdgeRef*>& edge_set)
{
	for (sEPAEdgeRef* the_edge : edge_set)
	{
		const Vector3& one_end = edge->ends[0];
		const Vector3& the_other_end = edge->ends[1];

		bool one_end_found = (std::find(the_edge->ends.begin(), the_edge->ends.end(), one_end) != the_edge->ends.end());
		bool the_other_end_found = (std::find(the_edge->ends.begin(), the_edge->ends.end(), the_other_end) != the_edge->ends.end());

		if (one_end_found && the_other_end_found)
			return the_edge;
	}

	return nullptr;
}

void sEPASimplex::AddEdgeRef(sEPAEdgeRef* edge)
{
	sEPAEdgeRef* existed = IsEdgeExisted(edge);
	if (existed == nullptr)
	{
		// does not exist
		m_edge_refs.push_back(edge);
		edge->ref_count++;
	}
	else
	{
		existed->ref_count++;

		//delete edge;
		//edge = nullptr;
	}
}

void sEPASimplex::AddEdgeRefSafe(sEPAEdgeRef* edge, std::vector<sEPAEdgeRef*>& edge_set)
{
	sEPAEdgeRef* existed = IsEdgeExisted(edge, edge_set);
	if (existed == nullptr)
	{
		edge_set.push_back(edge);
		edge->ref_count++;
	}
	else
	{
		existed->ref_count++;

		delete edge;
		edge = nullptr;
	}
}

void sEPASimplex::AddEdgeRefSafe(sEPAEdgeRef* edge)
{
	sEPAEdgeRef* existed = IsEdgeExisted(edge);
	if (existed == nullptr)
	{
		// does not exist
		m_edge_refs.push_back(edge);
		edge->ref_count++;
	}
	else
	{
		existed->ref_count++;

		delete edge;
		edge = nullptr;
	}
}

void sEPASimplex::RemoveEdgeRef(sEPAEdgeRef* edge)
{
	sEPAEdgeRef* existed = IsEdgeExisted(edge);
	ASSERT_OR_DIE(existed != nullptr, "edge'ref count has been reduced to 0 previously");

	existed->ref_count--;

	if (existed->ref_count == 0)
	{
		std::vector<sEPAEdgeRef*>::iterator it = std::find(m_edge_refs.begin(), m_edge_refs.end(), existed);
		m_edge_refs.erase(it);

		delete existed;
		existed = nullptr;
	}

	delete edge;
	edge = nullptr;
}

void sEPASimplex::FormNewFace(const Vector3& supp, sEPAEdgeRef* edge, const Vector3& external, std::vector<sEPAEdgeRef*>& new_edges)
{
	const Vector3& one_end = edge->ends[0];
	const Vector3& the_other_end = edge->ends[1];

	// Make sure face is formed correctly
	sEPAFace* new_face = new sEPAFace(supp, one_end, the_other_end);

	// but the above does not generate normal, nor does it assign dist to origin
	EPA_GenerateOutboundFaceNormal(external, new_face);
	new_face->ComputeAndSetDistOrigin();
	m_unordered_faces.push_back(new_face);

	// Make sure edges are formed correctly
	// remember that this edge has ref count of 1, increment it
	edge->ref_count++;

	//// for the other two edges, i do not know if it has been recorded
	//// so i will new it off and try to add a record
	//sEPAEdgeRef* e1 = new sEPAEdgeRef(supp, one_end);
	//sEPAEdgeRef* e2 = new sEPAEdgeRef(supp, the_other_end);
	//AddEdgeRefSafe(e1);
	//AddEdgeRefSafe(e2);
	//AddEdgeRef(e1);
	//AddEdgeRef(e2);

	sEPAEdgeRef* e1 = new sEPAEdgeRef(supp, one_end);
	sEPAEdgeRef* e2 = new sEPAEdgeRef(supp, the_other_end);
	AddEdgeRefSafe(e1, new_edges);
	AddEdgeRefSafe(e2, new_edges);
}

void sEPASimplex::AppendNewEdges(const std::vector<sEPAEdgeRef*>& new_edges)
{
	m_edge_refs.insert(m_edge_refs.end(), new_edges.begin(), new_edges.end());
}

sEPAEdgeRef::sEPAEdgeRef(const Vector3& v1, const Vector3& v2)
{
	ends.push_back(v1);
	ends.push_back(v2);
	ref_count = 0;
}

bool sEPAEdgeRef::operator==(const sEPAEdgeRef& compare) const
{
	const Vector3& one_end = compare.ends[0];
	const Vector3& the_other_end = compare.ends[1];

	bool one_end_found = (std::find(ends.begin(), ends.end(), one_end) != ends.end());
	bool the_other_end_found = (std::find(ends.begin(), ends.end(), the_other_end) != ends.end());

	if (one_end_found && the_other_end_found)
		return true;

	return false;
}
