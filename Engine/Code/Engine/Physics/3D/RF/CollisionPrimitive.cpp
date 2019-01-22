#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

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

	// draw debug basis
	//const Vector3& start = m_rigid_body->GetCenter();

	//Vector3 right_dir = m_transform_mat.GetRight();
	//right_dir.Normalize();

	//Vector3 up_dir = m_transform_mat.GetUp();
	//up_dir.Normalize();

	//Vector3 forward_dir = m_transform_mat.GetForward();
	//forward_dir.Normalize();

	//Vector3 right_end = start + right_dir * 3.f;
	//Vector3 up_end = start + up_dir * 3.f;
	//Vector3 forward_end = start + forward_dir * 3.f;

	//DebugRenderLine(.05f, start, right_end, 3.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
	//DebugRenderLine(.05f, start, up_end, 3.f, Rgba::GREEN, Rgba::GREEN, DEBUG_RENDER_USE_DEPTH);
	//DebugRenderLine(.05f, start, forward_end, 3.f, Rgba::BLUE, Rgba::BLUE, DEBUG_RENDER_USE_DEPTH);
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
