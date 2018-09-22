#include "Engine/Core/Primitive/Cube.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"

Cube::Cube()
{

}


Cube::Cube(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint,
	std::string meshName, std::string materialName, 
	eMoveStatus moveStat, eBodyIdentity bid, bool multipass,
	eCompare compare, eCullMode cull, eWindOrder order)
{
	Renderer* renderer = Renderer::GetInstance();

	// Set renderable
	Material* material;
	if (!multipass)
	{
		material = renderer->CreateOrGetMaterial(materialName);
	}
	else
	{
		material = renderer->CreateOrGetStagedMaterial(materialName);
	}

	Mesh* mesh = renderer->CreateOrGetMesh(meshName);
	Transform transform = Transform(pos, rot, scale);

	// Set tint
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(material, mesh, transform, tintVec4);

	m_desiredCompare = compare;
	m_desiredCull = cull;
	m_desiredOrder = order;

	Vector3 halfAABB = scale / 2.f;
	Vector3 aabbMin = pos - halfAABB;
	Vector3 aabbMax = pos + halfAABB;
	AABB3 aabb3 = AABB3(aabbMin, aabbMax);
	if (bid == BODY_PARTICLE)
		m_physEntity = new CubeEntity3(aabb3, moveStat);
	TODO("cube rigid body");
	m_physEntity->SetEntityForPrimitive();
}


Cube::~Cube()
{
	delete m_renderable;
	m_renderable = nullptr;
}


void Cube::Update(float deltaTime)
{
	if (m_physDriven)
	{
		m_physEntity->Integrate(deltaTime);
		m_physEntity->UpdateEntitiesTransforms();
		m_physEntity->UpdateEntityPrimitive();
		m_physEntity->UpdateBoundPrimitives();

		m_renderable->m_transform = m_physEntity->GetEntityTransform();
	}

	UpdateBasis();
}

void Cube::Render(Renderer* renderer)
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