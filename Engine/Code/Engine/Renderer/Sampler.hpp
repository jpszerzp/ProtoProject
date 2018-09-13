#pragma once

#include "Engine/Renderer/GLFunctions.hpp"

class Sampler
{
private:
	GLuint m_samplerHandler;

public:
	Sampler(bool mipmap = false);
	~Sampler();

	bool CreateSampler(bool mipmap);
	void DestroySampler();

	GLuint GetHandle() { return m_samplerHandler; }
};