#pragma once

#include "Engine/Core/Light/SpotLight.hpp"
#include "Engine/Renderer/Renderable.hpp"

SpotLight::SpotLight(Vector3 pos, Vector3 rot, Vector3 scale, Rgba color, Vector3 attenuation, Vector3 specAttenuation, Vector3 direction, float innerAngle, float outerAngle, float directionFac)
	: Light(pos, rot, scale, color, attenuation, specAttenuation, direction, innerAngle, outerAngle, directionFac)
{

}


SpotLight::SpotLight(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Rgba& color, const Vector3& direction)
	:Light(pos, rot, scale, color, Vector3::ZERO, Vector3::ZERO, direction, 0.f, 0.f, 0.f)
{
	
}

SpotLight::~SpotLight()
{

}


void SpotLight::Update(float)
{

}


void SpotLight::Render(Renderer* renderer)
{
	Mesh* mesh = m_renderable->GetMesh();
	Material* mat = m_renderable->GetMaterial();
	Shader* shader = mat->m_shader;
	Transform& transform = m_renderable->m_transform;

	if (mesh != nullptr)
	{
		renderer->UseShader(shader);

		for (std::map<int, Texture*>::iterator it = mat->m_textures.begin();
			it != mat->m_textures.end(); ++it)
		{
			int idx = it->first;
			Texture* tex = it->second;

			renderer->SetSampler2D(idx, tex->GetSampler());
			renderer->SetTexture2D(idx, tex);
		}

		m_renderable->m_material->SetProperty("TINT", m_renderable->GetTint());

		for (std::map<std::string, PropertyBlock*>::iterator it = mat->m_blocks.begin();
			it != mat->m_blocks.end(); ++it)
		{
			PropertyBlock* block = it->second;
			glBindBufferBase(GL_UNIFORM_BUFFER, block->m_blockInfo->blockIdx, block->GetHandle());

			size_t size = block->m_blockInfo->blockSize;
			block->CopyToGPU(size, block->m_dataBlock);
		}

		//renderer->m_objectData.model = transform.GetLocalMatrix();
		renderer->m_objectData.model = transform.GetWorldMatrix();

		glLineWidth(1.f);
		renderer->DrawMesh(mesh);
	}
}