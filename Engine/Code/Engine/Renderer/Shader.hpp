#pragma once

#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/ShaderProperties.hpp"
#include "Engine/Renderer/Texture.hpp"

// These enums should be defined in a RendererTypes.hpp or Renderer.hpp, wherever you 
// do the obfuscation for blend modes and the like
enum eCullMode 
{
	CULLMODE_BACK,          // GL_BACK     glEnable(GL_CULL_FACE); glCullFace(GL_BACK); 
	CULLMODE_FRONT,         // GL_FRONT    glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); 
	CULLMODE_NONE,          // GL_NONE     glDisable(GL_CULL_FACE)
	NUM_CULL_MODE
};

enum eFillMode 
{
	FILLMODE_SOLID,         // GL_FILL     glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )
	FILLMODE_WIRE,          // GL_LINE     glPolygonMode( GL_FRONT_AND_BACK, GL_LINE )
	NUM_FILL_MODE
};

enum eWindOrder 
{
	WIND_CLOCKWISE,         // GL_CW       glFrontFace( GL_CW ); 
	WIND_COUNTER_CLOCKWISE, // GL_CCW      glFrontFace( GL_CCW ); 
	NUM_WIND_ORDER
};

enum eStencilCompare
{
	STC_COMPARE_NEVER,
	STC_COMPARE_LESS,
	STC_COMPARE_LEQUAL,
	STC_COMPARE_GREATER,
	STC_COMPARE_GEQUAL,
	STC_COMPARE_EQUAL,
	STC_COMPARE_NOTEQUAL,
	STC_COMPARE_ALWAYS,
	NUM_STC_COMPARE_MODE
};

enum eDepthCompare
{
	COMPARE_NEVER,       // GL_NEVER
	COMPARE_LESS,        // GL_LESS
	COMPARE_LEQUAL,      // GL_LEQUAL
	COMPARE_GREATER,     // GL_GREATER
	COMPARE_GEQUAL,      // GL_GEQUAL
	COMPARE_EQUAL,       // GL_EQUAL
	COMPARE_NOT_EQUAL,   // GL_NOTEQUAL
	COMPARE_ALWAYS,      // GL_ALWAYS
	NUM_COMPARE_MODE
};

enum eBlendOp
{
	COMPARE_ADD,
	NUM_BLEND_OP
};

enum eBlendFactor
{
	BLEND_ZERO,
	BLEND_ONE,
	BLEND_SRC_ALPHA,
	BLEND_ONE_MINUS_SRC_ALPHA,
	NUM_BLEND_FACTOR
};

enum eRenderQueue
{
	RENDER_QUEUE_OPAGUE,
	RENDER_QUEUE_ALPHA,
	NUM_OF_RENDER_QUEUE
};

struct sRenderState 
{
	// Raster State Control
	eCullMode  m_cullMode = CULLMODE_BACK;      // CULL_BACK
	eFillMode  m_fillMode = FILLMODE_SOLID;      // FILL_SOLID
	eWindOrder m_windOrder = WIND_COUNTER_CLOCKWISE;    // WIND_COUNTER_CLOCKWISE
	eDepthCompare m_depthCompare = COMPARE_LESS;		// COMPARE_LESS
	bool m_depthWrite = true;				  // true
	// stencil

	// Blend
	eBlendOp	 m_colorBlendOp = COMPARE_ADD;          // COMPARE_ADD
	eBlendFactor m_colorSrcFactor = BLEND_ONE;    // BLEND_ONE
	eBlendFactor m_colorDstFactor = BLEND_ZERO;    // BLEND_ZERO

	eBlendOp	 m_alphaBlendOp = COMPARE_ADD;          // COMPARE_ADD
	eBlendFactor m_alphaSrcFactor = BLEND_ONE;    // BLEND_ONE
	eBlendFactor m_alphaDstFactor = BLEND_ZERO;    // BLEND_ONE

	std::vector<eCullMode> m_cullModes;
	std::vector<eWindOrder> m_windOrders;
	std::vector<eDepthCompare> m_depthCompares;   
}; 

// This is the part of Shader.hpp
// Shader is just a combination of a program and render state
// NOTE: single pass shader
class Shader
{
public:
	// Bind shader program and render state from xml
	static Shader* AcquireResource(const char* fp);
	static Shader* MakeShader(const char* fp);

	// none of these call GL calls, just sets the internal state
	void SetProgram( ShaderProgram* program ) { m_program = program; }

	ShaderProgram*		GetShaderProgram() const { return m_program; }
	ShaderProgramInfo*	GetProgramInfo() const { return m_programInfo; }

	//void EnableBlending( eBlendOp op, eBlendFactor src, eBlendFactor dst ); 
	//void DisableBlending();

	//void DisableDepth() { SetDepth( COMPARE_ALWAYS, false ); }
	void SetDepth( eDepthCompare compare, bool write ); 
	void SetCull( eCullMode cull ); 
	void SetFill( eFillMode fill ); 
	void SetWind( eWindOrder wind ); 

	static eDepthCompare SelectCompare(std::string compareStr);
	static eCullMode SelectCull(std::string cullStr);
	static eWindOrder SelectOrder(std::string orderStr);

	Shader();
	~Shader();

public:
	ShaderProgramInfo*	m_programInfo = nullptr;

	uint		 m_layer;
	eRenderQueue m_queue;

	/////////////////////////////////////////////////
	ShaderProgram*		m_program = nullptr; 
	sRenderState		m_state;
}; 

// attempt on multi-pass shader

struct sShaderPass
{
	ShaderProgram*		m_program = nullptr;
	ShaderProgramInfo*  m_programInfo = nullptr;

	std::map<std::string, PropertyBlock*> m_blocks;
	std::map<int, Texture*>				  m_textures;

	template<typename T>
	void SetProperty(const char* name, T const &v);
	void SetProperty(const char* name, void const* data, size_t datasize);

	PropertyBlock* CreateOrGetBlock(sPropertyBlockInfo* info);
};

class ShaderChannel
{
public:
	static ShaderChannel* AcquireResource(const char* fp);
	static eDepthCompare SelectCompare(std::string compareStr);
	static eCullMode SelectCull(std::string cullStr);
	static eWindOrder SelectOrder(std::string orderStr);

	void FillPropertyBlocks();
	void FillTextures();

	template<typename T>
	void SetProperty(const char* name, T const &v);

public:
	const static int MAX_SHADER_PASS = 16;

	sShaderPass*  m_shaderPasses[MAX_SHADER_PASS];
	sRenderState  m_state;

	uint		 m_layer;	
	eRenderQueue m_queue;
};

template<typename T>
void sShaderPass::SetProperty(const char* name, T const &value)
{
	SetProperty(name, &value, sizeof(T));
}


template<typename T>
void ShaderChannel::SetProperty(const char* name, T const &v)
{
	for (int idx = 0; idx < MAX_SHADER_PASS; ++idx)
	{
		sShaderPass* pass = m_shaderPasses[idx];

		if (pass != nullptr)
		{
			pass->SetProperty(name, v);		
		}
	}
}