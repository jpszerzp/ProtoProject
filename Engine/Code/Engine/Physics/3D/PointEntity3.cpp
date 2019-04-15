#include "Engine/Physics/3D/PointEntity3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

PointEntity3::PointEntity3(const Particle& primitive, eMoveStatus moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_PARTICLE;
	
	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_position;
	
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3(primitive.m_size);
	m_entityTransform = Transform(m_center, rot, scale);
	
	//m_sphereBoundTransform = m_entityTransform;
	m_boxBoundTransform = m_entityTransform;
	
	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
	}
	else
		m_massData.m_invMass = 0.f;
	
	//m_boundSphere = Sphere3(m_center, m_primitive.m_size / 2.f);
	m_boundSphere = BoundingSphere(m_center, primitive.m_size / 2.f);
	//m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = m_entityTransform;
	
	Vector3 boundBoxMin = m_center - scale / 2.f;
	Vector3 boundBoxMax = m_center + scale / 2.f;
	m_boundBox = AABB3(boundBoxMin, boundBoxMax);
	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);
}

PointEntity3::~PointEntity3()
{

}

void PointEntity3::UpdatePrimitives()
{
	m_boundSphere.SetCenter(m_center);
	m_boundBox.SetCenter(m_center);

	m_primitive.SetCenter(m_center);
}

void PointEntity3::Render(Renderer* renderer)
{
	// bound box
	if (m_drawBoundBox)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->m_objectData.model = m_boxBoundTransform.GetWorldMatrix();

		Vector4 colorV4;
		Rgba color = Rgba::RED;
		color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);
		renderer->m_colorData.rgba = colorV4;
		renderer->SetColorUBO(shader->GetShaderProgram()->GetHandle());

		glLineWidth(1.f);
		renderer->DrawMesh(m_boxBoundMesh);
	}

	if (m_drawBoundSphere)
		m_boundSphere.DrawBound(renderer);
}

void PointEntity3::Translate(Vector3 translation)
{
	m_center += translation;

	// update transform
	m_entityTransform.SetLocalPosition(m_center);
	m_boundSphere.m_transform.SetLocalPosition(m_center);
	m_boxBoundTransform.SetLocalPosition(m_center);

	// update primitive
	m_primitive.Translate(translation);
	m_boundBox.Translate(translation);
	m_boundSphere.Translate(translation);
}


void PointEntity3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}
