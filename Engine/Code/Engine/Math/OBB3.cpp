#include "Engine/Math/OBB3.hpp"

OBB3::OBB3(Vector3 center, Vector3 forward, Vector3 up, Vector3 right, Vector3 halfExt)
	: m_center(center), m_forward(forward), m_up(up), m_right(right), m_halfExt(halfExt) 
{
	Vector3 front_center = center + Vector3(0.f, 0.f, halfExt.z);
	OBB3Face front_face = OBB3Face(m_forward, front_center);

	Vector3 back_center = center + Vector3(0.f, 0.f, -halfExt.z);
	OBB3Face back_face = OBB3Face(-m_forward, back_center);

	Vector3 right_center = center + Vector3(halfExt.x, 0.f, 0.f);
	OBB3Face right_face = OBB3Face(m_right, right_center);

	Vector3 left_center = center + Vector3(-halfExt.x, 0.f, 0.f);
	OBB3Face left_face = OBB3Face(-m_right, left_center);

	Vector3 up_center = center + Vector3(0.f, halfExt.y, 0.f);
	OBB3Face up_face = OBB3Face(m_up, up_center);
	
	Vector3 down_center = center + Vector3(0.f, -halfExt.y, 0.f);
	OBB3Face down_face = OBB3Face(-m_up, down_center);

	m_faces.push_back(front_face);
	m_faces.push_back(back_face);
	m_faces.push_back(right_face);
	m_faces.push_back(left_face);
	m_faces.push_back(up_face);
	m_faces.push_back(down_face);

	Vector3 ftl = m_center - m_right * halfExt.x + m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 fbl = m_center - m_right * halfExt.x - m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 fbr = m_center + m_right * halfExt.x - m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 ftr = m_center + m_right * halfExt.x + m_up * halfExt.y - m_forward * halfExt.z;
	Vector3 btl = m_center - m_right * halfExt.x + m_up * halfExt.y + m_forward * halfExt.z;
	Vector3 bbl = m_center - m_right * halfExt.x - m_up * halfExt.y + m_forward * halfExt.z;
	Vector3 bbr = m_center + m_right * halfExt.x - m_up * halfExt.y + m_forward * halfExt.z;
	Vector3 btr = m_center + m_right * halfExt.x + m_up * halfExt.y + m_forward * halfExt.z;
	OBB3Vert vert_ftl = OBB3Vert(ftl);
	OBB3Vert vert_fbl = OBB3Vert(fbl);
	OBB3Vert vert_fbr = OBB3Vert(fbr);
	OBB3Vert vert_ftr = OBB3Vert(ftr);
	OBB3Vert vert_btl = OBB3Vert(btl);
	OBB3Vert vert_bbl = OBB3Vert(bbl);
	OBB3Vert vert_bbr = OBB3Vert(bbr);
	OBB3Vert vert_btr = OBB3Vert(btr);
	OBB3Edge e1 = OBB3Edge(vert_ftl, vert_fbl);
	OBB3Edge e2 = OBB3Edge(vert_fbl, vert_fbr);
	OBB3Edge e3 = OBB3Edge(vert_fbr, vert_ftr);
	OBB3Edge e4 = OBB3Edge(vert_ftr, vert_ftl);
	OBB3Edge e5 = OBB3Edge(vert_btl, vert_bbl);
	OBB3Edge e6 = OBB3Edge(vert_bbl, vert_bbr);
	OBB3Edge e7 = OBB3Edge(vert_bbr, vert_btr);
	OBB3Edge e8 = OBB3Edge(vert_btr, vert_btl);
	OBB3Edge e9 = OBB3Edge(vert_fbl, vert_bbl);
	OBB3Edge e10 = OBB3Edge(vert_fbr, vert_bbr);
	OBB3Edge e11 = OBB3Edge(vert_ftr, vert_btr);
	OBB3Edge e12 = OBB3Edge(vert_ftl, vert_btl);
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

	m_verts.push_back(vert_ftl);
	m_verts.push_back(vert_fbl);
	m_verts.push_back(vert_fbr);
	m_verts.push_back(vert_ftr);
	m_verts.push_back(vert_btl);
	m_verts.push_back(vert_bbl);
	m_verts.push_back(vert_bbr);
	m_verts.push_back(vert_btr);
}


Vector3 OBB3::GetFTL() const
{
	return GetCenter() - GetHalfExtX() + GetHalfExtY() - GetHalfExtZ();
}

Vector3 OBB3::GetFBL() const
{
	return GetCenter() - GetHalfExtX() - GetHalfExtY() - GetHalfExtZ();
}

Vector3 OBB3::GetFBR() const
{
	return GetCenter() + GetHalfExtX() - GetHalfExtY() - GetHalfExtZ();
}

Vector3 OBB3::GetFTR() const
{
	return GetCenter() + GetHalfExtX() + GetHalfExtY() - GetHalfExtZ();
}

Vector3 OBB3::GetBTL() const
{
	return GetCenter() - GetHalfExtX() + GetHalfExtY() + GetHalfExtZ();
}

Vector3 OBB3::GetBBL() const
{
	return GetCenter() - GetHalfExtX() - GetHalfExtY() + GetHalfExtZ();
}

Vector3 OBB3::GetBBR() const
{
	return GetCenter() + GetHalfExtX() - GetHalfExtY() + GetHalfExtZ();
}

Vector3 OBB3::GetBTR() const
{
	return GetCenter() + GetHalfExtX() + GetHalfExtY() + GetHalfExtZ();
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

const bool OBB3Edge::operator<(const OBB3Edge& compared) const
{
	Vector3 vec1 = m_end1.m_vert - m_end2.m_vert;
	Vector3 vec2 = compared.m_end1.m_vert - compared.m_end2.m_vert;
	return vec1.GetLength() < vec2.GetLength();
}
