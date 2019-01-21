#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"

void CollisionPrimitive::AttachToRigidBody(CollisionRigidBody* rb)
{

}


void CollisionPrimitive::Update(float deltaTime)
{
	// take rigid body and integrate
	m_rigid_body->Integrate(deltaTime);

	// update primitive transform based on updated rigid body
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
