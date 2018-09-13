#include "Engine/Math/Particle.hpp"

Particle::Particle()
{

}

Particle::Particle(Vector3 pos, float size)
{
	m_position = pos;
	m_size = size;
}

Particle::~Particle()
{

}

void Particle::Translate(Vector3 translation)
{
	m_position += translation;
}

