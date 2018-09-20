#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderable.hpp"


GameObject::GameObject()
{

}


GameObject::~GameObject()
{
	if (!m_renderableDelayedDelete)
	{
		delete m_renderable;
		m_renderable = nullptr;
	}

	delete m_forwardBasisMesh;
	m_forwardBasisMesh = nullptr;

	delete m_upBasisMesh;
	m_upBasisMesh = nullptr;

	delete m_rightBasisMesh;
	m_rightBasisMesh = nullptr;

	delete m_physEntity;
	m_physEntity = nullptr;
}

void GameObject::UpdateBasis()
{
	if (m_drawBasis)
	{
		Vector3 world = m_renderable->m_transform.GetWorldPosition();
		Vector3 worldRight = m_renderable->m_transform.GetWorldRight().GetNormalized() * 3.f;
		Vector3 worldUp = m_renderable->m_transform.GetWorldUp().GetNormalized() * 3.f;
		Vector3 worldForward = m_renderable->m_transform.GetWorldForward().GetNormalized() * 3.f;
		Vector3 rightEnd = world + worldRight;
		Vector3 upEnd = world + worldUp;
		Vector3 forwardEnd = world + worldForward;

		if (m_forwardBasisMesh != nullptr)
		{
			delete m_forwardBasisMesh;
			m_forwardBasisMesh = nullptr;
		}
		m_forwardBasisMesh = Mesh::CreateLineImmediate(VERT_PCU, world, forwardEnd, Rgba::BLUE);

		if (m_upBasisMesh != nullptr)
		{
			delete m_upBasisMesh;
			m_upBasisMesh = nullptr;
		}
		m_upBasisMesh = Mesh::CreateLineImmediate(VERT_PCU, world, upEnd, Rgba::GREEN);

		if (m_rightBasisMesh != nullptr)
		{
			delete m_rightBasisMesh;
			m_rightBasisMesh = nullptr;
		}
		m_rightBasisMesh = Mesh::CreateLineImmediate(VERT_PCU, world, rightEnd, Rgba::RED);
	}
}

void GameObject::Render(Renderer*)
{

}


void GameObject::RenderBasis(Renderer* renderer)
{
	if (!(m_forwardBasisMesh == nullptr || m_upBasisMesh == nullptr || m_rightBasisMesh == nullptr) && m_drawBasis)
	{
		Shader* basisShader = renderer->CreateOrGetShader("direct");
		renderer->UseShader(basisShader);
		Texture* basisTexture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, basisTexture);
		renderer->SetSampler2D(0, basisTexture->GetSampler());

		renderer->m_objectData.model = Matrix44::IDENTITY;
		glLineWidth(3.f);
		renderer->DrawMesh(m_forwardBasisMesh);
		renderer->DrawMesh(m_upBasisMesh);
		renderer->DrawMesh(m_rightBasisMesh);
	}
}

void GameObject::ToggleBoundSphereDebugDraw()
{
	if (m_physEntity != nullptr)
	{
		bool draw = m_physEntity->GetDrawBoundingSphere();
		m_physEntity->SetDrawBoundingSphere(!draw);
	}
}

void GameObject::ToggleBoundBoxDebugDraw()
{
	if (m_physEntity != nullptr)
	{
		bool draw = m_physEntity->GetDrawBoundingBox();
		m_physEntity->SetDrawBoundingBox(!draw);
	}
}

/*
 * Entity driven movement of game object
 */
void GameObject::EntityDriveTranslate(Vector3 translation)
{
	m_physEntity->Translate(translation);								// first update entity
	m_renderable->m_transform = m_physEntity->GetEntityTransform();		// then drive gameobject
}


void GameObject::ObjectDrivePosition(Vector3 pos)
{

}

Vector3 GameObject::GetWorldPosition()
{
	 const Matrix44& worldMat = m_renderable->m_transform.GetWorldMatrix();
	 Vector3 worldPos = Vector3(worldMat.Tx, worldMat.Ty, worldMat.Tz);
	 return worldPos;
}