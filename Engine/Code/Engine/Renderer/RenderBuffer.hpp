#pragma once

#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Core/EngineCommon.hpp"

class RenderBuffer
{
private:
	size_t m_bufferSize;
	GLuint m_handle;
	
public:
	RenderBuffer();
	~RenderBuffer();

	bool CopyToGPU( size_t const byte_count, void const *data );

	// getters
	GLuint GetHandle() { return m_handle; }
};


class IndexBuffer : public RenderBuffer
{
public:
	uint m_indexStride; 
	uint m_indexCount; 
};


class VertexBuffer : public RenderBuffer
{
public:
	// meta
	uint m_vertexCount;  
	uint m_vertexStride; 
};


class UniformBuffer : public RenderBuffer
{

};