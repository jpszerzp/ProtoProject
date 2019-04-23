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

	m_mesh = Mesh::CreateCube(VERT_PCU);

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
	if (!m_particle)
	{
		// delete objects if they go beyond bound
		for (CollisionPrimitive* primitive : m_primitives)
		{
			if (primitive->GetBasisAndPosition(3).x < m_bounds.m_min.x)
				primitive->SetShouldDelete(true);

			else if (primitive->GetBasisAndPosition(3).x > m_bounds.m_max.x)
				primitive->SetShouldDelete(true);

			else if (primitive->GetBasisAndPosition(3).y < m_bounds.m_min.y)
				primitive->SetShouldDelete(true);

			else if (primitive->GetBasisAndPosition(3).y > m_bounds.m_max.y)
				primitive->SetShouldDelete(true);

			else if (primitive->GetBasisAndPosition(3).z < m_bounds.m_min.z)
				primitive->SetShouldDelete(true);

			else if (primitive->GetBasisAndPosition(3).z > m_bounds.m_max.z)
				primitive->SetShouldDelete(true);
		}

		for (PhysXObject* px_obj : m_phys_obj)
		{
			if (px_obj->GetPos().x < m_bounds.m_min.x)
				px_obj->SetShouldDelete(true);
			else if (px_obj->GetPos().x > m_bounds.m_max.x)
				px_obj->SetShouldDelete(true);
			else if (px_obj->GetPos().y < m_bounds.m_min.y)
				px_obj->SetShouldDelete(true);
			else if (px_obj->GetPos().y > m_bounds.m_max.y)
				px_obj->SetShouldDelete(true);
			else if (px_obj->GetPos().z < m_bounds.m_min.z)
				px_obj->SetShouldDelete(true);
			else if (px_obj->GetPos().z > m_bounds.m_max.z)
				px_obj->SetShouldDelete(true);
		}
	}
	else
	{
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

void WrapAround::RemovePhysXObj(PhysXObject* px)
{
	for (int i = 0; i < m_phys_obj.size(); ++i)
	{
		if (m_phys_obj[i] == px)
		{
			std::vector<PhysXObject*>::iterator it = m_phys_obj.begin() + i;
			m_phys_obj.erase(it);
			i--;
		}
	}
}

void WrapAround::Render(Renderer* renderer)
{
	if (m_particle)
		RenderWraparounds(renderer);
}
