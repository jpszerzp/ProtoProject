#pragma once

#include "Engine/Math/Vector3.hpp"

class ParticleEmitter
{
public:
	float m_spawnRate;				// how many particles per seconds
	float m_rotation;
	float m_azimuth;
	bool m_on;
	float m_particleLifetime;
	int m_particleCount;

	const static int MAX_PARTICLES = 1000;
	constexpr static float RAD = 1.f;

	Vector3 m_pos;

public:
	ParticleEmitter(Vector3 pos, float spawnRate, float rotation, float azimuth, float particleLifetime);
	~ParticleEmitter();

	Vector3 SpawnVelocity();
};