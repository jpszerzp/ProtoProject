#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/3D/PointEntity3.hpp"
#include "Game/Fireworks.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TheGame.hpp"
#include "Game/Physics3State.hpp"


Fireworks::Fireworks(Vector3 pos)
	: Point(pos, Vector3::ZERO, 5.f, Rgba::GREEN, "point_pcu", "default", MOVE_DYNAMIC, BODY_PARTICLE)
{

}

Fireworks::~Fireworks()
{

}

void Fireworks::Update(float deltaTime)
{
	m_dead = (m_age > 0.f) ? false : true;

	if (!m_dead)
	{
		m_physEntity->Integrate(deltaTime);
		m_physEntity->UpdateTransforms();
		m_physEntity->UpdatePrimitives();
		//m_physEntity->UpdateBoundPrimitives();

		// physics driven, hence update renderable transform
		m_renderable->m_transform = m_physEntity->GetEntityTransform();

		m_age -= deltaTime;
	}
	else
	{		
		if (!m_lastRound)
		{
			Physics3State* state = dynamic_cast<Physics3State*>(g_theGame->GetStateMachine()->GetCurrentState());

			// add payload
			for (uint i = 0; i < 30; ++i)
				state->SetupFireworks(5.f, GetWorldPosition(), m_physEntity->GetLinearVelocity(), Vector3(2.f), Vector3(-2.f), true);
		}
	}

	// for fireworks, do not draw basis 
}

void Fireworks::Render(Renderer* renderer)
{
	if (m_age > 0.f)
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

		// render phys entity as needed
		if (m_physEntity != nullptr)
		{
			m_physEntity->Render(renderer);
		}
	}
}

void Fireworks::Configure(float age, Vector3 inheritVel, Vector3 maxVel, Vector3 minVel, bool lastRound)
{
	m_age = age;
	m_lastRound = lastRound;
	m_minVel = minVel;
	m_maxVel = maxVel;
	m_physEntity->SetLinearVelocity(inheritVel + GetRandomVector3(m_minVel, m_maxVel));
	m_physEntity->SetMass(1.f);
	m_physEntity->SetDamping(true);
	m_physEntity->SetNetForce(Vector3(0.f, -.5f, 0.f));
	m_physEntity->SetNetForcePersistent(true);
}
