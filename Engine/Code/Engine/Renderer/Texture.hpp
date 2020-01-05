//-----------------------------------------------------------------------------------------------
// Texture.hpp
//
#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include <string>

enum eTextureFormat 
{
	TEXTURE_FORMAT_UNKNOWN,
	TEXTURE_FORMAT_RGBA8, // default color format
	TEXTURE_FORMAT_RGB8,
	TEXTURE_FORMAT_D24S8, 
	NUM_TEXTURE_FORMAT
}; 

//---------------------------------------------------------------------------
class Texture
{
	friend class Renderer; // Textures are managed by a Renderer instance
	friend class SpriteAnimSetDef;

private:
	Texture();
	Texture( const std::string& imageFilePath, bool mipmap = false ); // Use renderer->CreateOrGetTexture() instead!
	void PopulateFromData( unsigned char* imageData, const IntVector2& texelSize, int numComponents, bool mipmap );

private:
	unsigned int	m_textureID;
	IntVector2		m_dimensions;		
	eTextureFormat  m_format;
	Sampler*		m_sampler;
	bool			m_mipmap;
	unsigned char*  m_data;

public:
	static Texture* CreateCompatible(Texture* subject);

	unsigned int	GetHandle() const { return m_textureID; }
	IntVector2		GetDimension() const { return m_dimensions; }
	eTextureFormat	GetFormat() const { return m_format; }
	Sampler*		GetSampler() const { return m_sampler; }
	unsigned char*  GetData() const {return m_data;}

	void SetDimensionWidth(int width) { m_dimensions.x = width; }
	void SetDimensionHeight(int height) { m_dimensions.y = height; }
	void SetFormat(eTextureFormat format) { m_format = format; }
	void SetSampler(Sampler* sampler)	{ m_sampler = sampler; }

	void FlipImageDataY(unsigned char* rawImageData, int numComponents);
	bool CreateRenderTarget( uint width, uint height, eTextureFormat fmt );
};


