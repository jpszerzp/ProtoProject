#include "Engine/Physics/3D/SphereEntity3.hpp"

//SphereEntity3::SphereEntity3(const Sphere3& primitive, bool isConst /*= false*/)
//{
//	m_primitive = primitive;
//	m_const = isConst;
//
//	m_linearVelocity = Vector3::ZERO;
//	m_center = primitive.m_center;
//
//	Vector3 rot = Vector3::ZERO;
//	float radius = primitive.m_radius;
//	Vector3 scale = Vector3(radius);
//	m_entityTransform = Transform(m_center, rot, scale);
//
//	float boundBoxDim = 2.f * radius;
//	Vector3 boundBoxScale = Vector3(boundBoxDim);
//	m_sphereBoundTransform = m_entityTransform;
//	m_boxBoundTransform = Transform(m_center, rot, boundBoxScale);
//
//	if (!m_const)
//	{
//		m_massData.m_mass = 1.f;
//		//m_massData.m_inertia = .01f;
//		m_massData.m_invMass = 1.f / m_massData.m_mass;
//		//m_massData.m_invInertia = 1.f / m_massData.m_inertia;
//	}
//	else
//	{
//		m_massData.m_mass = 0.f;
//		//m_massData.m_inertia = 0.f;
//	}
//
//	if (m_massData.m_mass == 0.f)
//	{
//		m_massData.m_invMass = 0.f;
//	}
//	//if (m_massData.m_inertia == 0.f)
//	//{
//	//	m_massData.m_invInertia = 0.f;
//	//}
//
//	//m_boundSphere = primitive;
//	m_boundSphere = BoundingSphere(m_center, m_primitive.m_radius);
//	m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
//
//	Vector3 boundBoxMin = m_center - boundBoxScale / 2.f;
//	Vector3 boundBoxMax = m_center + boundBoxScale / 2.f;
//	m_boundBox = AABB3(boundBoxMin, boundBoxMax);
//	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);
//}

SphereEntity3::SphereEntity3(const Sphere3& primitive, eMoveStatus moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_PARTICLE;

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_center;

	Vector3 rot = Vector3::ZERO;
	float radius = primitive.m_radius;
	Vector3 scale = Vector3(radius);
	m_entityTransform = Transform(m_center, rot, scale);

	float boundBoxDim = 2.f * radius;
	Vector3 boundBoxScale = Vector3(boundBoxDim);
	m_sphereBoundTransform = m_entityTransform;
	m_boxBoundTransform = Transform(m_center, rot, boundBoxScale);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
	}
	else
		m_massData.m_invMass = 0.f;

	//m_boundSphere = primitive;
	m_boundSphere = BoundingSphere(m_center, m_primitive.m_radius);
	m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);

	Vector3 boundBoxMin = m_center - boundBoxScale / 2.f;
	Vector3 boundBoxMax = m_center + boundBoxScale / 2.f;
	m_boundBox = AABB3(boundBoxMin, boundBoxMax);
	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);
}

SphereEntity3::~SphereEntity3()
{

}


void SphereEntity3::UpdateEntityPrimitive()
{
	m_primitive.m_center = m_center;
}

void SphereEntity3::Render(Renderer* renderer)
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

void SphereEntity3::Translate(Vector3 translation)
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

void SphereEntity3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}

