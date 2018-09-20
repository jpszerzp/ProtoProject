#pragma once

struct MassData
{
	// linear
	float m_mass;
	float m_invMass;

	// angular
	float m_inertia;
	float m_invInertia;
};

struct MassData3
{
	float m_mass;
	float m_invMass;

	Matrix33 m_tensor;
	Matrix33 m_invTensor;
};