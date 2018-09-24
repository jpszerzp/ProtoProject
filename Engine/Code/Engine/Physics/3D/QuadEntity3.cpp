#include "Engine/Physics/3D/QuadEntity3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

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
	//m_sphereBoundTransform = Transform(boundSpherePos, boundSphereRot, boundSphereScale);

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
	//m_sphereBoundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(boundSpherePos, boundSphereRot, boundSphereScale);

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
		m_boundSphere.DrawBound(renderer);
}

void QuadEntity3::Translate(Vector3 translation)
{
	m_center += translation;

	// update transform
	// bound box ignored
	m_entityTransform.SetLocalPosition(m_center);
	//m_sphereBoundTransform.SetLocalPosition(m_center);
	m_boundSphere.m_transform.SetLocalPosition(m_center);

	// update primitive
	// bound box ignored
	m_primitive.Translate(translation);
	m_boundSphere.Translate(translation);
}

void QuadEntity3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}
