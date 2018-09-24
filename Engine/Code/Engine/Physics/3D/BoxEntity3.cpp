#include "Engine/Physics/3D/BoxEntity3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

BoxEntity3::BoxEntity3(const OBB3& primitive, const Vector3& rot, eMoveStatus moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_PARTICLE;		// for primitive_entity3 (not primitive_RB3), considered as PARTICLE by default

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_center;

	Vector3 scale = m_primitive.GetHalfExt() * 2.f;
	m_entityTransform = Transform(m_center, rot, scale);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
	}
	else
		m_massData.m_invMass = 0.f;

	float diagonal = m_primitive.GetDiagonalRadius();
	m_boundSphere = BoundingSphere(m_center, diagonal);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(m_center, rot, Vector3(diagonal));

	TODO("AABB3 BV later if necessary");

	m_linearDamp = 1.f;
}

BoxEntity3::~BoxEntity3()
{

}

void BoxEntity3::UpdateEntityPrimitive()
{
	m_primitive.SetCenter(m_center);
}

void BoxEntity3::Render(Renderer* renderer)
{
	if (m_drawBoundSphere)
		m_boundSphere.DrawBound(renderer);
}

void BoxEntity3::SetEntityForPrimitive()
{
	m_primitive.SetEntity(this);
}
