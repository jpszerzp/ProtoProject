//-----------------------------------------------------------------------------------------------
// Texture.cpp
//
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//#define STB_IMAGE_IMPLEMENTATION    
#include "ThirdParty/stb/stb_image.h"

Texture::Texture()
	: m_textureID(0)
	, m_dimensions(0, 0)
{

}

//-----------------------------------------------------------------------------------------------
// Called only by the Renderer.  Use renderer->CreateOrGetTexture() to instantiate textures.
//
Texture::Texture( const std::string& imageFilePath, bool mipmap )
	: m_textureID( 0 )
	, m_dimensions( 0, 0 )
	, m_mipmap(mipmap)
{
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk, and create an OpenGL texture instance from it
	//unsigned char* imageData = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );
	m_data = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );
	FlipImageDataY(m_data, numComponents);
	PopulateFromData( m_data, m_dimensions, numComponents, mipmap );
	stbi_image_free( m_data );
}


void Texture::FlipImageDataY(unsigned char* rawImageData, int numComponents)
{
	int rgbaWidth = m_dimensions.x * numComponents;

	for (int yDimCount = 0; yDimCount < m_dimensions.y; ++yDimCount)
	{
		int processedRowIndex = yDimCount;
		int processedRowIndexReverse = (m_dimensions.y - yDimCount) - 1;

		if (processedRowIndex >= processedRowIndexReverse)
		{
			break;
		}

		for (int xDimCount = 0; xDimCount < rgbaWidth; ++xDimCount)
		{
			int swapIndexInTopRegion = processedRowIndex * rgbaWidth + xDimCount;
			int swapIndexInBottomRegion = processedRowIndexReverse * rgbaWidth + xDimCount;

			unsigned char temp = 0;
			temp = rawImageData[swapIndexInTopRegion];
			rawImageData[swapIndexInTopRegion] = rawImageData[swapIndexInBottomRegion];
			rawImageData[swapIndexInBottomRegion] = temp;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Creates a texture identity on the video card, and populates it with the given image texel data
//
void Texture::PopulateFromData( unsigned char* imageData, const IntVector2& texelSize, int numComponents, bool mipmap )
{
	m_dimensions = texelSize;

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	GLenum internalFormat = GL_RGBA8;
	if( numComponents == 3 )
	{
		bufferFormat = GL_RGB;
		internalFormat = GL_RGB8;
	}

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*) &m_textureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_textureID );

	// create the gpu buffer
	// note: only this is needed for render targets
	if (!mipmap)
	{
		glTexStorage2D( GL_TEXTURE_2D,
			1,               // number of levels (mip-layers)
			internalFormat, // how is the memory stored on the GPU
			m_dimensions.x, m_dimensions.y); // dimenions
	}
	else
	{
		/*
		glTexStorage2D(GL_TEXTURE_2D,
			0, 
			internalFormat,
			m_dimensions.x, m_dimensions.y);
			*/
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
			m_dimensions.x, m_dimensions.y, 0,
			GL_RGB, GL_UNSIGNED_BYTE, m_data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	// copies cpu memory to the gpu - needed for texture resources
	glTexSubImage2D( GL_TEXTURE_2D,
		0,             // mip layer we're copying to
		0, 0,          // offset
		m_dimensions.x, m_dimensions.y, // dimensions
		bufferFormat,			// which channels exist in the CPU buffer
		GL_UNSIGNED_BYTE,		 // how are those channels stored
		imageData );			// cpu buffer to copy;

	GL_CHECK_ERROR();
}


Texture* Texture::CreateCompatible(Texture* subject)
{
	Texture* targetTex = new Texture();
	targetTex->CreateRenderTarget( subject->GetDimension().x, subject->GetDimension().y, subject->GetFormat() );
	return targetTex; 
}


//------------------------------------------------------------------------
bool Texture::CreateRenderTarget( uint width, uint height, eTextureFormat fmt )
{
	// generate the link to this texture
	glGenTextures( 1, &m_textureID ); 
	if (m_textureID == NULL) {
		return false; 
	}

	TODO("Add a TextureFormatToGLFormats( GLenum*, GLenum*, GLenum*, eTextureFormat ) when more texture formats are required;");
	GLenum internal_format = GL_RGBA8; 
	GLenum channels = GL_RGBA;  
	GLenum pixel_layout = GL_UNSIGNED_BYTE;  
	if (fmt == TEXTURE_FORMAT_D24S8) {
		internal_format = GL_DEPTH24_STENCIL8; 
		channels = GL_DEPTH_STENCIL; 
		pixel_layout = GL_UNSIGNED_INT_24_8; 
	}

	// Copy the texture - first, get use to be using texture unit 0 for this; 
	glActiveTexture( GL_TEXTURE0 ); 
	glBindTexture( GL_TEXTURE_2D, m_textureID );    // bind our texture to our current texture unit (0)

	// create the gpu buffer
	// note: only this is needed for render targets
	glTexStorage2D( GL_TEXTURE_2D,
		1,               // number of levels (mip-layers)
		internal_format, // how is the memory stored on the GPU
		width, height ); // dimenions

	// cleanup after myself; 
	glBindTexture( GL_TEXTURE_2D, NULL ); // unset it; 

	// make sure it suceeded
	GL_CHECK_ERROR();

	// Save this all off
	m_dimensions.x = width;  
	m_dimensions.y = height; 

	m_format = fmt; // I save the format with the texture
					// for sanity checking.

	// great, success
	return true; 
}