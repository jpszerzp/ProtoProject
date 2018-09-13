#pragma once

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"

#include <string>
#include <vector>

class Image
{
public:
	explicit Image( const std::string& imageFilePath );
	explicit Image();
	~Image();
	Rgba			   GetTexel( int x, int y ) const { return m_texels[( y * m_dimensions.x ) + x]; } 			// (0,0) is top-left
	std::vector<Rgba>& GetTexels() { return m_texels; }
	IntVector2		   GetImageDimension() const { return m_dimensions; }
	void*			   GetBuffer(int x, int y) const;
	int				   GetBytesPerPixel() const { return m_components; }

	void SetTexel( int x, int y, const Rgba& color ) { m_texels[( y * m_dimensions.x ) + x] = color; }

private:
	IntVector2			m_dimensions;
	std::vector< Rgba >	m_texels; // ordered left-to-right, then down, from [0]=(0,0) at top-left
	unsigned char*		m_imageData;
	int					m_components;
};