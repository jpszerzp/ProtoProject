#pragma once

#include "Engine/Core/Image.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"

enum eTextureCubeSide
{
	TEXTURE_CUBE_RIGHT,
	TEXTURE_CUBE_LEFT,
	TEXTURE_CUBE_TOP,
	TEXTURE_CUBE_BOTTOM,
	TEXTURE_CUBE_FRONT,
	TEXTURE_CUBE_BACK,
	NUM_TEXTURE_CUBE_SIDE
};

class TextureCube
{
public:
	TextureCube();
	~TextureCube();

	bool FromImages(const Image* images);
	bool FromImage(const Image& image);
	bool FromImage(const char* fn);

	inline uint GetWidth() const { return m_size; }
	inline uint GetHeight() const { return m_size; }

	inline bool isValid() const { return (m_handle != NULL); }

	inline uint GetHandle() const { return m_handle; }

	void SetSampler(Sampler* sampler) { m_sampler = sampler; }

public:
	uint m_size;
	eTextureFormat m_format;

	// GL
	uint m_handle;
	
	Sampler* m_sampler;
};