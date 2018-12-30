#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB3::OBB3(Vector3 center, Vector3 forward, Vector3 up, Vector3 right, Vector3 halfExt)
	: m_center(center), m_forward(forward), m_up(up), m_right(right), m_halfExt(halfExt) 
{
	Vector3 front_center = center + m_forward * halfExt.z;
	OBB3Face front_face = OBB3Face(m_forward, front_center, F1);

	Vector3 back_center = center + (-forward) * halfExt.z;
	OBB3Face back_face = OBB3Face(-m_forward, back_center, F2);

	Vector3 right_center = center + m_right * halfExt.x;
	OBB3Face right_face = OBB3Face(m_right, right_center, F3);

	Vector3 left_center = center + (-right) * halfExt.x;
	OBB3Face left_face = OBB3Face(-m_right, left_center, F4);

	Vector3 up_center = center + m_up * halfExt.y;
	OBB3Face up_face = OBB3Face(m_up, up_center, F5);
	
	Vector3 down_center = center + (-up) * halfExt.y;
	OBB3Face down_face = OBB3Face(-m_up, down_center, F6);

	m_faces.push_back(front_face);
	m_faces.push_back(back_face);
	m_faces.push_back(right_face);
	m_faces.push_back(left_face);
	m_faces.push_back(up_face);
	m_faces.push_back(down_face);

	Vector3 btl = m_center - m_right * halfExt.x + m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 bbl = m_center - m_right * halfExt.x - m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 bbr = m_center + m_right * halfExt.x - m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 btr = m_center + m_right * halfExt.x + m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 ftl = m_center - m_right * halfExt.x + m_up * halfExt.y + m_forward * halfExt.z;
	Vector3 fbl = m_center - m_right * halfExt.x - m_up * halfExt.y + m_forward * halfExt.z;
	Vector3 fbr = m_center + m_right * halfExt.x - m_up * halfExt.y + m_forward * halfExt.z;
	Vector3 ftr = m_center + m_right * halfExt.x + m_up * halfExt.y + m_forward * halfExt.z;
	OBB3Vert vert_ftl = OBB3Vert(ftl, V1);
	OBB3Vert vert_fbl = OBB3Vert(fbl, V2);
	OBB3Vert vert_fbr = OBB3Vert(fbr, V3);
	OBB3Vert vert_ftr = OBB3Vert(ftr, V4);
	OBB3Vert vert_btl = OBB3Vert(btl, V5);
	OBB3Vert vert_bbl = OBB3Vert(bbl, V6);
	OBB3Vert vert_bbr = OBB3Vert(bbr, V7);
	OBB3Vert vert_btr = OBB3Vert(btr, V8);
	m_verts.push_back(vert_ftl);
	m_verts.push_back(vert_fbl);
	m_verts.push_back(vert_fbr);
	m_verts.push_back(vert_ftr);
	m_verts.push_back(vert_btl);
	m_verts.push_back(vert_bbl);
	m_verts.push_back(vert_bbr);
	m_verts.push_back(vert_btr);

	OBB3Edge e1 = OBB3Edge(vert_ftl, vert_fbl, E1);
	OBB3Edge e2 = OBB3Edge(vert_fbl, vert_fbr, E2);
	OBB3Edge e3 = OBB3Edge(vert_fbr, vert_ftr, E3);
	OBB3Edge e4 = OBB3Edge(vert_ftr, vert_ftl, E4);
	OBB3Edge e5 = OBB3Edge(vert_btl, vert_bbl, E5);
	OBB3Edge e6 = OBB3Edge(vert_bbl, vert_bbr, E6);
	OBB3Edge e7 = OBB3Edge(vert_bbr, vert_btr, E7);
	OBB3Edge e8 = OBB3Edge(vert_btr, vert_btl, E8);
	OBB3Edge e9 = OBB3Edge(vert_fbl, vert_bbl, E9);
	OBB3Edge e10 = OBB3Edge(vert_fbr, vert_bbr, E10);
	OBB3Edge e11 = OBB3Edge(vert_ftr, vert_btr, E11);
	OBB3Edge e12 = OBB3Edge(vert_ftl, vert_btl, E12);
	m_edges.push_back(e1);
	m_edges.push_back(e2);
	m_edges.push_back(e3);
	m_edges.push_back(e4);
	m_edges.push_back(e5);
	m_edges.push_back(e6);
	m_edges.push_back(e7);
	m_edges.push_back(e8);
	m_edges.push_back(e9);
	m_edges.push_back(e10);
	m_edges.push_back(e11);
	m_edges.push_back(e12);
}

Vector3 OBB3::GetFTLExt() const
{
	return GetFTL() - GetCenter();
}

Vector3 OBB3::GetFBLExt() const
{
	return GetFBL() - GetCenter();
}

Vector3 OBB3::GetFBRExt() const
{
	return GetFBR() - GetCenter();
}

Vector3 OBB3::GetFTRExt() const
{
	return GetFTR() - GetCenter();
}

Vector3 OBB3::GetBTLExt() const
{
	return GetBTL() - GetCenter();
}

Vector3 OBB3::GetBBLExt() const
{
	return GetBBL() - GetCenter();
}

Vector3 OBB3::GetBBRExt() const
{
	return GetBBR() - GetCenter();
}

Vector3 OBB3::GetBTRExt() const
{
	return GetBTR() - GetCenter();
}

// along is normalized
float OBB3::GetFTLExtAlong(const Vector3& along) const
{
	return (DotProduct(GetFTLExt(), along));
}

float OBB3::GetFBLExtAlong(const Vector3& along) const
{
	return (DotProduct(GetFBLExt(), along));
}

float OBB3::GetFBRExtAlong(const Vector3& along) const
{
	return (DotProduct(GetFBRExt(), along));
}

float OBB3::GetFTRExtAlong(const Vector3& along) const
{
	return (DotProduct(GetFTRExt(), along));
}

float OBB3::GetBTLExtAlong(const Vector3& along) const
{
	return (DotProduct(GetBTLExt(), along));
}

float OBB3::GetBBLExtAlong(const Vector3& along) const
{
	return(DotProduct(GetBBLExt(), along));
}

float OBB3::GetBBRExtAlong(const Vector3& along) const
{
	return (DotProduct(GetBBRExt(), along));
}

float OBB3::GetBTRExtAlong(const Vector3& along) const
{
	return (DotProduct(GetBTRExt(), along));
}

std::vector<Vector3> OBB3::GetVertices() const
{
	std::vector<Vector3> verts;

	verts.push_back(GetFTL());
	verts.push_back(GetFTR());
	verts.push_back(GetFBL());
	verts.push_back(GetFBR());
	verts.push_back(GetBTL());
	verts.push_back(GetBTR());
	verts.push_back(GetBBL());
	verts.push_back(GetBBR());

	return verts;
}

std::vector<Vector3> OBB3::GetBase() const
{
	std::vector<Vector3> base;
	
	base.push_back(m_forward);
	base.push_back(m_right);
	base.push_back(m_up);
	
	return base;
}

void OBB3::SetCenter(const Vector3& center)
{
	m_center = center;
}

void OBB3::UpdateFace()
{
	// the order of face is f b r l u b
	Vector3 front_center = m_center + m_forward * m_halfExt.z;
	m_faces[0].m_center = front_center;
	m_faces[0].m_normal = m_forward;

	Vector3 back_center = m_center + (-m_forward) * m_halfExt.z;
	m_faces[1].m_center = back_center;
	m_faces[1].m_normal = -m_forward;

	Vector3 right_center = m_center + m_right * m_halfExt.x;
	m_faces[2].m_center = right_center;
	m_faces[2].m_normal = m_right;

	Vector3 left_center = m_center + (-m_right) * m_halfExt.x;
	m_faces[3].m_center = left_center;
	m_faces[3].m_normal = -m_right;

	Vector3 up_center = m_center + m_up * m_halfExt.y;
	m_faces[4].m_center = up_center;
	m_faces[4].m_normal = m_up;

	Vector3 down_center = m_center + (-m_up) * m_halfExt.y;
	m_faces[5].m_center = down_center;
	m_faces[5].m_normal = -m_up;
}

void OBB3::UpdateVertAndEdge()
{
	Vector3 btl = m_center - m_right * m_halfExt.x + m_up * m_halfExt.y - m_forward * m_halfExt.z;
	Vector3 bbl = m_center - m_right * m_halfExt.x - m_up * m_halfExt.y - m_forward * m_halfExt.z;
	Vector3 bbr = m_center + m_right * m_halfExt.x - m_up * m_halfExt.y - m_forward * m_halfExt.z;
	Vector3 btr = m_center + m_right * m_halfExt.x + m_up * m_halfExt.y - m_forward * m_halfExt.z;
	Vector3 ftl = m_center - m_right * m_halfExt.x + m_up * m_halfExt.y + m_forward * m_halfExt.z;
	Vector3 fbl = m_center - m_right * m_halfExt.x - m_up * m_halfExt.y + m_forward * m_halfExt.z;
	Vector3 fbr = m_center + m_right * m_halfExt.x - m_up * m_halfExt.y + m_forward * m_halfExt.z;
	Vector3 ftr = m_center + m_right * m_halfExt.x + m_up * m_halfExt.y + m_forward * m_halfExt.z;

	m_verts[0].m_vert = ftl;
	m_verts[1].m_vert = fbl;
	m_verts[2].m_vert = fbr;
	m_verts[3].m_vert = ftr;
	m_verts[4].m_vert = btl;
	m_verts[5].m_vert = bbl;
	m_verts[6].m_vert = bbr;
	m_verts[7].m_vert = btr;

	m_edges[0].m_end1.m_vert = ftl;
	m_edges[0].m_end2.m_vert = fbl;
	m_edges[1].m_end1.m_vert = fbl;
	m_edges[1].m_end2.m_vert = fbr;
	m_edges[2].m_end1.m_vert = fbr;
	m_edges[2].m_end2.m_vert = ftr;
	m_edges[3].m_end1.m_vert = ftr;
	m_edges[3].m_end2.m_vert = ftl;
	m_edges[4].m_end1.m_vert = btl;
	m_edges[4].m_end2.m_vert = bbl;
	m_edges[5].m_end1.m_vert = bbl;
	m_edges[5].m_end2.m_vert = bbr;
	m_edges[6].m_end1.m_vert = bbr;
	m_edges[6].m_end2.m_vert = btr;
	m_edges[7].m_end1.m_vert = btr;
	m_edges[7].m_end2.m_vert = btl;
	m_edges[8].m_end1.m_vert = fbl;
	m_edges[8].m_end2.m_vert = bbl;
	m_edges[9].m_end1.m_vert = fbr;
	m_edges[9].m_end2.m_vert = bbr;
	m_edges[10].m_end1.m_vert = ftr;
	m_edges[10].m_end2.m_vert = btr;
	m_edges[11].m_end1.m_vert = ftl;
	m_edges[11].m_end2.m_vert = btl;
}


const bool OBB3Edge::operator<(const OBB3Edge& compared) const
{
	Vector3 vec1 = m_end1.m_vert - m_end2.m_vert;
	Vector3 vec2 = compared.m_end1.m_vert - compared.m_end2.m_vert;
	return vec1.GetLength() < vec2.GetLength();
}

Vector3 OBB3Edge::ToVec3() const
{
	const Vector3& end1_v3 = m_end1.m_vert;
	const Vector3& end2_v3 = m_end2.m_vert;

	return (end2_v3 - end1_v3);
}
