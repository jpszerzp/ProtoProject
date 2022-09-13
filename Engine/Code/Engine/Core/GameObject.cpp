#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Util/AssetUtils.hpp"
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

			Texture* texture = renderer->CreateOrGetTexture(GetAbsImgPath("white"));
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

		// set desired compare
		for each (eDepthCompare c in renderer->m_currentShader->m_state.m_depthCompares)
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
}


void GameObject::RenderWithBorder(Renderer* renderer)
{
	Mesh* mesh = m_renderable->m_mesh;

	if (mesh != nullptr)
	{
		TODO("Consistency between local and world transform");
		const Transform& transform_inner = m_renderable->m_transform;
		const Vector3& scale_inner = transform_inner.GetLocalScale();
		const Vector3& translation_inner = transform_inner.GetLocalPosition();
		const Vector3& rot_inner = transform_inner.GetLocalRotation();
		Vector3 scale_outer = scale_inner * 1.1f;
		Transform transform_outer = Transform(translation_inner, rot_inner, scale_outer);

		Shader* shader_inner = m_renderable->GetShader();
		Shader* shader_outer = renderer->MakeShader("shader/border");

		Texture* tex_inner = renderer->CreateOrGetTexture(GetAbsImgPath("couch\\couch_diffuse"));
		Texture* tex_outer = renderer->CreateOrGetTexture(GetAbsImgPath("white"));

		// draw the inner object, normally
		renderer->UseShader(shader_inner);
		renderer->SetTexture2D(0, tex_inner);
		renderer->SetSampler2D(0, tex_inner->GetSampler());
		renderer->m_colorData.rgba = m_renderable->GetTint();
		renderer->m_objectData.model = transform_inner.GetWorldMatrix();
		renderer->EnableStencil();
		renderer->SetStencilOP(GL_KEEP, GL_KEEP, GL_REPLACE);
		renderer->SetStencilFunc(GL_ALWAYS);
		renderer->SetStencilMask(0xFF);
		renderer->Draw(mesh);

		// draw outer border
		renderer->UseShader(shader_outer);
		renderer->SetTexture2D(0, tex_outer);
		renderer->SetSampler2D(0, tex_outer->GetSampler());
		renderer->m_objectData.model = transform_outer.GetWorldMatrix();
		renderer->SetStencilFunc(GL_NOTEQUAL);
		renderer->SetStencilMask(0x00);
		renderer->Draw(mesh, true, false);
		renderer->SetStencilMask(0xFF);
		renderer->EnableDepth();
		renderer->DisableStencil();
	}
}

void GameObject::RenderBasis(Renderer* renderer)
{
	if (!(m_forwardBasisMesh == nullptr || m_upBasisMesh == nullptr || m_rightBasisMesh == nullptr) && m_drawBasis)
	{
		Shader* basisShader = renderer->CreateOrGetShader("direct");
		renderer->UseShader(basisShader);
		Texture* basisTexture = renderer->CreateOrGetTexture(GetAbsImgPath("white"));
		renderer->SetTexture2D(0, basisTexture);
		renderer->SetSampler2D(0, basisTexture->GetSampler());

		renderer->m_objectData.model = Matrix44::IDENTITY;
		glLineWidth(3.f);
		renderer->DrawMesh(m_forwardBasisMesh);
		renderer->DrawMesh(m_upBasisMesh);
		renderer->DrawMesh(m_rightBasisMesh);
	}
}

Vector3 GameObject::GetWorldPosition()
{
	 const Matrix44& worldMat = m_renderable->m_transform.GetWorldMatrix();
	 Vector3 worldPos = Vector3(worldMat.Tx, worldMat.Ty, worldMat.Tz);
	 return worldPos;
}