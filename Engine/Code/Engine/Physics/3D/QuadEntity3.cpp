#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

//QuadEntity3::QuadEntity3(const Plane& primitive, bool isConst, Vector3 quadCenter, Vector3 quadRot, Vector3 quadScale)
//{
//	m_primitive = primitive;
//	m_const = isConst;
//
//	m_linearVelocity = Vector3::ZERO;
//
//	m_center = quadCenter;
//
//	// entity transform
//	m_entityTransform = Transform(m_center, quadRot, quadScale);
//
//	// BV transform - sphere
//	Vector3 boundSpherePos = m_center;
//	Vector3 boundSphereRot = Vector3::ZERO;
//	float boundSphereRadius = sqrtf(quadScale.x * quadScale.x + quadScale.y * quadScale.y) / 2.f;
//	Vector3 boundSphereScale = Vector3(boundSphereRadius, boundSphereRadius, boundSphereRadius);
//	m_sphereBoundTransform = Transform(boundSpherePos, boundSphereRot, boundSphereScale);
//
//	// BV box transform ignored
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
//	// BV sphere primitive
//	//m_boundSphere = Sphere3(boundSpherePos, boundSphereRad);
//	m_boundSphere = BoundingSphere(m_center, boundSphereRadius);
//	m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
//
//	// BV box primitive ignored
//}

QuadEntity3::QuadEntity3(const Plane& primitive, eMoveStatus moveStat,
	Vector3 center, Vector3 rot, Vector3 scale)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_PARTICLE;

	m_linearVelocity = Vector3::ZERO;

	m_center = center;

	// entity transform
	m_entityTransform = Transform(m_center, rot, scale);

	// BV transform - sphere
	Vector3 boundSpherePos = m_center;
	Vector3 boundSphereRot = Vector3::ZERO;
	float boundSphereRadius = sqrtf(scale.x * scale.x + scale.y * scale.y) / 2.f;
	Vector3 boundSphereScale = Vector3(boundSphereRadius, boundSphereRadius, boundSphereRadius);
	m_sphereBoundTransform = Transform(boundSpherePos, boundSphereRot, boundSphereScale);

	// BV box transform ignored

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
	}
	else
		m_massData.m_invMass = 0.f;

	// BV sphere primitive
	//m_boundSphere = Sphere3(boundSpherePos, boundSphereRad);
	m_boundSphere = BoundingSphere(m_center, boundSphereRadius);
	m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);

	// BV box primitive ignored
}

QuadEntity3::~QuadEntity3()
{

}

void QuadEntity3::UpdateEntityPrimitive()
{
	m_primitive.m_offset = DotProduct(m_center, m_primitive.m_normal);
}


void QuadEntity3::Render(Renderer* renderer)
{
	// bound box render ignored

	// bound sphere render
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

void QuadEntity3::Translate(Vector3 translation)
{
	m_center += translation;

	// update transform
	// bound box ignored
	m_entityTransform.SetLocalPosition(m_center);
	m_sphereBoundTransform.SetLocalPosition(m_center);

	// update primitive
	// bound box ignored
	m_primitive.Translate(translation);
	m_boundSphere.Translate(translation);
}

void QuadEntity3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}
