#include "Game/GameCommon.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Math/Particle.hpp"
#include "Engine/Physics/3D/PointEntity3.hpp"
#include "Engine/Input/InputSystem.hpp"

Point::Point()
{

}

Point::Point(Vector3 pos, Vector3 rot, float size, Rgba tint,
	std::string meshName, std::string materialName,
	eMoveStatus moveStat, eBodyIdentity bid, bool multiPass,
	eCompare compare, eCullMode cull, eWindOrder order)
{
	Renderer* renderer = Renderer::GetInstance();

	Material* material;
	if (!multiPass)
		material = renderer->CreateOrGetMaterial(materialName);
	else
		material = renderer->CreateOrGetStagedMaterial(materialName);

	Mesh* mesh = renderer->CreateOrGetMesh(meshName);
	Vector3 scale = Vector3(size);
	Transform transform = Transform(pos, rot, scale);

	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(material, mesh, transform, tintVec4);

	m_desiredCompare = compare;
	m_desiredCull = cull;
	m_desiredOrder = order;

	Particle particle = Particle(pos, size);
	if (bid == BODY_PARTICLE)
		m_physEntity = new PointEntity3(particle, moveStat);
	// for a point gameobject, it should ALWAYS be a particle entity, not rigidbody
	m_physEntity->SetEntityForPrimitive();
}

Point::~Point()
{
	delete m_renderable;
	m_renderable = nullptr;
}


void Point::Update(float deltaTime)
{
	UpdateInput(deltaTime);

	if (m_physDriven)
	{
		m_physEntity->Integrate(deltaTime);
		m_physEntity->UpdateTransforms();		// including bound transforms
		m_physEntity->UpdatePrimitives();

		// physics driven, hence update renderable transform
		m_renderable->m_transform = m_physEntity->GetEntityTransform();
	}

	// update basis as needed
	UpdateBasis();
}

void Point::UpdateInput(float)
{
	// adding the constrained condition is to only freeze particles in constraints like spring
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_3) && m_physEntity->m_constrained)
	{
		m_physEntity->m_frozen = !m_physEntity->m_frozen;
	}
}

void Point::ObjectDrivePosition(Vector3 pos)
{
	m_renderable->m_transform.SetLocalPosition(pos);

	m_physEntity->SetEntityCenter(pos);
	m_physEntity->UpdateTransforms();
	m_physEntity->UpdatePrimitives();
}

void Point::Render(Renderer* renderer)
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

			PointEntity3* pte = dynamic_cast<PointEntity3*>(m_physEntity);
			Particle particle = pte->GetParticlePrimitive();
			float ptSize = particle.m_size;
			glPointSize(ptSize);
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

	// render basis as needed
	RenderBasis(renderer);

	// render phys entity as needed
	if (m_physEntity != nullptr)
	{
		m_physEntity->Render(renderer);
	}
}

