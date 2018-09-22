#include "Engine/Physics/3D/BoundingVolume3.hpp"
#include "Engine/Math/MathUtils.hpp"

BoundingSphere::BoundingSphere(const BoundingSphere& child_one, const BoundingSphere& child_two)
{
	Vector3 disp = child_two.m_center - child_one.m_center;
	float distSqrd = disp.GetLengthSquared();
	float radDiff = child_two.m_radius - child_one.m_radius;

	// one sphere is entirely encapsulating another
	if (radDiff * radDiff >= distSqrd)
	{
		if (child_one.m_radius > child_two.m_radius)
		{
			// child_one is the enclosing one
			m_center = child_one.m_center;
			m_radius = child_one.m_radius;
		}
		else
		{
			m_center = child_two.m_center;
			m_radius = child_two.m_radius;
		}
	}
	else
	{
		// spheres are overlapping
		float dist = sqrtf(distSqrd);

		m_radius = (dist + child_one.m_radius + child_two.m_radius) * .5f;
		m_center = child_one.m_center;
		if (dist != 0.f)
			m_center += disp * ((m_radius - child_one.m_radius) / dist);
	}
}

BoundingSphere::~BoundingSphere()
{
	//delete m_boundMesh;
	//m_boundMesh = nullptr;
}

bool BoundingSphere::Overlaps(const BoundingSphere* other) const
{
	Vector3 disp = m_center - other->m_center;
	float distSqrd = disp.GetLengthSquared();
	float standardSqrd = (m_radius + other->m_radius) * (m_radius + other->m_radius);
	bool overlapped = (distSqrd < standardSqrd); 
	return overlapped;
}

float BoundingSphere::GetVolume() const
{
	float volume = (4.f / 3.f) * PI * m_radius * m_radius * m_radius;
	return volume;
}

float BoundingSphere::GetGrowth(const BoundingSphere& other) const
{
	BoundingSphere newBound(*this, other);

	// proportional to surface area
	return newBound.m_radius * newBound.m_radius - m_radius * m_radius;
}

void BoundingSphere::Translate(Vector3 translation)
{
	m_center += translation;
}

void BoundingSphere::SetCenter(Vector3 center)
{
	m_center = center;						// update center
}

void BoundingSphere::DrawBound(Renderer* renderer)
{
	// if at leaf, we can set this mesh to null so that this draw is ignored
	if (m_boundMesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->m_objectData.model = m_transform.GetWorldMatrix();

		Vector4 colorV4;
		Rgba color = Rgba::CYAN;
		color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);
		renderer->m_colorData.rgba = colorV4;
		renderer->SetColorUBO(shader->GetShaderProgram()->GetHandle());

		glLineWidth(1.f);
		renderer->DrawMesh(m_boundMesh);
	}
}
