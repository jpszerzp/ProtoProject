#include "Game/WrapAround3.hpp"
#include "Engine/Renderer/Renderable.hpp"

WrapAround::WrapAround(const Vector3& min, const Vector3& max,
	const Vector3& p1, const Vector3& p2, 
	const Vector3& p3, const Vector3& p4,
	const Vector3& p5, const Vector3& p6,
	const Vector3& p7, const Vector3& p8)
{
	m_bounds = AABB3(min, max);
	float bound_width = m_bounds.GetDimensions().x;
	float bound_height = m_bounds.GetDimensions().y;
	float bound_thickness = m_bounds.GetDimensions().z;

	m_mesh = Mesh::CreateCube(VERT_PCU);

	Vector3 pos = m_bounds.GetCenter();
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3(bound_width, bound_height, bound_thickness);
	m_transform = Transform(pos, rot, scale);

	float quater_width = bound_width * .25f;
	float quater_height = bound_height * .25f;
	float quater_thickness = bound_thickness * .25f;

	m_positions[0] = p1;
	m_positions[1] = p2;
	m_positions[2] = p3;
	m_positions[3] = p4;
	m_positions[4] = p5;
	m_positions[5] = p6;
	m_positions[6] = p7;
	m_positions[7] = p8;
}

WrapAround::~WrapAround()
{

}

void WrapAround::Update()
{
	// goes thru every gameobject in this wraparound group and update ONLY when they teleport
	for (GameObject* go : m_gos)
	{
		// surpassing xmin
		if (go->GetEntity()->GetEntityCenter().x < m_bounds.m_min.x)
		{
			float pos_x = m_bounds.m_max.x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().x > m_bounds.m_max.x)
		{
			float pos_x = m_bounds.m_min.x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().y < m_bounds.m_min.y)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = m_bounds.m_max.y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().y > m_bounds.m_max.y)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = m_bounds.m_min.y;
			float pos_z = go->GetEntity()->GetEntityCenter().z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().z < m_bounds.m_min.z)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = m_bounds.m_max.z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		if (go->GetEntity()->GetEntityCenter().z > m_bounds.m_max.z)
		{
			float pos_x = go->GetEntity()->GetEntityCenter().x;
			float pos_y = go->GetEntity()->GetEntityCenter().y;
			float pos_z = m_bounds.m_min.z;

			go->GetEntity()->SetEntityCenter(Vector3(pos_x, pos_y, pos_z));
		}

		go->GetEntity()->m_entityTransform.SetLocalPosition(go->GetEntity()->GetEntityCenter());
		go->GetEntity()->m_boundSphere.m_transform.SetLocalPosition(go->GetEntity()->GetEntityCenter());
		go->GetEntity()->m_boxBoundTransform.SetLocalPosition(go->GetEntity()->GetEntityCenter());

		go->GetEntity()->UpdatePrimitives();

		go->m_renderable->m_transform = go->GetEntity()->GetEntityTransform();

		go->UpdateBasis();
	}
}

void WrapAround::Render(Renderer* renderer)
{
	Shader* shader = renderer->CreateOrGetShader("wireframe_color");
	renderer->UseShader(shader);

	Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());
	glLineWidth(2.f);

	renderer->m_objectData.model = m_transform.GetWorldMatrix();

	if (m_mesh != nullptr)
		renderer->DrawMesh(m_mesh, false);
}
