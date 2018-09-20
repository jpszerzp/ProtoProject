#pragma once

struct PhysicsMaterial
{
	float m_density;
	float m_elasticity = 1.f;
	float m_frictionCoef = 0.f;
	float m_gravityScale = 1.f;
};