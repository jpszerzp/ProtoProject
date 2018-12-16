#pragma once

#include "Engine/Renderer/external/gl/glcorearb.h"
#include "Engine/Renderer/external/gl/glext.h"
#include "Engine/Renderer/external/gl/wglext.h"

#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

// Four needed variables.  Globals or private members of Renderer are fine; 
static HMODULE gGLLibrary  = NULL; 
static HWND gGLwnd         = NULL;    // window our context is attached to; 
static HDC gHDC            = NULL;    // our device context
static HGLRC gGLContext    = NULL;    // our rendering context; 


// Use this to deduce type of the pointer so we can cast; 
template <typename T>
bool wglGetTypedProcAddress( T *out, char const *name ) 
{
	// Grab the function from the currently bound render contect
	// most opengl 2.0+ features will be found here
	*out = (T) wglGetProcAddress(name); 

	if ((*out) == nullptr) {
		// if it is not part of wgl (the device), then attempt to get it from the GLL library
		// (most OpenGL functions come from here)
		*out = (T) GetProcAddress( gGLLibrary, name); 
	}

	return (*out != nullptr); 
}

#define GL_BIND_FUNCTION(f)      wglGetTypedProcAddress( &f, #f )
#define GL_CHECK_ERROR()		 GLCheckError( __FILE__, __LINE__ )

extern PFNGLCLEARPROC glClear;
extern PFNGLCLEARCOLORPROC glClearColor;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLDRAWARRAYSPROC glDrawArrays;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLGENSAMPLERSPROC glGenSamplers;
extern PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
extern PFNGLDELETESAMPLERSPROC glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC glBindSampler;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLBINDTEXTUREPROC glBindTexture;
extern PFNGLENABLEPROC glEnable;
extern PFNGLPIXELSTOREIPROC glPixelStorei;
extern PFNGLGENTEXTURESPROC glGenTextures;
extern PFNGLTEXIMAGE2DPROC glTexImage2D;
extern PFNGLBLENDFUNCPROC glBlendFunc;
extern PFNGLGETERRORPROC glGetError;
extern PFNGLDEPTHFUNCPROC glDepthFunc;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLDEPTHMASKPROC glDepthMask;
extern PFNGLCLEARDEPTHFPROC glClearDepthf;
extern PFNGLTEXSTORAGE2DPROC glTexStorage2D;
extern PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
extern PFNGLLINEWIDTHPROC glLineWidth;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
extern PFNGLDRAWELEMENTSPROC glDrawElements;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLPOLYGONMODEPROC glPolygonMode;
extern PFNGLPOINTSIZEPROC glPointSize;
extern PFNGLFRONTFACEPROC glFrontFace;
extern PFNGLCULLFACEPROC glCullFace;
extern PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
extern PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
extern PFNGLBLENDEQUATIONPROC glBlendEquation;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv; 
extern PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
extern PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName;
extern PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName;
extern PFNGLDELETETEXTURESPROC glDeleteTextures;
extern PFNGLDISABLEPROC glDisable;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf;
extern PFNGLTEXPARAMETERIPROC glTexParameteri;
extern PFNGLDRAWBUFFERPROC glDrawBuffer;
extern PFNGLSTENCILOPPROC glStencilOp;
extern PFNGLSTENCILMASKPROC glStencilMask;
extern PFNGLSTENCILFUNCPROC glStencilFunc;

extern PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

extern bool RenderStartup( HWND hwnd, float startupWidth, float startupHeight );
extern void GLShutdown();
extern bool GLCheckError(char const *file, int line);
extern bool GLFailed();
extern bool GLSucceeded();