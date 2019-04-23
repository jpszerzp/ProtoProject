#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

PFNGLCLEARPROC glClear = nullptr;
PFNGLCLEARCOLORPROC glClearColor = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLDETACHSHADERPROC glDetachShader = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDRAWARRAYSPROC glDrawArrays = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLGENSAMPLERSPROC glGenSamplers = nullptr;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri = nullptr;
PFNGLDELETESAMPLERSPROC glDeleteSamplers = nullptr;
PFNGLBINDSAMPLERPROC glBindSampler = nullptr;
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLBINDTEXTUREPROC glBindTexture = nullptr;
PFNGLENABLEPROC glEnable = nullptr;
PFNGLPIXELSTOREIPROC glPixelStorei = nullptr;
PFNGLGENTEXTURESPROC glGenTextures = nullptr;
PFNGLTEXIMAGE2DPROC glTexImage2D = nullptr;
PFNGLBLENDFUNCPROC glBlendFunc = nullptr;
PFNGLGETERRORPROC glGetError = nullptr;
PFNGLDEPTHFUNCPROC glDepthFunc = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = nullptr;
PFNGLDRAWBUFFERSPROC glDrawBuffers = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = nullptr;
PFNGLDEPTHMASKPROC glDepthMask = nullptr;
PFNGLCLEARDEPTHFPROC glClearDepthf = nullptr;
PFNGLTEXSTORAGE2DPROC glTexStorage2D = nullptr;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D = nullptr;
PFNGLLINEWIDTHPROC glLineWidth = nullptr;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = nullptr;
PFNGLDRAWELEMENTSPROC glDrawElements = nullptr;
PFNGLUNIFORM1FVPROC glUniform1fv = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
PFNGLPOLYGONMODEPROC glPolygonMode = nullptr;
PFNGLPOINTSIZEPROC glPointSize = nullptr;
PFNGLFRONTFACEPROC glFrontFace = nullptr;
PFNGLCULLFACEPROC glCullFace = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = nullptr;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = nullptr;
PFNGLBLENDEQUATIONPROC glBlendEquation = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = nullptr;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv = nullptr;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName = nullptr;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = nullptr;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName = nullptr;
PFNGLDELETETEXTURESPROC glDeleteTextures = nullptr;
PFNGLDISABLEPROC glDisable = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;
PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf = nullptr;
PFNGLTEXPARAMETERIPROC glTexParameteri = nullptr;
PFNGLDRAWBUFFERPROC glDrawBuffer = nullptr;
PFNGLSTENCILOPPROC glStencilOp = nullptr;
PFNGLSTENCILMASKPROC glStencilMask = nullptr;
PFNGLSTENCILFUNCPROC glStencilFunc = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;

PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

//------------------------------------------------------------------------
static HGLRC CreateOldRenderContext( HDC hdc ) 
{
	// Setup the output to be able to render how we want
	// (in our case, an RGBA (4 bytes per channel) output that supports OpenGL
	// and is double buffered
	PIXELFORMATDESCRIPTOR pfd;
	memset( &pfd, 0, sizeof(pfd) ); 
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 0; // 24; Depth/Stencil handled by FBO
	pfd.cStencilBits = 0; // 8; DepthStencil handled by FBO
	pfd.iLayerType = PFD_MAIN_PLANE; // ignored now according to MSDN

	// Find a pixel format that matches our search criteria above. 
	int pixel_format = ::ChoosePixelFormat( hdc, &pfd );
	if ( pixel_format == NULL ) {
		return NULL; 
	}

	// Set our HDC to have this output. 
	if (!::SetPixelFormat( hdc, pixel_format, &pfd )) {
		return NULL; 
	}

	// Create the context for the HDC
	HGLRC context = wglCreateContext( hdc );
	if (context == NULL) {
		return NULL; 
	}

	// return the context; 
	return context; 
}

//------------------------------------------------------------------------
// Creates a real context as a specific version (major.minor)
static HGLRC CreateRealRenderContext( HDC hdc, int major, int minor ) 
{
	// So similar to creating the temp one - we want to define 
	// the style of surface we want to draw to.  But now, to support
	// extensions, it takes key_value pairs
	int const format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,    // The rc will be used to draw to a window
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,    // ...can be drawn to by GL
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,     // ...is double buffered
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // ...uses a RGBA texture
		WGL_COLOR_BITS_ARB, 24,             // 24 bits for color (8 bits per channel)
											// WGL_DEPTH_BITS_ARB, 24,          // if you wanted depth a default depth buffer...
											// WGL_STENCIL_BITS_ARB, 8,         // ...you could set these to get a 24/8 Depth/Stencil.
											NULL, NULL,                         // Tell it we're done.
	};

	// Given the above criteria, we're going to search for formats
	// our device supports that give us it.  I'm allowing 128 max returns (which is overkill)
	size_t const MAX_PIXEL_FORMATS = 128;
	int formats[MAX_PIXEL_FORMATS];
	int pixel_format = 0;
	UINT format_count = 0;

	BOOL succeeded = wglChoosePixelFormatARB( hdc, 
		format_attribs, 
		nullptr, 
		MAX_PIXEL_FORMATS, 
		formats, 
		(UINT*)&format_count );

	if (!succeeded) {
		return NULL; 
	}

	// Loop through returned formats, till we find one that works
	for (unsigned int i = 0; i < format_count; ++i) {			
		pixel_format = formats[i];
		succeeded = SetPixelFormat( hdc, pixel_format, NULL ); // same as the temp context; 
		if (succeeded) {
			DebuggerPrintf("found one that works!");
			break;
		} else {
			DWORD error = GetLastError();
			//Logf( "Failed to set the format: %u", error ); 
			DebuggerPrintf( "Failed to set the format: %u", error );
		}
	}

	if (!succeeded) {
		return NULL; 
	}

	// Okay, HDC is setup to the rihgt format, now create our GL context

	// First, options for creating a debug context (potentially slower, but 
	// driver may report more useful errors). 
	int context_flags = 0; 
#if defined(_DEBUG)
	context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB; 
#endif

	// describe the context
	int const attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,                             // Major GL Version
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,                             // Minor GL Version
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,   // Restrict to core (no compatibility)
		WGL_CONTEXT_FLAGS_ARB, context_flags,                             // Misc flags (used for debug above)
		0, 0
	};

	// Try to create context
	HGLRC context = wglCreateContextAttribsARB( hdc, NULL, attribs );
	if (context == NULL) {
		return NULL; 
	}

	return context;
}

void BindNewWGLFunctions()
{
	GL_BIND_FUNCTION( wglGetExtensionsStringARB		); 
	GL_BIND_FUNCTION( wglChoosePixelFormatARB		); 
	GL_BIND_FUNCTION( wglCreateContextAttribsARB	);
}

void BindGLFunctions()
{
	GL_BIND_FUNCTION( glClear					); 
	GL_BIND_FUNCTION( glClearColor				); 
	GL_BIND_FUNCTION( glDeleteShader			);
	GL_BIND_FUNCTION( glCreateShader			);
	GL_BIND_FUNCTION( glShaderSource			);
	GL_BIND_FUNCTION( glCompileShader			);
	GL_BIND_FUNCTION( glGetShaderiv				);
	GL_BIND_FUNCTION( glGetShaderInfoLog		);
	GL_BIND_FUNCTION( glCreateProgram			);
	GL_BIND_FUNCTION( glAttachShader			);
	GL_BIND_FUNCTION( glLinkProgram				);
	GL_BIND_FUNCTION( glGetProgramiv			);
	GL_BIND_FUNCTION( glDeleteProgram			);
	GL_BIND_FUNCTION( glDetachShader			);
	GL_BIND_FUNCTION( glGetProgramInfoLog		);
	GL_BIND_FUNCTION( glDeleteBuffers			);
	GL_BIND_FUNCTION( glGenBuffers				);
	GL_BIND_FUNCTION( glBindBuffer				);
	GL_BIND_FUNCTION( glBufferData				);
	GL_BIND_FUNCTION( glGetAttribLocation		);
	GL_BIND_FUNCTION( glEnableVertexAttribArray );
	GL_BIND_FUNCTION( glVertexAttribPointer		);
	GL_BIND_FUNCTION( glUseProgram				);
	GL_BIND_FUNCTION( glDrawArrays				);
	GL_BIND_FUNCTION( glGenVertexArrays			);
	GL_BIND_FUNCTION( glBindVertexArray			);
	GL_BIND_FUNCTION( glGetUniformLocation		);
	GL_BIND_FUNCTION( glUniformMatrix4fv		);
	GL_BIND_FUNCTION( glGenSamplers				);
	GL_BIND_FUNCTION( glSamplerParameteri		);
	GL_BIND_FUNCTION( glDeleteSamplers			);
	GL_BIND_FUNCTION( glBindSampler				);
	GL_BIND_FUNCTION( glActiveTexture			);
	GL_BIND_FUNCTION( glBindTexture				);
	GL_BIND_FUNCTION( glEnable					);
	GL_BIND_FUNCTION( glPixelStorei				);
	GL_BIND_FUNCTION( glGenTextures				);
	GL_BIND_FUNCTION( glTexImage2D				);
	GL_BIND_FUNCTION( glBlendFunc				);
	GL_BIND_FUNCTION( glGetError				);
	GL_BIND_FUNCTION( glDepthFunc				);
	GL_BIND_FUNCTION( glDeleteFramebuffers		);
	GL_BIND_FUNCTION( glGenFramebuffers			);
	GL_BIND_FUNCTION( glBindFramebuffer			);
	GL_BIND_FUNCTION( glFramebufferTexture		);
	GL_BIND_FUNCTION( glDrawBuffers				);
	GL_BIND_FUNCTION( glCheckFramebufferStatus	);
	GL_BIND_FUNCTION( glBlitFramebuffer			);
	GL_BIND_FUNCTION( glDepthMask				);
	GL_BIND_FUNCTION( glClearDepthf				);
	GL_BIND_FUNCTION( glTexStorage2D			);
	GL_BIND_FUNCTION( glTexSubImage2D			);
	GL_BIND_FUNCTION( glLineWidth				);
	GL_BIND_FUNCTION( glBindBufferBase			);
	GL_BIND_FUNCTION( glGetUniformBlockIndex	);
	GL_BIND_FUNCTION( glUniformBlockBinding		);
	GL_BIND_FUNCTION( glDrawElements			);
	GL_BIND_FUNCTION( glUniform1fv				);
	GL_BIND_FUNCTION( glUniform2fv				);
	GL_BIND_FUNCTION( glUniform3fv				);
	GL_BIND_FUNCTION( glUniform4fv				);
	GL_BIND_FUNCTION( glPolygonMode				);
	GL_BIND_FUNCTION( glPointSize				);
	GL_BIND_FUNCTION( glFrontFace				);
	GL_BIND_FUNCTION( glCullFace				);
	GL_BIND_FUNCTION( glBlendEquationSeparate	);
	GL_BIND_FUNCTION( glBlendFuncSeparate		);
	GL_BIND_FUNCTION( glBlendEquation			);
	GL_BIND_FUNCTION( glGetActiveUniformBlockiv	);
	GL_BIND_FUNCTION( glGetActiveUniformsiv		);
	GL_BIND_FUNCTION( glGetActiveUniformName	);
	GL_BIND_FUNCTION( glGetActiveUniform		);
	GL_BIND_FUNCTION( glStencilOp				);
	GL_BIND_FUNCTION( glStencilMask				);
	GL_BIND_FUNCTION( glStencilFunc				);
	GL_BIND_FUNCTION( glUniform1i				);
	GL_BIND_FUNCTION( glGetActiveUniformBlockName	);
	GL_BIND_FUNCTION( glDeleteTextures				);
	GL_BIND_FUNCTION( glDisable						);
	GL_BIND_FUNCTION( glGenerateMipmap				);
	GL_BIND_FUNCTION( glSamplerParameterf			);
	GL_BIND_FUNCTION( glTexParameteri				);
	GL_BIND_FUNCTION( glDrawBuffer					);
}

bool AreGLFuncitonsBound()
{
	if (glClear							==		nullptr ||				
		glClearColor					==		nullptr ||		
		glDeleteShader					==		nullptr ||	
		glCreateShader					==		nullptr ||	
		glShaderSource					==		nullptr ||	
		glCompileShader					==		nullptr ||	
		glGetShaderiv					==		nullptr ||	
		glGetShaderInfoLog				==		nullptr ||	
		glCreateProgram					==		nullptr ||	
		glAttachShader					==		nullptr ||	
		glLinkProgram					==		nullptr ||	
		glGetProgramiv					==		nullptr ||	
		glDeleteProgram					==		nullptr ||	
		glDetachShader					==		nullptr ||	
		glGetProgramInfoLog				==		nullptr ||	
		glDeleteBuffers					==		nullptr ||	
		glGenBuffers					==		nullptr ||	
		glBindBuffer					==		nullptr ||	
		glBufferData					==		nullptr ||	
		glGetAttribLocation				==		nullptr ||	
		glEnableVertexAttribArray 		==		nullptr ||	
		glVertexAttribPointer			==		nullptr ||	
		glUseProgram					==		nullptr ||	
		glDrawArrays					==		nullptr ||	
		glGenVertexArrays				==		nullptr ||	
		glBindVertexArray				==		nullptr ||
		glGetUniformLocation			==		nullptr ||
		glUniformMatrix4fv				==		nullptr ||
		glGenSamplers					==		nullptr ||
		glSamplerParameteri				==		nullptr ||
		glDeleteSamplers				==		nullptr ||
		glBindSampler					==		nullptr ||
		glActiveTexture					==		nullptr ||
		glBindTexture					==		nullptr ||
		glEnable						==		nullptr ||
		glPixelStorei					==		nullptr ||
		glGenTextures					==		nullptr ||
		glTexImage2D					==		nullptr ||
		glBlendFunc						==		nullptr ||
		glGetError						==		nullptr ||
		glDepthFunc						==		nullptr ||
		glDeleteFramebuffers			==		nullptr ||
		glGenFramebuffers				==		nullptr ||
		glBindFramebuffer				==		nullptr ||
		glFramebufferTexture			==		nullptr ||
		glDrawBuffers					==		nullptr ||
		glCheckFramebufferStatus		==		nullptr ||
		glBlitFramebuffer				==		nullptr ||
		glDepthMask						==		nullptr ||
		glClearDepthf					==		nullptr ||
		glTexStorage2D					==		nullptr ||
		glTexSubImage2D					==		nullptr ||
		glLineWidth						==		nullptr ||
		glBindBufferBase				==		nullptr ||
		glGetUniformBlockIndex			==		nullptr	||
		glUniformBlockBinding			==		nullptr	||
		glDrawElements					==		nullptr	||
		glUniform1fv					==		nullptr	||
		glUniform2fv					==		nullptr	||
		glUniform3fv					==		nullptr	||
		glUniform4fv					==		nullptr	||
		glPolygonMode					==		nullptr	||
		glPointSize						==		nullptr	||
		glFrontFace						==		nullptr	||
		glCullFace						==		nullptr	||
		glBlendEquationSeparate			==		nullptr	||
		glBlendFuncSeparate				==		nullptr	||
		glBlendEquation					==		nullptr	||
		glGetActiveUniformBlockiv		==		nullptr	||
		glGetActiveUniformsiv			==		nullptr	||
		glGetActiveUniformName			==		nullptr	||
		glGetActiveUniform				==		nullptr	||
		glGetActiveUniformBlockName		==		nullptr	||
		glDeleteTextures				==		nullptr	||
		glDisable						==		nullptr	||
		glGenerateMipmap				==		nullptr	||
		glSamplerParameterf				==		nullptr	||
		glTexParameteri					==		nullptr	||
		glDrawBuffer					==		nullptr	||
		glStencilOp						==		nullptr	||
		glStencilMask					==		nullptr	||
		glStencilFunc					==		nullptr	||
		glUniform1i						==		nullptr	||
		wglGetExtensionsStringARB		==		nullptr ||
		wglChoosePixelFormatARB			==		nullptr ||
		wglCreateContextAttribsARB		==		nullptr 
		)
	{
		return false;
	}

	return true;
}

void GLShutdown()
{
	wglMakeCurrent( gHDC, NULL ); 

	::wglDeleteContext( gGLContext ); 
	::ReleaseDC( gGLwnd, gHDC ); 

	gGLContext = NULL; 
	gHDC = NULL;
	gGLwnd = NULL; 

	::FreeLibrary( gGLLibrary ); 
}

// Rendering startup - called after we have created our window
// error checking has been removed for brevity, but you should check
// the return values at each step.
bool RenderStartup( HWND hwnd, float startupWidth, float startupHeight ) 
{
	// load and get a handle to the opengl dll (dynamic link library)
	gGLLibrary = ::LoadLibraryA( "opengl32.dll" ); 

	// Get the Device Context (DC) - how windows handles the interace to rendering devices
	// This "acquires" the resource - to cleanup, you must have a ReleaseDC(hwnd, hdc) call. 
	HDC hdc = ::GetDC( hwnd );       

	// use the DC to create a rendering context (handle for all OpenGL state - like a pointer)
	// This should be very simiilar to SD1
	HGLRC temp_context = CreateOldRenderContext( hdc ); 

	::wglMakeCurrent( hdc, temp_context ); 
	BindNewWGLFunctions();  // find the functions we'll need to create the real context; 

	// create the real context, using opengl version 4.2
	HGLRC real_context = CreateRealRenderContext( hdc, 4, 2 ); 

	// Set and cleanup
	::wglMakeCurrent( hdc, real_context ); 
	::wglDeleteContext( temp_context ); 

	// Bind all our OpenGL functions we'll be using.
	BindGLFunctions();
	bool areGLFunctionsBound = AreGLFuncitonsBound();
	ASSERT_OR_DIE(areGLFunctionsBound, "Some GL functions are not bound!");

	// set the globals
	gGLwnd = hwnd;
	gHDC = hdc; 
	gGLContext = real_context; 

	Renderer::GetInstance()->PostStartup(startupWidth, startupHeight);

	return true;
}

//------------------------------------------------------------------------
bool GLCheckError( char const *file, int line )
{
#if defined(_DEBUG)
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		DebuggerPrintf("GL ERROR [0x%04x] at [%s(%i)]\n", error, file, line);
		return true; 
	}
#endif
	return false; 
}

//------------------------------------------------------------------------
bool GLFailed()
{
	return GL_CHECK_ERROR(); 
}

//------------------------------------------------------------------------
bool GLSucceeded()
{
	return !GLFailed();
}