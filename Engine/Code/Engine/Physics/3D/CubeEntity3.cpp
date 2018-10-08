#include "Engine/Physics/3D/CubeEntity3.hpp"

CubeEntity3::CubeEntity3(const AABB3& primitive, eMoveStatus moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_PARTICLE;		// for primitive_entity3 (not primitive_RB3), considered as PARTICLE by default

	m_linearVelocity = Vector3::ZERO;

	Vector3 min = primitive.m_min;
	Vector3 max = primitive.m_max;
	Vector3 center = (min + max) * 0.5f;
	m_center = center;

	Vector3 pos = primitive.GetCenter();
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = primitive.GetDimensions();
	m_entityTransform = Transform(pos, rot, scale);

	float xDim = primitive.GetDimensions().x;
	float yDim = primitive.GetDimensions().y;
	float zDim = primitive.GetDimensions().z;
	float boundSphereRadius = sqrtf(xDim * xDim + yDim * yDim + zDim * zDim) / 2.f;
	Vector3 boundSphereScale = Vector3(boundSphereRadius, boundSphereRadius, 1.f);
	m_boxBoundTransform = m_entityTransform;
	//m_sphereBoundTransform = Transform(pos, rot, boundSphereScale);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
	}
	else
		m_massData.m_invMass = 0.f;

	m_boundSphere = BoundingSphere(m_center, boundSphereRadius);
	//m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(pos, rot, boundSphereScale);

	m_boundBox = primitive;
	m_boxBoundMesh = Mesh::CreateCube(VERT_PCU);
}

CubeEntity3::~CubeEntity3()
{

}

void CubeEntity3::UpdatePrimitives()
{
	m_boundSphere.SetCenter(m_center);
	m_boundBox.SetCenter(m_center);

	m_primitive.SetCenter(m_center);
}

void CubeEntity3::Render(Renderer* renderer)
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

void CubeEntity3::Translate(Vector3 translation)
{
	m_center += translation;

	// update transform
	m_entityTransform.SetLocalPosition(m_center);
	//m_sphereBoundTransform.SetLocalPosition(m_center);
	m_boundSphere.m_transform.SetLocalPosition(m_center);
	m_boxBoundTransform.SetLocalPosition(m_center);

	// update primitive
	m_primitive.Translate(translation);
	m_boundBox.Translate(translation);
	m_boundSphere.Translate(translation);
}

void CubeEntity3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}
