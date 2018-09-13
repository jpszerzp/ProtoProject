#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/GLFunctions.hpp"

class FrameBuffer
{
public:
	FrameBuffer(); 
	~FrameBuffer();

	// should just update members
	// finalize does the actual binding
	void SetColorTarget( Texture* color_target )	{ m_color_target = color_target; }
	void SetDepthStencilTarget( Texture* depth_target )		{ m_depth_stencil_target = depth_target; }

	GLuint GetHandle() const { return m_handle; }

	// Setup the the GL framebuffer - called before us. 
	// Make sure this only does work if the targets have changed.
	bool Finalize(); 

public:
	GLuint m_handle; 
	Texture* m_color_target; 
	Texture* m_depth_stencil_target; 
};