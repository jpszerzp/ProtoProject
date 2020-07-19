#include "Engine/Core/ParticleEmitter.hpp"
#include "Engine/Math/MathUtils.hpp"

ParticleEmitter::ParticleEmitter(Vector3 pos, float spawnRate, float rotation, float azimuth, float particleLifetime)
{
	m_pos = pos;
	m_spawnRate = spawnRate;
	m_rotation = rotation;
	m_azimuth = azimuth;
	m_on = true;
	m_particleLifetime = particleLifetime;
}


ParticleEmitter::~ParticleEmitter()
{

}


Vector3 ParticleEmitter::SpawnVelocity()
{
	float rot = GetRandomFloatInRange(0.f, m_rotation);
	float azi = GetRandomFloatInRange(0.f, m_azimuth);

	Vector3 direction = PolarToCartesian(RAD, rot, azi);
	direction.NormalizeAndGetLength();

	return direction;
}