#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Math/MathUtils.hpp"

void CollisionPrimitive::BuildCommon()
{
	Renderer* rdr = Renderer::GetInstance();

	SetShader(rdr->CreateOrGetShader("default"));
	SetTexture(rdr->CreateOrGetTexture("Data/Images/perspective_test.png"));

	Vector4 tintV4;
	Rgba tint = Rgba::WHITE;
	tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);
	SetTint(tintV4);
}

void CollisionPrimitive::AttachToRigidBody(CollisionRigidBody*)
{

}


void CollisionPrimitive::Update(float deltaTime)
{
	// take rigid body and integrate
	m_rigid_body->Integrate(deltaTime);

	// calculate internal
	m_transform_mat = m_rigid_body->GetTransformMat4();
}

void CollisionPrimitive::Render(Renderer* renderer)
{
	if (m_mesh)
	{
		renderer->UseShader(m_shader);
		renderer->SetTexture2D(0, m_texture);
		renderer->SetSampler2D(0, m_texture->GetSampler());
	}

	// ubo
	renderer->m_colorData.rgba = m_tint;
	renderer->m_objectData.model = m_transform_mat;

	// draw
	renderer->DrawMesh(m_mesh);
}

Vector3 CollisionPrimitive::GetBasisAndPosition(uint index) const
{
	// 0 gives I - right, 1 gives J - up, 2 gives K - forward
	Vector3 res;

	if (index == 0)
		res = m_transform_mat.GetRight();

	else if (index == 1)
		res = m_transform_mat.GetUp();

	else if (index == 2)
		res = m_transform_mat.GetForward();

	else if (index == 3)
		res = m_transform_mat.GetTranslation();

	else 
		ASSERT_OR_DIE(false, "Invalid index for axis");

	return res;
}

Vector3 CollisionPrimitive::GetPrimitiveRight() const
{
	return m_transform_mat.GetRight();
}

Vector3 CollisionPrimitive::GetPrimitiveUp() const
{
	return m_transform_mat.GetUp();
}

Vector3 CollisionPrimitive::GetPrimitiveForward() const
{
	return m_transform_mat.GetForward();
}

void CollisionPrimitive::SetRigidBodyPosition(const Vector3& pos)
{
	m_rigid_body->SetCenter(pos);

	// position changed, update cache
	m_rigid_body->CacheData();

	// update primitive transform at last
	m_transform_mat = m_rigid_body->GetTransformMat4();
}

CollisionSphere::CollisionSphere(const float& radius)
	: m_radius(radius)
{
	Renderer* renderer = Renderer::GetInstance();

	// render data
	SetMesh(renderer->CreateOrGetMesh("sphere_pcu"));
	SetShader(renderer->CreateOrGetShader("default"));
	SetTexture(renderer->CreateOrGetTexture("Data/Images/perspective_test.png"));

	Vector4 tintV4;
	Rgba tint = Rgba::WHITE;
	tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);
	SetTint(tintV4);
}

void CollisionSphere::AttachToRigidBody(CollisionRigidBody* rb)
{
	SetRigidBody(rb);

	// set tensor info
	float factor = .4f * rb->GetMass() * m_radius * m_radius;
	Vector3 tensor_i = Vector3(factor, 0.f, 0.f);
	Vector3 tensor_j = Vector3(0.f, factor, 0.f);
	Vector3 tensor_k = Vector3(0.f, 0.f, factor);
	Matrix33 tensor = Matrix33(tensor_i, tensor_j, tensor_k);

	rb->SetTensor(tensor);
	rb->SetInvTensor(tensor.Invert());

	// so that transform mat and inv tensor world are set 
	rb->CacheData();

	// use same transform mat for primitive
	SetPrimitiveTransformMat4(rb->GetTransformMat4());
}

CollisionBox::CollisionBox(const Vector3& half)
	: m_half_size(half)
{
	Renderer* renderer = Renderer::GetInstance();

	SetMesh(renderer->CreateOrGetMesh("cube_pcu"));
	SetShader(renderer->CreateOrGetShader("default"));
	SetTexture(renderer->CreateOrGetTexture("Data/Images/perspective_test.png"));

	Vector4 tintV4;
	Rgba tint = Rgba::WHITE;
	tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);
	SetTint(tintV4);
}

void CollisionBox::AttachToRigidBody(CollisionRigidBody* rb)
{
	SetRigidBody(rb);

	// initialize box inertia tensor as needed
	const float& mass = rb->GetMass();
	float ext_x = m_half_size.x * 2.f;
	float ext_y = m_half_size.y * 2.f;
	float ext_z = m_half_size.z * 2.f;
	float factor_i = (1.f / 12.f) * mass * (ext_y * ext_y + ext_z * ext_z);
	float factor_j = (1.f / 12.f) * mass * (ext_x * ext_x + ext_z * ext_z);
	float factor_k = (1.f / 12.f) * mass * (ext_x * ext_x + ext_y * ext_y);
	Vector3 tensor_i = Vector3(factor_i, 0.f, 0.f);
	Vector3 tensor_j = Vector3(0.f, factor_j, 0.f);
	Vector3 tensor_k = Vector3(0.f, 0.f, factor_k);
	Matrix33 tensor = Matrix33(tensor_i, tensor_j, tensor_k);

	rb->SetTensor(tensor);
	rb->SetInvTensor(tensor.Invert());

	rb->CacheData();

	SetPrimitiveTransformMat4(rb->GetTransformMat4());
}

CollisionPlane::CollisionPlane(const Vector2& bound, const Vector3& normal, const float& offset)
	: m_bound(bound), m_normal(normal), m_offset(offset)
{
	Renderer* renderer = Renderer::GetInstance();

	SetMesh(renderer->CreateOrGetMesh("quad_pcu_110"));
	SetShader(renderer->CreateOrGetShader("default"));
	SetTexture(renderer->CreateOrGetTexture("Data/Images/perspective_test.png"));

	Vector4 tintV4;
	Rgba tint = Rgba::WHITE;
	tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);
	SetTint(tintV4);
}

void CollisionPlane::AttachToRigidBody(CollisionRigidBody* rb)
{
	SetRigidBody(rb);
	
	const float& mass = rb->GetMass();
	float factor_1 = m_bound.y * m_bound.y;
	float factor_2 = m_bound.x * m_bound.x;
	float factor_3 = m_bound.GetLengthSquared();					// x ^ 2 + y ^ 2
	Vector3 tensor_i = Vector3(mass * factor_1 / 12.f, 0.f, 0.f);
	Vector3 tensor_j = Vector3(0.f, mass * factor_2 / 12.f, 0.f);
	Vector3 tensor_k = Vector3(0.f, 0.f, mass * factor_3 / 12.f);
	Matrix33 tensor = Matrix33(tensor_i, tensor_j, tensor_k);

	rb->SetTensor(tensor);
	rb->SetInvTensor(tensor.Invert());

	// transform and inv tensor world prepared
	rb->CacheData();

	SetPrimitiveTransformMat4(rb->GetTransformMat4());
}

Vector3 CollisionConvexObject::s_ref = Vector3::ZERO;

CollisionConvexObject::CollisionConvexObject(const ConvexHull& hull)
	: m_hull(hull)
{
	// common initialization for general primitive object
	// shader, texture, tint...
	BuildCommon();

	// intersection verts and born polygons
	BuildVerticesAndPolygons(hull);

	// vertices (represented by indices) in above polygons may be random ordered, sort them CCW 
	SortPolygonVerticesCCW();

	// triangulated vert index for each polygon
	// polygon may NOT be a triangle (there may be more than one triangle for a polygon)
	std::vector<IntVector3> triangle_vert_idx = GetTriangulationIndices();

	// for each triangle mesh, compute a B - tetrahedron body
	// in world space (w1, w2, w3 are in world space)
	std::vector<TetrahedronBody> tBodies = GetTetrahedronBodies(triangle_vert_idx);

	// accumulate tbody to get info for total body
	// in world space
	TetrahedronBody total_body = GetSummedTetrahedronBody(tBodies);

	// now we get total covariance against ref point, we adjust it to be against com; todo: total_body.m_com - ref?
	// c_total is in local space, but total_body in world space still
	Matrix33 c_total = TranslateCovariance(total_body.m_covariance, total_body.m_com, total_body.m_mass, s_ref - total_body.m_com);

	// with adjusted total covariance the inertia tensor can be derived
	// m_it in local space
	m_initial_it = GetInertiaTensorFromCovariance(c_total);

	// mass
	m_initial_mass = total_body.m_mass;

	// "point of interest" - candidate of com
	m_initial_poi = total_body.m_com;

	// with polygon verts sorted, build meshes out of them
	BuildPolygonMeshes();
}

void CollisionConvexObject::AttachToRigidBody(CollisionRigidBody* rb)
{
	SetRigidBody(rb);

	rb->SetTensor(m_initial_it);
	rb->SetInvTensor(m_initial_it.Invert());

	rb->CacheData();

	SetPrimitiveTransformMat4(rb->GetTransformMat4());
}

void CollisionConvexObject::BuildVerticesAndPolygons(const ConvexHull& hull)
{
	// list of planes
	const std::vector<Plane>& planes = hull.GetPlaneCopies();

	uint plane_num = planes.size();
	m_polygons.resize(plane_num);
	for (int i = 0; i < plane_num; ++i)
	{
		const Plane& plane_ref = planes[i];
		const Vector3& n = plane_ref.GetNormal();

		ConvexPolygon polygon = ConvexPolygon(n);
		m_polygons[i] = polygon;
	}

	for (int i = 0; i <= plane_num - 3; ++i)
	{
		for (int j = i; j <= plane_num - 2; ++j)
		{
			for (int k = j; k <= plane_num - 1; ++k)
			{
				if (i != j && j != k && i != k)
				{
					bool legal = true;

					const Plane& p_i = planes[i];
					const Plane& p_j = planes[j];
					const Plane& p_k = planes[k];

					Vector3 intersection;
					bool intersected = ComputePlaneIntersectionPoint(p_i, p_j, p_k, intersection);

					if (intersected)
					{
						// if this point has existed?
						// if yes, go to next check of planes directly
						bool found = std::find(m_verts.begin(), m_verts.end(), intersection) != m_verts.end();
						if (found)
							continue;

						// for this new intersection point, see if it is valid
						// if it is outside ANY plane, it is NOT valid
						for (int m = 0; m <= plane_num - 1; ++m)
						{
							const Plane& subject_plane = planes[m];

							if (IsPointOutwardPlane(intersection, subject_plane))
							{
								legal = false;
								break;		
							}
						}

						if (legal)
						{
							m_verts.push_back(intersection);

							int this_idx = m_verts.size() - 1;

							m_polygons[i].AddVertexIndex(this_idx);
							m_polygons[j].AddVertexIndex(this_idx);
							m_polygons[k].AddVertexIndex(this_idx);
						}
					}
				}
			}
		}
	}
	// post-condition:
	// convex hull has all planes
	// m_verts has all verts
	// m_polygons has all polys recording all points and correct normal
}

void CollisionConvexObject::BuildPolygonMeshes()
{
	MeshBuilder mb = MeshBuilder();

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	for (int i = 0; i < m_polygons.size(); ++i)
	{
		AppendPolygonMesh(mb, m_polygons[i]);
	}

	mb.End();

	Mesh* gpu_mesh = mb.CreateMesh(VERT_PCU, DRAW_TRIANGLE);
	SetMesh(gpu_mesh);
}

void CollisionConvexObject::SortVerticesCCW(ConvexPolygon& polygon)
{
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
					float cos_theta = DotProduct(to_wind_normalized, to_candidate_normalized);
					float ang_rad = acos(cos_theta);		// 0 to pi

					if (ang_rad < min_angle)
					{
						min_angle = ang_rad;

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

void CollisionConvexObject::SortPolygonVerticesCCW()
{
	for (int i = 0; i < m_polygons.size(); ++i)
	{
		ConvexPolygon& polygon = m_polygons[i];
		SortVerticesCCW(polygon);
	}
}

std::vector<IntVector3> CollisionConvexObject::GetTriangulationIndices() const
{
	std::vector<IntVector3> triangle_vert_idx;
	for (int i = 0; i < m_polygons.size(); ++i)
	{
		// copy of this polygon
		const ConvexPolygon& polygon = m_polygons[i];

		// vert indices of this polygon
		const std::vector<int>& polygon_vert_indices = polygon.m_vert_idx;

		for (int j = 2; j < polygon_vert_indices.size(); ++j)
		{
			IntVector3 indices = IntVector3(polygon_vert_indices[0], 
				polygon_vert_indices[j - 1], polygon_vert_indices[j]);

			triangle_vert_idx.push_back(indices);
		}
	}

	return triangle_vert_idx;
}

std::vector<TetrahedronBody> CollisionConvexObject::GetTetrahedronBodies(const std::vector<IntVector3>& triangle_vert_idx) const
{
	std::vector<TetrahedronBody> tBodies;
	const Vector3& w0 = s_ref;			// "random" ref point
	Matrix33 c_canonical = GetCanonicalTetrahedronCovariance();
	for (int i = 0; i < triangle_vert_idx.size(); ++i)
	{
		const IntVector3& indices = triangle_vert_idx[i];

		const Vector3& w1 = m_verts[indices.x];
		const Vector3& w2 = m_verts[indices.y];
		const Vector3& w3 = m_verts[indices.z];

		Matrix33 affine = Matrix33(w1 - w0, w2 - w0, w3 - w0);

		// C - covariance
		// todo: use abs value or not?
		float abs_det_a = abs(affine.GetDeterminant());
		//float abs_det_a = affine.GetDeterminant();
		const Matrix33& affine_transpose = affine.Transpose();
		Matrix33 c1 = Matrix33::IDENTITY;
		c1.Append(affine);
		c1.Append(c_canonical);
		c1.Append(affine_transpose);
		c1 *= abs_det_a;

		// mass, from volume and density
		static const float tDensity = .5f;
		float tVolume = (1.f / 6.f) * abs_det_a;		// todo: use abs or not?
		float tMass = tDensity * tVolume;

		// since b is a tetrahedron, com is centroid
		Vector3 com = (w0 + w1 + w2 + w3) / 4.f;		

		TetrahedronBody tBody = TetrahedronBody(c1, com, tMass);

		tBodies.push_back(tBody);
	}

	return tBodies;
}

TetrahedronBody CollisionConvexObject::GetSummedTetrahedronBody(const std::vector<TetrahedronBody>& bodies) const
{
	TetrahedronBody total_body;
	for (int i = 0; i < bodies.size(); ++i)
	{
		total_body += bodies[i];
	}

	return total_body;
}

void CollisionConvexObject::AppendPolygonMesh(MeshBuilder& mb, ConvexPolygon& polygon)
{
	// triangulate
	for (int i = 2; i < polygon.m_vert_idx.size(); ++i)
	{
		Vector3 vert0 = m_verts[polygon.m_vert_idx[0]];
		Vector3 vert1 = m_verts[polygon.m_vert_idx[i - 1]];
		Vector3 vert2 = m_verts[polygon.m_vert_idx[i]];

		// to use transform, build mesh in local space
		// vert0/1/2 turns to direction (not position) after thiss
		vert0 -= m_initial_poi;
		vert1 -= m_initial_poi;
		vert2 -= m_initial_poi;

		mb.SetNormal(polygon.m_normal);

		mb.SetUV(Vector2(0.f, 0.f));
		uint idx = mb.PushVertex(vert0);

		mb.SetUV(Vector2(1.f, 0.f));
		mb.PushVertex(vert1);

		mb.SetUV(Vector2(1.f, 1.f));
		mb.PushVertex(vert2);

		mb.AddTriangle(idx, idx + 1, idx + 2);
	}
}

Vector3 CollisionConvexObject::ComputeVolumeWeightedCenter() const
{
	Vector3 volume_weighted_center = Vector3::ZERO;

	// tetrahedron
	if (m_polygons.size() == 4)
	{
		const ConvexPolygon& p0 = m_polygons[0];
		const Vector3& base_centroid = GetPolygonCentroid(m_verts, p0);
		const Vector3& geo_centroid = ComputeGeometricCentroid();

		volume_weighted_center = base_centroid * .75f + geo_centroid * .25f;
	}

	return volume_weighted_center;
}

Vector3 CollisionConvexObject::ComputeGeometricCentroid() const
{
	Vector3 centroid = Vector3::ZERO;

	for (int i = 0; i < m_verts.size(); ++i)
	{
		centroid += m_verts[i];
	}

	uint vert_num = m_verts.size();

	centroid /= vert_num;

	return centroid;
}

// pre: axis is thru origin
float CollisionConvexObject::ProjectVertToAxis(const Vector3& axis, const int& idx) const
{
	const Vector3& vert = m_verts[idx];

	const Vector3& n_axis = axis.GetNormalized();
	float ext = DotProduct(vert, n_axis);

	return ext;
}

float CollisionConvexObject::ProjectCenterToAxis(const Vector3& axis) const
{
	const Vector3& n_axis = axis.GetNormalized();
	
	const Vector3& center = GetRigidBody()->GetCenter();

	float ext = DotProduct(center, n_axis);

	return ext;
}

void CollisionConvexObject::ProjectToAxisForInterval(const Vector3& axis, float& tmin, float& tmax, Vector3& vmin, Vector3& vmax) const
{
	float min = FLT_MAX;
	float max = FLT_MIN;
	Vector3 min_v = Vector3(FLT_MAX);
	Vector3 max_v = Vector3(FLT_MIN);

	for (int i = 0; i < m_verts.size(); ++i)
	{
		float ext = ProjectVertToAxis(axis, i);

		if (ext < min)
		{
			min = ext;
			min_v = m_verts[i];
		}

		if (ext > max)
		{
			max = ext;
			max_v = m_verts[i];
		}
	}

	tmin = min;
	tmax = max;

	vmin = min_v;
	vmax = max_v;
}

std::vector<Vector3> CollisionConvexObject::GetAxes() const
{
	std::vector<Vector3> axes;

	for (int i = 0; i < m_polygons.size(); ++i)
	{
		const ConvexPolygon& poly = m_polygons[i];
		const Vector3& n = poly.m_normal;

		axes.push_back(n);
	}

	return axes;
}

// TETRAHEDRONBODY
TetrahedronBody TetrahedronBody::operator+(const TetrahedronBody& other) const
{
	TetrahedronBody sum_body;
	
	sum_body.m_covariance = m_covariance + other.m_covariance;
	sum_body.m_mass = m_mass + other.m_mass;
	sum_body.m_com = (m_com * m_mass + other.m_com * other.m_mass) / (m_mass + other.m_mass);

	return sum_body;
}

void TetrahedronBody::operator+=(const TetrahedronBody& other)
{
	m_covariance += other.m_covariance;
	m_mass += other.m_mass;
	m_com = (m_com * m_mass + other.m_com * other.m_mass) / (m_mass + other.m_mass);
}

Matrix33 TetrahedronBody::TranslateTetrahedronWithOffsetToGetCovariance(const Vector3& offset) const
{
	float dot1 = DotProduct(offset, m_com);
	float dot2 = DotProduct(m_com, offset);
	float dot3 = DotProduct(offset, offset);

	return (m_covariance + m_mass * (dot1 + dot2 + dot3));
}
