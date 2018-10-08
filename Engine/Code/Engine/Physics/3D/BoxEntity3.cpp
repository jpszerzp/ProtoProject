#include "Engine/Physics/3D/BoxEntity3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

BoxEntity3::BoxEntity3(const OBB3& primitive, const Vector3& rot, eMoveStatus moveStat)
{
	m_primitive = primitive;
	m_moveStatus = moveStat;
	m_bodyID = BODY_PARTICLE;		// for primitive_entity3 (not primitive_RB3), considered as PARTICLE by default

	m_linearVelocity = Vector3::ZERO;
	m_center = primitive.m_center;

	Vector3 scale = primitive.GetHalfExt() * 2.f;
	m_entityTransform = Transform(m_center, rot, scale);

	if (m_moveStatus != MOVE_STATIC)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
	}
	else
		m_massData.m_invMass = 0.f;

	float diagonal = primitive.GetDiagonalRadius();
	m_boundSphere = BoundingSphere(m_center, diagonal);
	m_boundSphere.m_boundMesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
	m_boundSphere.m_transform = Transform(m_center, rot, Vector3(diagonal));

	TODO("AABB3 BV later if necessary");

	m_linearDamp = 1.f;
}

BoxEntity3::~BoxEntity3()
{

}

Vector3 BoxEntity3::GetFeaturedPoint(eContactFeature feature)
{
	switch (feature)
	{
	case V1: return m_primitive.GetFTL(); break;
	case V2: return m_primitive.GetFBL(); break;
	case V3: return m_primitive.GetFBR(); break;
	case V4: return m_primitive.GetFTR(); break;
	case V5: return m_primitive.GetBTL(); break;
	case V6: return m_primitive.GetBBL(); break;
	case V7: return m_primitive.GetBBR(); break;
	case V8: return m_primitive.GetBTR(); break;
	default: break;
	}
	return Vector3::INVALID;
}

LineSegment3 BoxEntity3::GetFeaturedEdge(eContactFeature feature)
{
	switch (feature)
	{
	case E1: return LineSegment3(GetFeaturedPoint(V1), GetFeaturedPoint(V2)); break;
	case E2: return LineSegment3(GetFeaturedPoint(V2), GetFeaturedPoint(V3)); break;
	case E3: return LineSegment3(GetFeaturedPoint(V3), GetFeaturedPoint(V4)); break;
	case E4: return LineSegment3(GetFeaturedPoint(V4), GetFeaturedPoint(V1)); break;
	case E5: return LineSegment3(GetFeaturedPoint(V5), GetFeaturedPoint(V6)); break;
	case E6: return LineSegment3(GetFeaturedPoint(V6), GetFeaturedPoint(V7)); break;
	case E7: return LineSegment3(GetFeaturedPoint(V7), GetFeaturedPoint(V8)); break;
	case E8: return LineSegment3(GetFeaturedPoint(V8), GetFeaturedPoint(V5)); break;
	case E9: return LineSegment3(GetFeaturedPoint(V5), GetFeaturedPoint(V1)); break;
	case E10: return LineSegment3(GetFeaturedPoint(V8), GetFeaturedPoint(V4)); break;
	case E11: return LineSegment3(GetFeaturedPoint(V7), GetFeaturedPoint(V3)); break;
	case E12: return LineSegment3(GetFeaturedPoint(V6), GetFeaturedPoint(V2)); break;
	case UNKNOWN: break;
	default: break;
	}
	return LineSegment3(Vector3::INVALID, Vector3::INVALID);
}

void BoxEntity3::UpdatePrimitives()
{
	m_boundSphere.SetCenter(m_center);
	m_boundBox.SetCenter(m_center);

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
