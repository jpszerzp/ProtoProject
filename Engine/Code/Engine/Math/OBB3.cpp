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

