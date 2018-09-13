#include "Engine/Core/Image.hpp"

//#define STB_IMAGE_IMPLEMENTATION    
#include "ThirdParty/stb/stb_image.h"

Image::Image(const std::string& imageFilePath) : m_dimensions( 0, 0 )
{
	int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
	int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk, and create an OpenGL texture instance from it
	unsigned char* imageData = stbi_load( imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );

	int index = 0;
	Rgba texel;

	for (int texelCount = 0; texelCount < m_dimensions.x * m_dimensions.y; ++texelCount)
	{
		if (numComponents == 3)
		{
			texel = Rgba(imageData[index], imageData[index + 1], imageData[index + 2]);
			index = index + 3;
		}
		else
		{
			texel = Rgba(imageData[index], imageData[index + 1], imageData[index + 2], imageData[index + 3]);
			index = index + 4;
		}

		m_texels.push_back(texel);
	}

	m_components = numComponents;
	m_imageData = imageData;
}


Image::Image()
{

}


Image::~Image()
{
	stbi_image_free( m_imageData );
}


void* Image::GetBuffer(int x, int y) const
{
	int offset = (m_dimensions.x * y + x) * GetBytesPerPixel();
	return m_imageData + offset;
}