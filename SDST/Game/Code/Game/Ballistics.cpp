#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Ballistics.hpp"

Ballistics::Ballistics()
{

}

Ballistics::Ballistics(eBallisticsType type, Vector3 pos)
	: Point(pos, Vector3::ZERO, 10.f, Rgba::WHITE, "point_pcu", "default", MOVE_DYNAMIC, BODY_PARTICLE)
{
	m_type = type;
	ConfigureBallisticsType();
}

Ballistics::Ballistics(eBallisticsType type, Vector3 pos, bool frozen, Rgba color)
	: Point(pos, Vector3::ZERO, 10.f, color, "point_pcu", "default", MOVE_DYNAMIC, BODY_PARTICLE)
{
	m_physEntity->SetFrozen(frozen);
	m_type = type;
	ConfigureBallisticsType();
}

Ballistics::~Ballistics()
{

}

void Ballistics::ConfigureBallisticsType()
{
	switch (m_type)
	{
	case PISTOL:
	{
		m_physEntity->SetMass(2.f); // 2.0kg
		m_physEntity->SetLinearVelocity(0.f, 0.f, 35.f); // 35m/s
		m_physEntity->SetNetForce(0.f, -1.f, 0.f);
		m_physEntity->SetDamping(.99f);
	}
		break;
	case ARTILLERY:
	{
		m_physEntity->SetMass(200.f); 
		m_physEntity->SetLinearVelocity(0.f, 0.f, 50.f);
		m_physEntity->SetNetForce(0.f, -20.f, 0.f);
		m_physEntity->SetDamping(.99f);
	}
		break;
	case FIREBALL:
	{
		m_physEntity->SetMass(1.f); 
		m_physEntity->SetLinearVelocity(0.f, 0.f, 10.f);
		m_physEntity->SetNetForce(0.f, .6f, 0.f);
		m_physEntity->SetDamping(.9f);
	}
		break;
	case LASER:
	{
		m_physEntity->SetMass(.1f); 
		m_physEntity->SetLinearVelocity(0.f, 0.f, 100.f);
		m_physEntity->SetNetForce(0.f, 0.f, 0.f);
		m_physEntity->SetDamping(.99f);
	}
		break;
	case FREEFALL:
	{
		m_physEntity->SetMass(1.f);
		m_physEntity->SetLinearVelocity(Vector3::ZERO);
		m_physEntity->SetNetForce(Vector3::GRAVITY / 2.f);
		m_physEntity->SetDamping(.99f);
	}
		break;
	case NUM_BALLISTICS_TYPE:
		break;
	default: 
		break;
	}
}

void Ballistics::Update(float deltaTime)
{
	UpdateInput(deltaTime);

	// Ballistics is supposed to be physics driven by nature, hence I do not check it here
	m_physEntity->Integrate(deltaTime);
	m_physEntity->UpdateEntitiesTransforms();
	m_physEntity->UpdateEntityPrimitive();
	m_physEntity->UpdateBoundPrimitives();

	// physics driven, hence update renderable transform
	m_renderable->m_transform = m_physEntity->GetEntityTransform();

	UpdateBasis();
}

void Ballistics::UpdateInput(float deltaTime)
{
	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_P))
	{
		bool frostness = m_physEntity->IsFrozen();
		m_physEntity->SetFrozen(!frostness);
	}
}

