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

	//delete m_physEntity;
	//m_physEntity = nullptr;
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

void GameObject::Render(Renderer* renderer)
{
	Mesh* mesh = m_renderable->m_mesh;
	Transform& transform = m_renderable->m_transform;

	if (mesh != nullptr)
	{
		Shader* shader;
		if (!m_debugOn)
		{
			shader = m_renderable->GetMaterial()->m_shader;

			std::map<int, Texture*>& boundTextures = m_renderable->GetMaterial()->m_textures;
			for (std::map<int, Texture*>::iterator it = boundTextures.begin(); it != boundTextures.end(); ++it)
			{
				int bindIdx = it->first;
				Texture* texture = it->second;

				renderer->SetTexture2D(bindIdx, texture);
				renderer->SetSampler2D(bindIdx, texture->GetSampler());
			}
		}
		else
		{
			shader = renderer->CreateOrGetShader("wireframe");

			Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
			renderer->SetTexture2D(0, texture);
			renderer->SetSampler2D(0, texture->GetSampler());

			glLineWidth(2.f);
		}
		renderer->UseShader(shader);

		m_renderable->GetMaterial()->SetProperty("TINT", m_renderable->GetTint());

		for (std::map<std::string, PropertyBlock*>::iterator it = m_renderable->GetMaterial()->m_blocks.begin();
			it != m_renderable->GetMaterial()->m_blocks.end(); ++it)
		{
			PropertyBlock* block = it->second;
			glBindBufferBase(GL_UNIFORM_BUFFER, block->m_blockInfo->blockIdx, block->GetHandle());

			size_t size = block->m_blockInfo->blockSize;
			block->CopyToGPU(size, block->m_dataBlock);
		}

		// Update object UBO values 
		renderer->m_objectData.model = transform.GetWorldMatrix();
		//renderer->m_objectData.model = transform.GetLocalMatrix();

		// set desired compare
		for each (eCompare c in renderer->m_currentShader->m_state.m_depthCompares)
		{
			if ( c == m_desiredCompare )
			{
				renderer->m_currentShader->m_state.m_depthCompare = m_desiredCompare;
			}
		}
		// by default, if compare mode not found in those supported by shader, will use COMPRAE_LESS

		// set desired cull
		for each (eCullMode c in renderer->m_currentShader->m_state.m_cullModes)
		{
			if (c == m_desiredCull)
			{
				renderer->m_currentShader->m_state.m_cullMode = m_desiredCull;
			}
		}
		// by default, if cull mode not found in those supported by shader, use CULLMODE_BACK

		// set desired order
		for each (eWindOrder w in renderer->m_currentShader->m_state.m_windOrders)
		{
			if (w == m_desiredOrder)
			{
				renderer->m_currentShader->m_state.m_windOrder = m_desiredOrder;
			}
		}
		// by default, if wind order not found in those supported by shader, use WIND_COUNTER_CLOCKWISE

		renderer->DrawMesh(mesh);
	}

	RenderBasis(renderer);

	if (m_physEntity != nullptr)
		m_physEntity->Render(renderer);
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
void GameObject::EntityDrivePosition(Vector3 translation)
{
	m_physEntity->Translate(translation);								// first update entity
	m_renderable->m_transform = m_physEntity->GetEntityTransform();		// then drive gameobject
}


Vector3 GameObject::GetWorldPosition()
{
	 const Matrix44& worldMat = m_renderable->m_transform.GetWorldMatrix();
	 Vector3 worldPos = Vector3(worldMat.Tx, worldMat.Ty, worldMat.Tz);
	 return worldPos;
}