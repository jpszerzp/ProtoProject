#include "Engine/Physics/3D/GJK3Simplex.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <assert.h>
#include <vector>

void GJK_GenerateFaceNormal(Vector3& norm1, Vector3& norm2, const GJK3SimplexFace& face)
{
	const Vector3& v1 = face.m_verts[0];
	const Vector3& v2 = face.m_verts[1];
	const Vector3& v3 = face.m_verts[2];

	Vector3 side1 = v2 - v1;
	Vector3 side2 = v3 - v1;

	norm1 = side1.Cross(side2).GetNormalized();
	norm2 = side2.Cross(side1).GetNormalized();
}


void GJK_GenerateOutboundFaceNormal(const Vector3& external, GJK3SimplexFace& face)
{
	Vector3 norm1;
	Vector3 norm2;
	GJK_GenerateFaceNormal(norm1, norm2, face);

	// decide on initial face normal based on v4
	Vector3 towardExternal = external - face.m_verts[0];
	float ext1 = DotProduct(towardExternal, norm1);
	Vector3 norm = (ext1 < 0.f) ? norm1 : norm2;
	face.m_normal = norm;
}

GJK3SimplexFace::GJK3SimplexFace(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	m_verts[0] = v1;
	m_verts[1] = v2;
	m_verts[2] = v3;
}


bool GJK3SimplexFace::IsPointBehindFace(const Vector3& pt) const
{
	const Vector3& v = m_verts[0];
	Vector3 disp = pt - v;
	return (DotProduct(disp, m_normal) < 0.f);
}

GJK3SimplexTetra::GJK3SimplexTetra(const std::set<Vector3>& verts)
{
	assert(verts.size() == 4);
	std::vector<Vector3> v = ConvertToVectorFromSet(verts);
	const Vector3& v1 = v[0];
	const Vector3& v2 = v[1];
	const Vector3& v3 = v[2];
	const Vector3& v4 = v[3];

	// first 3 verts will form the face
	GJK3SimplexFace f1 = GJK3SimplexFace(v1, v2, v3);

	GJK_GenerateOutboundFaceNormal(v4, f1);

	// generate other faces
	GJK3SimplexFace f2 = GJK3SimplexFace(v2, v1, v4);
	GJK3SimplexFace f3 = GJK3SimplexFace(v1, v3, v4);
	GJK3SimplexFace f4 = GJK3SimplexFace(v2, v4, v3);

	GJK_GenerateOutboundFaceNormal(v3, f2);
	GJK_GenerateOutboundFaceNormal(v2, f3);
	GJK_GenerateOutboundFaceNormal(v1, f4);

	m_faces[0] = f1;
	m_faces[1] = f2;
	m_faces[2] = f3;
	m_faces[3] = f4;
}

bool GJK3SimplexTetra::IsPointInTetra(const Vector3& pt)
{
	const GJK3SimplexFace& f1 = m_faces[0];
	const GJK3SimplexFace& f2 = m_faces[1];
	const GJK3SimplexFace& f3 = m_faces[2];
	const GJK3SimplexFace& f4 = m_faces[3];

	bool behind1 = f1.IsPointBehindFace(pt);
	bool behind2 = f2.IsPointBehindFace(pt);
	bool behind3 = f3.IsPointBehindFace(pt);
	bool behind4 = f4.IsPointBehindFace(pt);

	return (behind1 && behind2 && behind3 && behind4);
}
