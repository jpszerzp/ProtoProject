#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderable.hpp"


Line::Line()
{

}

Line::Line(Vector3 start, Vector3 end, float thickness, Rgba tint, 
	std::string materialName, bool multipass)
	: m_start(start), m_end(end), m_thickness(thickness)
{
	Renderer* renderer = Renderer::GetInstance();

	Material* material;
	if (!multipass)
	{
		material = renderer->CreateOrGetMaterial(materialName);
	}
	else
	{
		material = renderer->CreateOrGetStagedMaterial(materialName);
	}

	Mesh* mesh = Mesh::CreateLineImmediate(VERT_PCU, start, end, tint);

	Vector3 pos = Vector3::ZERO;
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3::ONE;
	Transform transform = Transform(pos, rot, scale);

	// Set tint
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(material, mesh, transform, tintVec4);

	// set entity as needed
}

Line::~Line()
{
	delete m_renderable;
	m_renderable = nullptr;
}


void Line::Update(float)
{
	
}


void Line::Render(Renderer* renderer)
{
	Mesh* mesh = m_renderable->m_mesh;
	// not using transform since we use immediate mesh

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

		// mesh is immediate
		renderer->m_objectData.model = Matrix44::IDENTITY;

		glLineWidth(m_thickness);
		renderer->DrawMesh(mesh);
	}

	// render phys entity as needed
}

