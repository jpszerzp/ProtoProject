#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

GLenum ToGLCubeSide(eTextureCubeSide side)
{
	switch (side)
	{
	case TEXTURE_CUBE_RIGHT:
		break;
	case TEXTURE_CUBE_LEFT:
		break;
	case TEXTURE_CUBE_TOP:
		break;
	case TEXTURE_CUBE_BOTTOM:
		break;
	case TEXTURE_CUBE_FRONT:
		break;
	case TEXTURE_CUBE_BACK:
		break;
	case NUM_TEXTURE_CUBE_SIDE:
		break;
	default:
		break;
	}

	return GL_NONE;
}


TextureCube::TextureCube()
{
	m_size = 0U;
	m_handle = 0U;
	m_format = TEXTURE_FORMAT_UNKNOWN;
	m_sampler = new Sampler();
}


TextureCube::~TextureCube()
{
	if (isValid())
	{
		glDeleteTextures(1, &m_handle);
		m_handle = NULL;
	}

	m_size = 0;
	m_format = TEXTURE_FORMAT_UNKNOWN;
}


static void BindImageToSide(int sideIdx, const Image& img,
	uint size, uint ox, uint oy, GLenum channels, GLenum pixelLayout)
{
	const void* ptr = img.GetBuffer(ox, oy);			
	glTexSubImage2D(
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideIdx,
		0,
		0, 0,
		size, size,
		channels,
		pixelLayout,
		ptr);					

	GL_CHECK_ERROR();
}


// Make cube with 6 separate images
bool TextureCube::FromImages(const Image*)
{
	return false;
}


bool TextureCube::FromImage(const Image& image)
{
	int width = image.GetImageDimension().x;
	int size = (int)(width / 4.f);

	ASSERT_RECOVERABLE(image.GetImageDimension().y == (size * 3), "image height does not fit");

	if (m_handle == NULL)
	{
		glGenTextures(1, (GLuint*)&m_handle);
		ASSERT_RECOVERABLE(isValid(), "texture cube is not valid");
	}

	m_size = size;

	GLenum iFormat;
	GLenum channels;

	if (image.GetBytesPerPixel() == 3)
	{
		m_format = TEXTURE_FORMAT_RGB8;
		iFormat = GL_RGB8;
		channels = GL_RGB;
	}
	else if (image.GetBytesPerPixel() == 4)
	{
		m_format = TEXTURE_FORMAT_RGBA8;
		iFormat = GL_RGBA8;
		channels = GL_RGBA;
	}

	GLenum pixelLayout = GL_UNSIGNED_BYTE;

	// Bind cube map 
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);
	glBindSampler(0, m_sampler->GetHandle());
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, iFormat, m_size, m_size);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

	// Bind image to each side
	BindImageToSide(0, image, m_size, m_size * 2, m_size * 1, channels, pixelLayout);
	BindImageToSide(1, image, m_size, m_size * 0, m_size * 1, channels, pixelLayout);
	BindImageToSide(2, image, m_size, m_size * 1, m_size * 0, channels, pixelLayout);
	BindImageToSide(3, image, m_size, m_size * 1, m_size * 2, channels, pixelLayout);
	BindImageToSide(4, image, m_size, m_size * 1, m_size * 1, channels, pixelLayout);
	BindImageToSide(5, image, m_size, m_size * 3, m_size * 1, channels, pixelLayout);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	return GLSucceeded();
}


bool TextureCube::FromImage(const char* fn)
{
	Image img = Image(fn);

	return FromImage(img);
}
