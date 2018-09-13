#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


FrameBuffer::FrameBuffer()
{
	m_handle = 0;

	glGenFramebuffers( 1, &m_handle ); 
}


FrameBuffer::~FrameBuffer()
{
	if (m_handle != NULL) 
	{
		glDeleteFramebuffers( 1, &m_handle ); 
		m_handle = NULL; 
	}

	//delete m_color_target;
	m_color_target = nullptr;

	//delete m_depth_stencil_target;
	m_depth_stencil_target = nullptr;
}


bool FrameBuffer::Finalize()
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_handle ); 

	// keep track of which outputs go to which attachments; 
	GLenum targets[1]; 

	// Bind a color target to an attachment point
	// and keep track of which locations to to which attachments. 
	glFramebufferTexture( GL_FRAMEBUFFER, 
		GL_COLOR_ATTACHMENT0 + 0, 
		m_color_target->GetHandle(), 0 ); 
	// 0 to to attachment 0
	targets[0] = GL_COLOR_ATTACHMENT0 + 0; 

	// Update target bindings
	glDrawBuffers( 1, targets ); 

	// Bind depth if available;
	if (m_depth_stencil_target == nullptr) {
		glFramebufferTexture( GL_FRAMEBUFFER, 
			GL_DEPTH_STENCIL_ATTACHMENT, 
			NULL, 0 ); 
	} else {
		glFramebufferTexture( GL_FRAMEBUFFER, 
			GL_DEPTH_STENCIL_ATTACHMENT, 
			m_depth_stencil_target->GetHandle(), 0 ); 
	}

	//GL_CHECK_ERROR();

	// Error Check - recommend only doing in debug
#if defined(_DEBUG)
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		DebuggerPrintf("Failed to create framebuffer:  %u", status);
		return false;
	}
#endif

	return true;
}