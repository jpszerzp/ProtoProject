#pragma once

#include "Engine/Core/Transform.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Physics/3D/RF/CollisionEntity.hpp"
#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Physics/3D/RF/ConvexPolyhedron.hpp"

// covariance method
struct TetrahedronBody
{
	Matrix33 m_covariance = Matrix33::ZERO;
	Vector3 m_com = Vector3::ZERO;
	float m_mass = 0.f;

	TetrahedronBody(){}
	TetrahedronBody(const Matrix33& cov, const Vector3& com, const float& mass)
		: m_covariance(cov), m_com(com), m_mass(mass){}
	~TetrahedronBody(){}

	TetrahedronBody operator+(const TetrahedronBody& other) const;
	void operator+=(const TetrahedronBody& other);

	// DEPRECATED
	Matrix33 TranslateTetrahedronWithOffsetToGetCovariance(const Vector3& offset) const;
};

class CollisionPrimitive
{
	Matrix44 m_transform_mat;

	CollisionRigidBody* m_rigid_body = nullptr;

	// for render pipeline
	Mesh* m_mesh = nullptr;
	Shader* m_shader = nullptr;
	Texture* m_texture = nullptr;
	Vector4 m_tint;

public:
	void BuildCommon(const std::string& shader = "default", const std::string& tx = "Data/Images/perspective_test.png");

	virtual void AttachToRigidBody(CollisionRigidBody* rb);

	void SetRigidBody(CollisionRigidBody* rb) { m_rigid_body = rb; }
	void SetPrimitiveTransformMat4(const Matrix44& transform) { m_transform_mat = transform; }
	void SetMesh(Mesh* mesh) { m_mesh = mesh; }
	void SetShader(Shader* shader) { m_shader = shader; }
	void SetTexture(Texture* texture) { m_texture = texture; }
	void SetTint(const Vector4& tint) { m_tint = tint; }
	void SetRigidBodyPosition(const Vector3& pos);

	void Update(float deltaTime);

	void Render(Renderer* renderer);

	CollisionRigidBody* GetRigidBody() const { return m_rigid_body; }
	float GetBodyMass() const { return m_rigid_body->GetMass(); }
	Vector3 GetBasisAndPosition(uint index) const;
	Matrix44 GetTransformMat4() const { return m_transform_mat; }
	Mesh* GetMesh() const { return m_mesh; }
	Shader* GetShader() const { return m_shader; }
	Texture* GetTexture() const { return m_texture; }
	Vector4 GetTint() const { return m_tint; }

	Vector3 GetPrimitiveRight() const;
	Vector3 GetPrimitiveUp() const;
	Vector3 GetPrimitiveForward() const;
};

class CollisionSphere : public CollisionPrimitive
{
	float m_radius;

public:
	CollisionSphere(const float& radius);

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	float GetRadius() const { return m_radius; }
};

class CollisionBox : public CollisionPrimitive
{
	Vector3 m_half_size;

public:
	CollisionBox(const Vector3& half);

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	Vector3 GetHalfSize() const { return m_half_size; }
};

class CollisionPlane : public CollisionPrimitive
{
	Vector3 m_normal;

	float m_offset;

	Vector2 m_bound;

public:
	CollisionPlane(const Vector2& bound, const Vector3& normal, const float& offset, const std::string& fp = "default", const std::string& tx = "Data/Images/perspective_test.png");
	CollisionPlane(const Vector2& bound, const std::string& mn, const Vector3& normal, const float& offset, const std::string& fp = "default", const std::string& tx = "Data/Images/perspective_test.png");

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	Vector3 GetNormal() const { return m_normal; }
	float GetOffset() const { return m_offset; }
};

class CollisionConvexObject : public CollisionPrimitive
{
	ConvexHull m_hull;

	std::vector<Vector3> m_verts;
	std::vector<Vector3> m_unit_verts;

	std::vector<ConvexPolygon> m_polygons;

	// BAD DESIGN, ONLY USED ONCE...
	// the mystery of GO contains an entity or an entity owns a GO
	// to find inertia tensor: the covariance method
	Vector3 m_initial_poi;

	// we computed it in obj in this case, so we cache it for later use when attaching rb
	Matrix33 m_initial_it;

	// record the initial mass so that it can be reused
	float m_initial_mass;

	// random ref point of covariance method (to compute inertia tensor)
	static Vector3 s_ref;

public:
	CollisionConvexObject(const ConvexHull& hull, const std::string& fp = "default", const std::string& tx = "Data/Images/perspective_test.png");

	void AttachToRigidBody(CollisionRigidBody* rb) override;

	void BuildVerticesAndPolygons(const ConvexHull& hull);
	void BuildPolygonMeshes();
	void BuildUnitVerts();

	void SortVerticesCCW(ConvexPolygon& polygon);
	void SortPolygonVerticesCCW();

	std::vector<IntVector3> GetTriangulationIndices() const;
	std::vector<TetrahedronBody> GetTetrahedronBodies(const std::vector<IntVector3>& triangle_vert_idx) const;
	TetrahedronBody GetSummedTetrahedronBody(const std::vector<TetrahedronBody>& bodies) const; 

	void AppendPolygonMesh(MeshBuilder& mb, ConvexPolygon& polygon);

	Vector3 ComputeVolumeWeightedCenter() const;
	Vector3 ComputeGeometricCentroid() const;

	Vector3 GetInitialCOM() const { return m_initial_poi; }
	Matrix33 GetInitialIT() const { return m_initial_it; }
	float GetInitialMass() const { return m_initial_mass; }
	int GetVertNum() const { return (int)m_verts.size(); }
	Vector3 GetRawVert(const int& idx) const { return m_verts[idx]; }
	Vector3 GetUnitVert(const int& idx) const { return m_unit_verts[idx]; }

	// SAT
	float ProjectVertToAxis(const Vector3& axis, const int& idx) const;
	float ProjectCenterToAxis(const Vector3& axis) const;
	void ProjectToAxisForInterval(const Vector3& axis, float& tmin, float& tmax, Vector3& vmin, Vector3& vmax) const;
	std::vector<Vector3> GetAxes() const;
};