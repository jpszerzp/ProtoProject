#include "Game/WrapAround3.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Physics3State.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"

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

	//m_mesh = Mesh::CreateCube(VERT_PCU);
	m_mesh = Mesh::CreateUnitQuadInLine(VERT_PCU, Rgba::WHITE);

	Vector3 pos = m_bounds.GetCenter();
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3(bound_width, bound_height, bound_thickness);
	m_transform = Transform(pos, rot, scale);

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
	UpdateWraparound();
}


void WrapAround::UpdateWraparound()
{
	// goes thru every gameobject in this wraparound group and update ONLY when they teleport
	for (CollisionPrimitive* primitive : m_primitives)
	{
		// surpassing xmin
		if (primitive->GetBasisAndPosition(3).x < m_bounds.m_min.x)
		{
			float pos_x = m_bounds.m_max.x;
			float pos_y = primitive->GetBasisAndPosition(3).y;
			float pos_z = primitive->GetBasisAndPosition(3).z;

			primitive->SetRigidBodyPosition(Vector3(pos_x, pos_y, pos_z));
		}

		if (primitive->GetBasisAndPosition(3).x > m_bounds.m_max.x)
		{
			float pos_x = m_bounds.m_min.x;
			float pos_y = primitive->GetBasisAndPosition(3).y;
			float pos_z = primitive->GetBasisAndPosition(3).z;

			primitive->SetRigidBodyPosition(Vector3(pos_x, pos_y, pos_z));
		}

		if (primitive->GetBasisAndPosition(3).y < m_bounds.m_min.y)
		{
			float pos_x = primitive->GetBasisAndPosition(3).x;
			float pos_y = m_bounds.m_max.y;
			float pos_z = primitive->GetBasisAndPosition(3).z;

			primitive->SetRigidBodyPosition(Vector3(pos_x, pos_y, pos_z));
		}

		if (primitive->GetBasisAndPosition(3).y > m_bounds.m_max.y)
		{
			float pos_x = primitive->GetBasisAndPosition(3).x;
			float pos_y = m_bounds.m_min.y;
			float pos_z = primitive->GetBasisAndPosition(3).z;

			primitive->SetRigidBodyPosition(Vector3(pos_x, pos_y, pos_z));
		}

		if (primitive->GetBasisAndPosition(3).z < m_bounds.m_min.z)
		{
			float pos_x = primitive->GetBasisAndPosition(3).x;
			float pos_y = primitive->GetBasisAndPosition(3).y;
			float pos_z = m_bounds.m_max.z;

			primitive->SetRigidBodyPosition(Vector3(pos_x, pos_y, pos_z));
		}

		if (primitive->GetBasisAndPosition(3).z > m_bounds.m_max.z)
		{
			float pos_x = primitive->GetBasisAndPosition(3).x;
			float pos_y = primitive->GetBasisAndPosition(3).y;
			float pos_z = m_bounds.m_min.z;

			primitive->SetRigidBodyPosition(Vector3(pos_x, pos_y, pos_z));
		}
	}
}

void WrapAround::RenderWraparounds(Renderer* renderer)
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

// remove this prim from storage; do not delete it as that is handled somewhere else
void WrapAround::RemovePrimitive(CollisionPrimitive* prim)
{
	for (int i = 0; i < m_primitives.size(); ++i)
	{
		if (m_primitives[i] == prim)
		{
			std::vector<CollisionPrimitive*>::iterator it = m_primitives.begin() + i;

			m_primitives.erase(it);
			i--;
		}
	}
}

void WrapAround::Render(Renderer* renderer)
{
	RenderWraparounds(renderer);
}
