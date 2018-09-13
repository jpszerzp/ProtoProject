#include "Engine/Renderer/Sampler.hpp"


Sampler::Sampler(bool mipmap) : m_samplerHandler(NULL)
{
	CreateSampler(mipmap);
}


Sampler::~Sampler()
{
	DestroySampler();
}


bool Sampler::CreateSampler(bool mipmap)
{
	// create the sampler handle if needed; 
	if (m_samplerHandler == NULL) {
		glGenSamplers( 1, &m_samplerHandler ); 
		if (m_samplerHandler == NULL) {
			return false; 
		}
	}

	// setup wrapping
	glSamplerParameteri( m_samplerHandler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );	
	glSamplerParameteri( m_samplerHandler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );  
	glSamplerParameteri( m_samplerHandler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );  

	if (!mipmap)
	{
		glSamplerParameteri( m_samplerHandler, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glSamplerParameteri( m_samplerHandler, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	else
	{
		// filtering; (trilinear) - really - if you want mips with nearest, siwtch the GL_LINEAR_* to GL_NEAREST_*
		glSamplerParameteri( m_samplerHandler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );         // Default: GL_LINEAR
		glSamplerParameteri( m_samplerHandler, GL_TEXTURE_MAG_FILTER, GL_LINEAR );     

		glSamplerParameterf( m_samplerHandler, GL_TEXTURE_MIN_LOD, -1000.f ); 
		glSamplerParameterf( m_samplerHandler, GL_TEXTURE_MAX_LOD, 1000.f ); 
	}

	return true; 
}


void Sampler::DestroySampler()
{
	if (m_samplerHandler != NULL) {
		glDeleteSamplers( 1, &m_samplerHandler ); 
		m_samplerHandler = NULL; 
	}
}