#include "Engine/Physics/3D/PointEntity3.hpp"

PointEntity3::PointEntity3(const Particle& primitive, bool isConst /*= false*/)
{
	m_primitive = primitive;
	m_const = isConst;

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_position;

	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3(primitive.m_size);
	m_entityTransform = Transform(m_center, rot, scale);

	m_sphereBoundTransform = m_entityTransform;
	m_boxBoundTransform = m_entityTransform;

	if (!m_const)
	{
		m_massData.m_mass = 1.f;
		//m_massData.m_mass = 100.f;
		m_massData.m_inertia = .01f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
		m_massData.m_invInertia = 1.f / m_massData.m_inertia;
	}
	else
	{
		m_massData.m_mass = 0.f;
		m_massData.m_inertia = 0.f;
	}

	if (m_massData.m_mass == 0.f)
	{
		m_massData.m_invMass = 0.f;			// infinite mass
	}
	if (m_massData.m_inertia == 0.f)
	{	
		m_massData.m_invInertia = 0.f;		// infinite inertia
	}

	m_boundSphere = Sphere3(m_center, m_primitive.m_size / 2.f);
	m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);

	Vector3 boundBoxMin = m_center - scale / 2.f;
	Vector3 boundBoxMax = m_center + scale / 2.f;
	m_boundBox = AABB3(boundBoxMin, boundBoxMax);
	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);
}

PointEntity3::~PointEntity3()
{

}

void PointEntity3::UpdateEntityPrimitive()
{
	m_primitive.m_position = m_center;
}

void PointEntity3::Update(float deltaTime)
{

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
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->m_objectData.model = m_sphereBoundTransform.GetWorldMatrix();

		Vector4 colorV4;
		Rgba color = Rgba::BLUE;
		color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);
		renderer->m_colorData.rgba = colorV4;
		renderer->SetColorUBO(shader->GetShaderProgram()->GetHandle());

		glLineWidth(1.f);
		renderer->DrawMesh(m_sphereBoundMesh);
	}
}

void PointEntity3::Translate(Vector3 translation)
{
	m_center += translation;

	// update transform
	m_entityTransform.SetLocalPosition(m_center);
	m_sphereBoundTransform.SetLocalPosition(m_center);
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
