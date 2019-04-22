#include "Engine/Core/Primitive/Cone.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void Cone::Render(Renderer* renderer)
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
		for each (eDepthCompare c in renderer->m_currentShader->m_state.m_depthCompares)
		{
			if ( c == m_desiredCompare )
				renderer->m_currentShader->m_state.m_depthCompare = m_desiredCompare;
		}
		// by default, if compare mode not found in those supported by shader, will use COMPRAE_LESS

		// set desired cull
		for each (eCullMode c in renderer->m_currentShader->m_state.m_cullModes)
		{
			if (c == m_desiredCull)
				renderer->m_currentShader->m_state.m_cullMode = m_desiredCull;
		}
		// by default, if cull mode not found in those supported by shader, use CULLMODE_BACK

		// set desired order
		for each (eWindOrder w in renderer->m_currentShader->m_state.m_windOrders)
		{
			if (w == m_desiredOrder)
				renderer->m_currentShader->m_state.m_windOrder = m_desiredOrder;
		}
		// by default, if wind order not found in those supported by shader, use WIND_COUNTER_CLOCKWISE

		renderer->DrawMesh(mesh);
	}
}

void Cone::Update(float deltaTime)
{

}

Cone::Cone(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint, std::string meshName, std::string materialName, eMoveStatus, eBodyIdentity, bool multipass /*= false*/, eDepthCompare compare /*= COMPARE_LESS*/, eCullMode cull /*= CULLMODE_BACK*/, eWindOrder wind /*= WIND_COUNTER_CLOCKWISE*/)
{
	Renderer* r = Renderer::GetInstance();

	// Set renderable
	Material* material;
	if (!multipass)
		material = r->CreateOrGetMaterial(materialName);
	else
		material = r->CreateOrGetStagedMaterial(materialName);
	Mesh* mesh = r->CreateOrGetMesh(meshName);
	Transform transform = Transform(pos, rot, scale);
	Vector4 tintV4;
	tint.GetAsFloats(tintV4.x, tintV4.y, tintV4.z, tintV4.w);
	m_renderable = new Renderable(material, mesh, transform, tintV4);

	m_desiredCompare = compare;
	m_desiredCull = cull;
	m_desiredOrder = wind;

	TODO("Physics entity");
}

Cone::~Cone()
{
	delete m_renderable;
	m_renderable = nullptr;
}
