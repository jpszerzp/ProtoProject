#include "Engine/Physics/3D/GJK3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/Util/DataUtils.hpp"

eGJKSimplex GJK_UpdateSimplex(std::set<Vector3>& simplex)
{
	size_t size = simplex.size();
	if (size == 0)
		return GJK_SIMPLEX_NONE;
	else if (size == 1)
		return GJK_SIMPLEX_PT;
	else if (size == 2)
		return GJK_SIMPLEX_LINE;
	else if (size == 3)
		return GJK_SIMPLEX_TRIANGLE;
	else if (size == 4)
		return GJK_SIMPLEX_TETRA;
}

Vector3 GJK_FindSupp(QuickHull* hull, const Line3& dir)
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

Mesh* GJK_CreateSimplexMesh(const std::set<Vector3>& gjk_simplex, const eGJKSimplex& stat)
{
	switch (stat)
	{
	case GJK_SIMPLEX_NONE:
		break;
	case GJK_SIMPLEX_PT:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(gjk_simplex);

		const Vector3& pt = v[0];
		Mesh* pt_mesh = Mesh::CreatePointImmediate(VERT_PCU, pt, Rgba::MEGENTA);
		return pt_mesh;
	}
	break;
	case GJK_SIMPLEX_LINE:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(gjk_simplex);

		const Vector3& pt_0 = v[0];
		const Vector3& pt_1 = v[1];

		Mesh* line_mesh = Mesh::CreateLineImmediate(VERT_PCU, pt_0, pt_1, Rgba::MEGENTA);
		return line_mesh;
	}
	break;
	case GJK_SIMPLEX_TRIANGLE:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(gjk_simplex);

		const Vector3& pt_0 = v[0];
		const Vector3& pt_1 = v[1];
		const Vector3& pt_2 = v[2];

		Mesh* triangle_mesh = Mesh::CreateTriangleImmediate(VERT_PCU, Rgba::MEGENTA, pt_0, pt_1, pt_2);
		return triangle_mesh;
	}
	break;
	case GJK_SIMPLEX_TETRA:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(gjk_simplex);

		const Vector3& pt_0 = v[0];
		const Vector3& pt_1 = v[1];
		const Vector3& pt_2 = v[2];
		const Vector3& pt_3 = v[3];

		Mesh* tetra_mesh = Mesh::CreateTetrahedronImmediate(VERT_PCU, Rgba::MEGENTA,pt_0, pt_1, pt_2, pt_3);
		return tetra_mesh;
	}
	break;
	default:
		break;
	}
}

void GJK_DrawSimplex(Mesh* gjk_simplex_mesh, const eGJKSimplex& stat)
{
	switch (stat)
	{
	case GJK_SIMPLEX_NONE:
		break;
	case GJK_SIMPLEX_PT:
		DrawPoint(gjk_simplex_mesh, 5.f);
		break;
	case GJK_SIMPLEX_LINE:
		DrawLine(gjk_simplex_mesh);
		break;
	case GJK_SIMPLEX_TRIANGLE:
		DrawTriangle(gjk_simplex_mesh);
		break;
	case GJK_SIMPLEX_TETRA:
		DrawTetrahedron(gjk_simplex_mesh);
		break;
	default:
		break;
	}
}

Vector3 GJK_FindMinNormalBase(std::set<Vector3>& simplex, eGJKSimplex& stat, float& dist)
{
	switch (stat)
	{
	case GJK_SIMPLEX_NONE:
		break;
	case GJK_SIMPLEX_PT:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(simplex);
		dist = (v[0] - Vector3::ZERO).GetLength();
		return v[0];
	}
		break;
	case GJK_SIMPLEX_LINE:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(simplex);

		const Vector3& pt_0 = v[0];
		const Vector3& pt_1 = v[1];

		Line3 simp = Line3(pt_0, pt_1);
		//Vector3 projection = ProjectPointToLine(Vector3::ZERO, simp, ext);
		//Vector3 projection = ProjectPointToLineFeature(Vector3::ZERO, simp, dist);
		Vector3 projection = GJK_SimplexReduction_PointToLine(simplex, Vector3::ZERO, simp, dist);
		stat = GJK_UpdateSimplex(simplex);			// there is a chance that the simplex got reduced, update the stat variable
		return projection;
	}
		break;
	case GJK_SIMPLEX_TRIANGLE:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(simplex);

		const Vector3& pt_0 = v[0];
		const Vector3& pt_1 = v[1];
		const Vector3& pt_2 = v[2];

		//Vector3 projection = ProjectPointToPlane(Vector3::ZERO, pt_0, pt_1, pt_2, dist);
		//Vector3 projection = ProjectPointToPlaneFeature(Vector3::ZERO, pt_0, pt_1, pt_2, dist);
		Vector3 projection = GJK_SimplexReduction_PointToTriangle(simplex, Vector3::ZERO, pt_0, pt_1, pt_2, dist);
		stat = GJK_UpdateSimplex(simplex);
		return projection;
	}
		break;
	case GJK_SIMPLEX_TETRA:
	{
		std::vector<Vector3> v = ConvertToVectorFromSet(simplex);

		const Vector3& pt_0 = v[0];
		const Vector3& pt_1 = v[1];
		const Vector3& pt_2 = v[2];
		const Vector3& pt_3 = v[3];

		//Vector3 projection = ProjectPointToTetraFeature(Vector3::ZERO, pt_0, pt_1, pt_2, pt_3, dist);
		Vector3 projection = GJK_SimplexReduction_PointToTetrahedron(simplex, Vector3::ZERO, pt_0, pt_1, pt_2, pt_3, dist);
		stat = GJK_UpdateSimplex(simplex);
		return projection;
	}
		break;
	default:
		break;
	}
}

Vector3 GJK_SimplexReduction_PointToLine(std::set<Vector3>& gjk_simplex, const Vector3& point, const Line3& line, float& dist)
{
	const Vector3& one_end = line.one;
	const Vector3& the_other_end = line.the_other;

	Vector3 one_end_to_point = point - one_end;
	Vector3 dir = (the_other_end - one_end).GetNormalized();

	float ext = DotProduct(one_end_to_point, dir);

	if (ext < 0.f)
	{
		// reduce the simplex to this one point only
		RemoveExcept(gjk_simplex, one_end);
		dist = (point - one_end).GetLength();
		return one_end;
	}

	Vector3 projection = one_end + dir * ext;
	Vector3 to_other_end = projection - the_other_end;

	ext = DotProduct(to_other_end, dir);

	if (ext < 0.f)
	{
		dist = (point-projection).GetLength();
		return projection;
	}

	RemoveExcept(gjk_simplex, the_other_end);
	dist = (point-the_other_end).GetLength();
	return the_other_end;
}

Vector3 GJK_SimplexReduction_PointToTriangle(std::set<Vector3>& gjk_simplex, const Vector3& pt, const Vector3& a, const Vector3& b, const Vector3& c, float& dist)
{
	// see p139 of real time collision detection
	// in my case, I want the distance and feature info
	Vector3 ab = b - a;
	Vector3 ac = c - a;
	Vector3 bc = c - b;

	// Voronoi related to ab
	float snom = DotProduct(pt - a, ab);
	float sdenom = DotProduct(pt - b, a - b);

	// Voronoi related to ac
	float tnom = DotProduct(pt - a, ac);
	float tdenom = DotProduct(pt - c, a - c);

	if (snom <= 0.f && tnom <= 0.f)
	{
		dist = (pt - a).GetLength();
		RemoveExcept(gjk_simplex, a);
		return a;
	}

	// Voronoi related to bc
	float unom = DotProduct(pt - b, bc);
	float udenom = DotProduct(pt - c, b - c);

	if (sdenom <= 0.f && unom <= 0.f)
	{
		dist = (pt - b).GetLength();
		RemoveExcept(gjk_simplex, b);
		return b;
	}

	if (tdenom <= 0.f && udenom <= 0.f)
	{
		dist = (pt - c).GetLength();
		RemoveExcept(gjk_simplex,c);
		return c;
	}

	// investigate edge features with barycentric methods
	// ab
	Vector3 n = ab.Cross(ac);
	Vector3 toA = a - pt;
	Vector3 toB = b - pt;
	float vc = DotProduct(n, toA.Cross(toB));
	if (vc <= 0.f && snom >= 0.f && sdenom >= 0.f)
	{
		// closest feature is edge ab
		Vector3 dev = ab * (snom / (snom + sdenom));
		Vector3 closest = a + dev;
		dist = (-toA - dev).GetLength();
		std::vector<Vector3> keep = {a, b};
		RemoveExcept(gjk_simplex, keep);

		return closest;
	}

	// bc
	Vector3 toC = c - pt;
	float va = DotProduct(n, toB.Cross(toC));
	if (va <= 0.f && unom >= 0.f && udenom >= 0.f)
	{
		// closest feature is edge bc
		Vector3 dev = bc * (unom / (unom + udenom));
		Vector3 closest = b + dev;
		dist = (-toB - dev).GetLength();
		std::vector<Vector3> keep = {b,c};
		RemoveExcept(gjk_simplex, keep);

		return closest;
	}

	// ac
	float vb = DotProduct(n, toC.Cross(toA));
	if (vb <= 0.f && tnom >= 0.f && tdenom >= 0.f)
	{
		// closest feature is edge ac
		Vector3 dev = ac * (tnom / (tnom + tdenom));
		Vector3 closest = a + dev;
		dist = (-toA - dev).GetLength();
		std::vector<Vector3> keep = {a,c};
		RemoveExcept(gjk_simplex, keep);

		return closest;
	}

	// in this case pt project within the triangle 
	float u = va / (va + vb + vc);
	float v = vb / (va + vb + vc);
	float w = 1.f - u - v;
	Vector3 closest = a * u + b * v + c * w;
	dist = (pt - closest).GetLength();

	return closest;
}

Vector3 GJK_SimplexReduction_PointToTetrahedron(std::set<Vector3>& gjk_simplex, const Vector3& pt, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, const Vector3& vert4, float& dist)
{
	float min_dist = INFINITY;
	//Vector3 closest;
	
	Vector3 a, b, c;

	Vector3 close = ProjectPointToPlaneFeature(pt, vert1, vert2, vert3, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		a = vert1;
		b = vert2;
		c = vert3;
	}

	close = ProjectPointToPlaneFeature(pt, vert2, vert3, vert4, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		a = vert2;
		b = vert3;
		c = vert4;
	}

	close = ProjectPointToPlaneFeature(pt, vert1, vert2, vert4, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		a = vert1;
		b = vert2;
		c = vert4;
	}

	close =ProjectPointToPlaneFeature(pt, vert1, vert3, vert4, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		a = vert1;
		b = vert3;
		c = vert4;
	}

	dist = min_dist;

	// see p139 of real time collision detection
	// in my case, I want the distance and feature info
	Vector3 ab = b - a;
	Vector3 ac = c - a;
	Vector3 bc = c - b;

	// Voronoi related to ab
	float snom = DotProduct(pt - a, ab);
	float sdenom = DotProduct(pt - b, a - b);

	// Voronoi related to ac
	float tnom = DotProduct(pt - a, ac);
	float tdenom = DotProduct(pt - c, a - c);

	if (snom <= 0.f && tnom <= 0.f)
	{
		//dist = (pt - a).GetLength();
		RemoveExcept(gjk_simplex, a);
		return a;
	}

	// Voronoi related to bc
	float unom = DotProduct(pt - b, bc);
	float udenom = DotProduct(pt - c, b - c);

	if (sdenom <= 0.f && unom <= 0.f)
	{
		//dist = (pt - b).GetLength();
		RemoveExcept(gjk_simplex, b);
		return b;
	}

	if (tdenom <= 0.f && udenom <= 0.f)
	{
		//dist = (pt - c).GetLength();
		RemoveExcept(gjk_simplex,c);
		return c;
	}

	// investigate edge features with barycentric methods
	// ab
	Vector3 n = ab.Cross(ac);
	Vector3 toA = a - pt;
	Vector3 toB = b - pt;
	float vc = DotProduct(n, toA.Cross(toB));
	if (vc <= 0.f && snom >= 0.f && sdenom >= 0.f)
	{
		// closest feature is edge ab
		Vector3 dev = ab * (snom / (snom + sdenom));
		Vector3 closest = a + dev;
		//dist = (-toA - dev).GetLength();
		std::vector<Vector3> keep = {a, b};
		RemoveExcept(gjk_simplex, keep);

		return closest;
	}

	// bc
	Vector3 toC = c - pt;
	float va = DotProduct(n, toB.Cross(toC));
	if (va <= 0.f && unom >= 0.f && udenom >= 0.f)
	{
		// closest feature is edge bc
		Vector3 dev = bc * (unom / (unom + udenom));
		Vector3 closest = b + dev;
		//dist = (-toB - dev).GetLength();
		std::vector<Vector3> keep = {b,c};
		RemoveExcept(gjk_simplex, keep);

		return closest;
	}

	// ac
	float vb = DotProduct(n, toC.Cross(toA));
	if (vb <= 0.f && tnom >= 0.f && tdenom >= 0.f)
	{
		// closest feature is edge ac
		Vector3 dev = ac * (tnom / (tnom + tdenom));
		Vector3 closest = a + dev;
		//dist = (-toA - dev).GetLength();
		std::vector<Vector3> keep = {a,c};
		RemoveExcept(gjk_simplex, keep);

		return closest;
	}

	// in this case pt project within the triangle 
	float u = va / (va + vb + vc);
	float v = vb / (va + vb + vc);
	float w = 1.f - u - v;
	Vector3 closest = a * u + b * v + c * w;
	//dist = (pt - closest).GetLength();
	std::vector<Vector3> keep = {a,b,c};
	RemoveExcept(gjk_simplex, keep);

	return closest;
}
