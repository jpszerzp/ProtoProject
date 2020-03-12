#include "Engine/Physics/3D/RF/ConvexPolyhedron.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Math/MathUtils.hpp"

//Vector3 GetPolygonCentroid(const std::vector<Vector3>& verts, const ConvexPolygon& polygon)
//{
//	Vector3 centroid = Vector3::ZERO;
//
//	const std::vector<int>& vert_indices = polygon.m_vert_idx;
//
//	for (int i = 0; i < vert_indices.size(); ++i)
//	{
//		centroid += verts[vert_indices[i]];
//	}
//
//	uint vert_num = vert_indices.size();
//
//	centroid /= vert_num;
//
//	return centroid;
//}

int GetPolygonFirstVert(const std::vector<Vector3>& verts, const ConvexPolygon& polygon, Vector3& pos)
{
	int idx = polygon.m_vert_idx[0];
	pos = verts[idx];
	return idx;
}

ConvexPolyhedron::ConvexPolyhedron(ConvexHull* hull)
	: m_hull(hull)
{
	// try to populate vert, edge and planes
	ASSERT_OR_DIE(m_hull->GetPlaneNum() >= 4, "Convex hull has less than 4 planes, cannot form the shape");

	// initialzation will use the first four planes to form the hull
	const Plane& p1 = m_hull->GetPlane(0);
	const Plane& p2 = m_hull->GetPlane(1);
	const Plane& p3 = m_hull->GetPlane(2);
	const Plane& p4 = m_hull->GetPlane(3);

	// samples
	/*
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 0.f, 0.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(5.f, 0.f, 0.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-5.f, 0.f, 0.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 0.f, -3.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 0.f, -6.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 0.f, 2.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 0.f, 4.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, -3.f, 0.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, -4.5f, 0.f)   , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 2.f, 0.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, 4.f, 0.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(7.f, 0.f, 0.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-7.f, 0.f, 0.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(3.f, 1.5f, 2.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(3.f, -1.5f, 2.f)   , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-3.f, -1.5f, 2.f)  , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-3.f, 1.5f, 2.f)   , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(3.f, 2.f, 3.f)     , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(4.f, -2.f, 3.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-4.f, -2.f, 3.f)   , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-3.f, 2.f, 3.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(5.f, -2.f, -5.f)   , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(5.f, 2.f, -4.f)    , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-5.f, 2.f, -4.f)   , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-5.f, -2.f, -5.f)  , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(8.f, -3.3f, -7.f)  , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-8.f, -3.3f, -7.f) , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(13.f, -4.1f, -9.f) , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(-13.f, -4.1f, -9.f), Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	DebugRenderPoint(10000.f, 10.f, Vector3(0.f, -2.5f, 7.5f)  , Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	*/
	//m_hull->AddSample( Vector3(0.f, 0.f, 0.f)      );
	//m_hull->AddSample( Vector3(5.f, 0.f, 0.f)      );
	//m_hull->AddSample( Vector3(-5.f, 0.f, 0.f)     );
	//m_hull->AddSample( Vector3(0.f, 0.f, -3.f)     );
	//m_hull->AddSample( Vector3(0.f, 0.f, -6.f)     );
	//m_hull->AddSample( Vector3(0.f, 0.f, 2.f)      );
	//m_hull->AddSample( Vector3(0.f, 0.f, 4.f)      );
	//m_hull->AddSample( Vector3(0.f, -3.f, 0.f)     );
	//m_hull->AddSample( Vector3(0.f, -4.5f, 0.f)    );
	//m_hull->AddSample( Vector3(0.f, 2.f, 0.f)      );
	//m_hull->AddSample( Vector3(0.f, 4.f, 0.f)      );
	//m_hull->AddSample( Vector3(7.f, 0.f, 0.f)      );
	//m_hull->AddSample( Vector3(-7.f, 0.f, 0.f)     );
	//m_hull->AddSample( Vector3(3.f, 1.5f, 2.f)     );
	//m_hull->AddSample( Vector3(3.f, -1.5f, 2.f)    );
	//m_hull->AddSample( Vector3(-3.f, -1.5f, 2.f)   );
	//m_hull->AddSample( Vector3(-3.f, 1.5f, 2.f)    );
	//m_hull->AddSample( Vector3(3.f, 2.f, 3.f)      );
	//m_hull->AddSample( Vector3(4.f, -2.f, 3.f)     );
	//m_hull->AddSample( Vector3(-4.f, -2.f, 3.f)    );
	//m_hull->AddSample( Vector3(-3.f, 2.f, 3.f)     );
	//m_hull->AddSample( Vector3(5.f, -2.f, -5.f)    );
	//m_hull->AddSample( Vector3(5.f, 2.f, -4.f)     );
	//m_hull->AddSample( Vector3(-5.f, 2.f, -4.f)    );
	//m_hull->AddSample( Vector3(-5.f, -2.f, -5.f)   );
	//m_hull->AddSample( Vector3(8.f, -3.3f, -7.f)   );
	//m_hull->AddSample( Vector3(-8.f, -3.3f, -7.f)  );
	//m_hull->AddSample( Vector3(13.f, -4.1f, -9.f)  );
	//m_hull->AddSample( Vector3(-13.f, -4.1f, -9.f) );
	//m_hull->AddSample( Vector3(0.f, -2.5f, 7.5f)   );

	// p123, (0, -5, 13)
	const Vector3& p123 = ComputeIntersectionPoint(p1, p2, p3);
	m_verts.push_back(p123);		// 0	

	// p124, (-25, -5, -12)
	const Vector3& p124 = ComputeIntersectionPoint(p1, p2, p4);
	m_verts.push_back(p124);		// 1

	// p134, (25, -5, -12)
	const Vector3& p134 = ComputeIntersectionPoint(p1, p3, p4);
	m_verts.push_back(p134);		// 2

	// p234, (0, 7, 0.7)
	const Vector3& p234 = ComputeIntersectionPoint(p2, p3, p4);
	m_verts.push_back(p234);		// 3

	TODO("Later consider cases with more than 4 faces");

	//DebugRenderPoint(10000.f, 10.f, p123, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	//DebugRenderPoint(10000.f, 10.f, p124, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	//DebugRenderPoint(10000.f, 10.f, p134, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	//DebugRenderPoint(10000.f, 10.f, p234, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);

	// f1 unsorted
	std::vector<int> f1;
	f1.push_back(0);	// p123
	f1.push_back(1);	// p124
	f1.push_back(2);	// p134
	ConvexPolygon cp1 = ConvexPolygon(f1, p1.m_normal);

	// f2 unsorted
	std::vector<int> f2;
	f2.push_back(0);	// p123
	f2.push_back(1);	// p124
	f2.push_back(3);	// p234
	ConvexPolygon cp2 = ConvexPolygon(f2, p2.m_normal);

	// f3 unsorted
	std::vector<int> f3;
	f3.push_back(0);	// p123
	f3.push_back(2);	// p134
	f3.push_back(3);	// p234
	ConvexPolygon cp3 = ConvexPolygon(f3, p3.m_normal);

	// f4 unsorted
	std::vector<int> f4;
	f4.push_back(1);	// p124
	f4.push_back(2);	// p134
	f4.push_back(3);	// p234
	ConvexPolygon cp4 = ConvexPolygon(f4, p4.m_normal);

	SortVerticesCCW(cp1);
	SortVerticesCCW(cp2);
	SortVerticesCCW(cp3);
	SortVerticesCCW(cp4);

	// say all CCW sorted verts are stored, create mesh for them
	m_cpu_mesh = MeshBuilder();

	m_cpu_mesh.Begin(DRAW_TRIANGLE, true);
	m_cpu_mesh.SetColor(Rgba::WHITE);

	AppendPolygonMesh(cp1);
	AppendPolygonMesh(cp2);
	AppendPolygonMesh(cp3);
	AppendPolygonMesh(cp4);

	m_cpu_mesh.End();

	m_gpu_mesh = m_cpu_mesh.CreateMesh(VERT_PCU, DRAW_TRIANGLE);
}

ConvexPolyhedron::~ConvexPolyhedron()
{
	delete m_hull;
	m_hull = nullptr;
}

void ConvexPolyhedron::Render(Renderer* rdr)
{
	if (m_gpu_mesh)
	{
		Shader* shader = rdr->CreateOrGetShader("default");
		Texture* tex = rdr->CreateOrGetTexture("Data/Images/perspective_test.png");

		Vector4 tintV4;
		Rgba tint = Rgba::WHITE;
		tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);

		rdr->UseShader(shader);
		rdr->SetTexture2D(0, tex);
		rdr->SetSampler2D(0, tex->GetSampler());

		rdr->m_colorData.rgba = tintV4;

		// model, in trs
		Matrix44 model = Matrix44::IDENTITY;
		Matrix44 t = Matrix44::MakeTranslation3D(pos);
		Matrix44 r = Matrix44::IDENTITY;
		r.RotateZ(rot.z);
		r.RotateX(rot.x);
		r.RotateY(rot.y);
		Matrix44 s = Matrix44::MakeScale3D(scale.x, scale.y, scale.z);
		model.Append(t);
		model.Append(r);
		model.Append(s);
		rdr->m_objectData.model = model;

		rdr->DrawMesh(m_gpu_mesh);
	}
}

Vector3 ConvexPolyhedron::ComputeIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3)
{
	const Vector3& n1 = p1.GetNormal();
	const Vector3& n2 = p2.GetNormal();
	const Vector3& n3 = p3.GetNormal();
	
	const float& d1 = p1.GetOffset();
	const float& d2 = p2.GetOffset();
	const float& d3 = p3.GetOffset();
	
	Vector3 ns_i = Vector3(n1.x, n2.x, n3.x);
	Vector3 ns_j = Vector3(n1.y, n2.y, n3.y);
	Vector3 ns_k = Vector3(n1.z, n2.z, n3.z);
	Matrix33 ns = Matrix33(ns_i, ns_j, ns_k);
	
	Vector3 dv = Vector3(d1, d2, d3);
	
	const Matrix33& ns_inv = ns.GetInverse();
	
	return ns_inv * dv;
}

void ConvexPolyhedron::SortVerticesCCW(ConvexPolygon& polygon)
{
	// f1 centroid
	std::vector<int> sorted_idx;

	const Vector3& centroid = GetPolygonCentroid(m_verts, polygon);

	// after centroid it is to start at a point in face
	Vector3 wind_pos;
	int wind_idx = GetPolygonFirstVert(m_verts, polygon, wind_pos);
	sorted_idx.push_back(wind_idx);

	// plane normal cross centroid->wind_start is the CCW filter plane
	const Vector3& pl_normal = polygon.m_normal;

	// start point of wind
	// (can probably optimize the following with do-while)
	Vector3 to_wind_vert = wind_pos - centroid;
	Vector3 fileter_pl_normal = to_wind_vert.Cross(pl_normal);		// not normalized

	// keep the vert set small, or this becomes very bad for performance
	while (sorted_idx.size() != polygon.GetVertNum())
	{
		float min_angle = FLT_MAX;
		int win_index = -1;

		for (int i = 0; i < polygon.m_vert_idx.size(); ++i)
		{
			int vert_idx = polygon.m_vert_idx[i];

			// if this vert has been pushed
			std::vector<int>::iterator it = std::find(sorted_idx.begin(), sorted_idx.end(), vert_idx);
			bool visited = (it != sorted_idx.end());

			// if not this point is a candidate
			if (!visited)
			{
				const Vector3& candidate = m_verts[vert_idx];
				const Vector3& to_candidate = candidate - centroid;

				// if this candidate is on correct side of filter plane
				float dot = DotProduct(to_candidate, fileter_pl_normal);
				if (dot > 0.f)
				{
					// if its angle to wind point is smaller, update that
					const Vector3& to_wind_normalized = to_wind_vert.GetNormalized();
					const Vector3& to_candidate_normalized = to_candidate.GetNormalized();
					float theta = DotProduct(to_wind_normalized, to_candidate_normalized);

					if (theta < min_angle)
					{
						min_angle = theta;

						win_index = vert_idx;
					}
				}
			}
		}

		wind_pos = m_verts[win_index];
		sorted_idx.push_back(win_index);

		to_wind_vert = wind_pos - centroid;
		fileter_pl_normal = to_wind_vert.Cross(pl_normal);	
	}

	// at the end, sorted_index contains sorted CCW vert index
	polygon.m_vert_idx = sorted_idx;

	// keep the invariant before going into next face
	sorted_idx.clear();
}

void ConvexPolyhedron::AppendPolygonMesh(ConvexPolygon& polygon)
{
	// triangulate
	for (int i = 2; i < polygon.m_vert_idx.size(); ++i)
	{
		const Vector3& vert0 = m_verts[polygon.m_vert_idx[0]];
		const Vector3& vert1 = m_verts[polygon.m_vert_idx[i - 1]];
		const Vector3& vert2 = m_verts[polygon.m_vert_idx[i]];

		m_cpu_mesh.SetNormal(polygon.m_normal);

		m_cpu_mesh.SetUV(Vector2(0.f, 0.f));
		uint idx = m_cpu_mesh.PushVertex(vert0);

		m_cpu_mesh.SetUV(Vector2(1.f, 0.f));
		m_cpu_mesh.PushVertex(vert1);

		m_cpu_mesh.SetUV(Vector2(1.f, 1.f));
		m_cpu_mesh.PushVertex(vert2);

		m_cpu_mesh.AddTriangle(idx, idx + 1, idx + 2);
	}
}
