#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Submesh.hpp"
#include "Engine/Renderer/ShaderProperties.hpp"
#include "Engine/Renderer/Drawcall.hpp"
#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Vertex.hpp"
#include "Engine/Core/Light/Light.hpp"
#include "Engine/Core/Light/PointLight.hpp"
#include "Engine/Core/Light/DirectionalLight.hpp"
#include "Engine/Core/Light/SpotLight.hpp"
#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Math/MathUtils.hpp"

//#define STB_IMAGE_IMPLEMENTATION    

#include <string>
#include <vector>
#include <sstream>

Renderer* Renderer::m_rendererInstance = nullptr;

static const char* builtinDefaultSPVS = 
"// define the shader version (this is required)\n"
"#version 420 core\n"
"\n"
"// Attributes - input to this shader stage (constant as far as the code is concerned)\n"
"in vec3 POSITION;\n"
"\n"
"// Entry point - required.  What does this stage do?\n"
"void main( void )\n"
"{\n"
"	// for now, we're going to set the \n"
"	// clip position of this vertex to the passed \n"
"	// in position. \n"
"	// gl_Position is a system variable, or have special \n"
"	// meaning within the shader.\n"
"	gl_Position = vec4( POSITION, 1 ); \n"
"}\n";

static const char* builtinDefaultSPFS =
"#version 420 core\n"
"\n"
"// Outputs\n"
"out vec4 outColor;\n" 
"\n"
"// Entry Point\n"
"void main( void )\n"
"{\n"
"	outColor = vec4( 0, 1, 0, 1 ); \n"
"}\n";

static const char* builtinInvalidSPVS =
"// define the shader version (this is required)\n"
"#version 420 core\n"
"\n"
"// Attributes - input to this shader stage (constant as far as the code is concerned)\n"
"in vec3 POSITION;\n"
"\n"
"// Entry point - required.  What does this stage do?\n"
"void main( void )\n"
"{\n"
"	// for now, we're going to set the \n"
"	// clip position of this vertex to the passed "
"	// in position. \n"
"	// gl_Position is a system variable, or have special \n"
"	// meaning within the shader.\n"
"	gl_Position = vec4( POSITION, 1 ); \n"
"}\n";

static const char* builtinInvalidSPFS =
"#version 420 core\n"
"\n"
"// Outputs\n"
"out vec4 outColor; \n"
"\n"
"// Entry Point\n"
"void main( void )\n"
"{\n"
"	outColor = vec4( 1, 0, 1, 1 ); \n"
"}\n";


static GLenum ToGLCompare( eCompare compare ) 
{
	// Convert our engine to GL enum 
	switch (compare)
	{
	case COMPARE_NEVER:
		return GL_NEVER;
	case COMPARE_LESS:
		return GL_LESS;
	case COMPARE_LEQUAL:
		return GL_LEQUAL;
	case COMPARE_GREATER:
		return GL_GREATER;
	case COMPARE_GEQUAL:
		return GL_GEQUAL;
	case COMPARE_EQUAL:
		return GL_EQUAL;
	case COMPARE_NOT_EQUAL:
		return GL_NOTEQUAL;
	case COMPARE_ALWAYS:
		return GL_ALWAYS;
	default:
		return GL_NONE;
	}
}


GLenum ToGLBlendOp(eBlendOp op)
{
	switch (op)
	{
	case COMPARE_ADD:
		return GL_FUNC_ADD;
	default:
		return GL_NONE;
	}
}


GLenum ToGLBlendFactor(eBlendFactor factor)
{
	switch (factor)
	{
	case BLEND_ZERO:
		return GL_ZERO;
	case BLEND_ONE:
		return GL_ONE;
	case BLEND_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case BLEND_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	default:
		return GL_NONE;
	}
}


GLenum ToGLPrimitiveType(eDrawPrimitiveType type)
{
	switch (type)
	{
	case DRAW_POINT:
		return GL_POINTS;
	case DRAW_LINE:
		return GL_LINES;
	case DRAW_QUAD:
		return GL_QUADS;
	case DRAW_TRIANGLE:
		return GL_TRIANGLES;
	default:
		return GL_NONE;
	}
}


GLenum ToGLDataType( eRenderType type )
{
	switch (type)
	{
	case RT_FLOAT:
		return GL_FLOAT;
	case RT_UNSIGNED_BYTE:
		return GL_UNSIGNED_BYTE;
	default:
		return GL_NONE;
	}
}


GLenum ToGLFillMode(eFillMode mode)
{
	switch (mode)
	{
	case FILLMODE_SOLID:
		return GL_FILL;
	case FILLMODE_WIRE:
		return GL_LINE;
	default:
		return GL_NONE;
	}
}

GLenum ToGLWindOrder(eWindOrder order)
{
	switch (order)
	{
	case WIND_CLOCKWISE:
		return GL_CW;
	case WIND_COUNTER_CLOCKWISE:
		return GL_CCW;
	default:
		return GL_NONE;
	}
}

GLenum ToGLCullMode(eCullMode mode)
{
	switch (mode)
	{
	case CULLMODE_BACK:
		return GL_BACK;
	case CULLMODE_FRONT:
		return GL_FRONT;
	case CULLMODE_NONE:
		return GL_NONE;
	default:
		return GL_NONE;
	}
}


float ToDataSize(GLenum type)
{
	switch (type)
	{
	case GL_FLOAT:
		return 4.f;
	case GL_FLOAT_VEC4_ARB:
		return 16.f;
	case GL_FLOAT_VEC2_ARB:
		return 8.f;
	case GL_FLOAT_MAT4_ARB:
		return 64.f;
	default:
		return -1.f;
	}
}


void BindLayoutToProgram( GLuint programHandle, VertexLayout *layout ) 
{
	int attribCount = layout->GetAttributeCount(); 
	for (int attribIdx = 0; attribIdx < attribCount; ++attribIdx) {
		VertexAttribute* attrib = layout->GetAttribute(attribIdx); 

		GLint bind = glGetAttribLocation( programHandle, attrib->m_handle.c_str() );
		if (bind >= 0) {
			glEnableVertexAttribArray( bind ); 

			glVertexAttribPointer( bind, 
				attrib->m_elementCount, 
				ToGLDataType(attrib->m_type), 
				attrib->m_isNormalized, 
				layout->m_stride, 
				(GLvoid*)attrib->m_memberOffset ); 
		}
	}
	
	//GL_CHECK_ERROR();
}


Renderer::Renderer()
{
	m_immediateBuffer = new RenderBuffer();
	m_uboCamera = new UniformBuffer();
	m_uboLights = new UniformBuffer();
	m_uboObjects = new UniformBuffer();
	m_uboColors = new UniformBuffer();
	m_uboDebugMode = new UniformBuffer();
	m_uboFogInfo = new UniformBuffer();
	m_uboGameTime = new UniformBuffer();
	m_defaultCamera = new Camera();
	m_currentCamera = new Camera();
	m_effectCamera = nullptr;
	m_immediateMesh = nullptr;

	m_lightsData.ambience = Vector4(1.f, 1.f, 1.f, .5f);
	m_debugModeData.mode = Vector4(1.f, 1.f, 1.f, 0.f);

	m_fogData.fogColor = Vector4(1.f, 1.f, 1.f, 1.f);
	m_fogData.fogFarFactor = .8f;
	m_fogData.fogNearFactor = .2f;
	m_fogData.fogFarPlane = 60.f;
	m_fogData.fogNearPlane = 0.f;
}


Renderer* Renderer::GetInstance()
{
	if (!m_rendererInstance)
	{
		m_rendererInstance = new Renderer();
	}

	return m_rendererInstance;
}


void Renderer::DestroyInstance()
{
	delete m_rendererInstance;
	m_rendererInstance = nullptr;
}


Renderer::~Renderer()
{
	for(std::map<std::string, Texture*>::iterator itr = m_loadedTextures.begin(); itr != m_loadedTextures.end(); itr++)
	{
		delete (itr->second);
		itr->second = nullptr;
	}
	m_loadedTextures.clear();

	for(std::map<std::string, Sprite*>::iterator itr = m_loadedSprites.begin(); itr != m_loadedSprites.end(); itr++)
	{
		delete (itr->second);
		itr->second = nullptr;
	}
	m_loadedSprites.clear();

	for(std::map<std::string, BitmapFont*>::iterator itr = m_loadedFonts.begin(); itr != m_loadedFonts.end(); itr++)
	{
		delete (itr->second);
		itr->second = nullptr;
	}
	m_loadedFonts.clear();

	for(std::map<std::string, ShaderProgram*>::iterator itr = m_loadedShaderPrograms.begin(); itr != m_loadedShaderPrograms.end(); itr++)
	{
		delete (itr->second);
		itr->second = nullptr;
	}
	m_loadedShaderPrograms.clear();

	delete m_defaultShader;
	m_defaultShader = nullptr;
	//delete m_currentShader;
	m_currentShader = nullptr;

	delete m_immediateBuffer;
	m_immediateBuffer = nullptr;

	delete m_uboCamera;
	m_uboCamera = nullptr;

	delete m_uboLights;
	m_uboLights = nullptr;

	delete m_uboObjects;
	m_uboObjects = nullptr;

	delete m_uboColors;
	m_uboColors = nullptr;

	delete m_uboDebugMode;
	m_uboDebugMode = nullptr;

	delete m_uboGameTime;
	m_uboGameTime = nullptr;

	delete m_uboFogInfo;
	m_uboFogInfo = nullptr;

	delete m_defaultSampler;
	m_defaultSampler = nullptr;

	//delete m_pointSampler;
	m_pointSampler = nullptr;

	delete m_defaultCamera;
	m_defaultCamera = nullptr;

	//delete m_currentCamera;
	m_currentCamera = nullptr;

	delete m_defaultColorTarget;
	m_defaultColorTarget = nullptr;

	delete m_defaultDepthTarget;
	m_defaultDepthTarget = nullptr;

	delete m_effectCamera;
	m_effectCamera = nullptr;

	delete m_effectTarget;
	m_effectTarget = nullptr;

	delete m_effectScratch;
	m_effectScratch = nullptr;

	delete m_immediateMesh;
	m_immediateMesh = nullptr;

	delete m_assimpLoader;
	m_assimpLoader = nullptr;
}


void Renderer::BeginFrame()
{
	// Clear all screen (backbuffer) pixels to black
	ClearScreen(Rgba::BLACK);
}


void Renderer::EndFrame()
{
	HWND hWnd = GetActiveWindow();
	HDC hDC = GetDC( hWnd );

	CopyFrameBuffer( nullptr, &(m_currentCamera->GetFrameBuffer()) ); 
	SwapBuffers( hDC );

	GL_CHECK_ERROR();
}


void Renderer::PostStartup(float startupWidth, float startupHeight)
{
	// m_defaultVAO is a GLuint member variable
	glGenVertexArrays( 1, &m_defaultVAO ); 
	glBindVertexArray( m_defaultVAO );

	//CreateOrGetShader("include_test");			// convenient to test include here
	//CreateOrGetShaderChannel("shader_channel_test");	// to test multi-pass shader
	RegisterBuiltinShaders();

	m_defaultShader = CreateOrGetShader("default");
	m_currentShader = m_defaultShader;

	// prepare default sampler
	m_defaultSampler = new Sampler();

	// prepare point sampler
	m_pointSampler = new Sampler();

	// the default color and depth should match our output window
	// so get width/height however you need to.
	uint window_width = static_cast<uint>(startupWidth); 
	uint window_height = static_cast<uint>(startupHeight);

	// create our output textures
	m_defaultColorTarget = CreateRenderTarget( window_width, 
		window_height );
	m_defaultDepthTarget = CreateDepthStencilTarget( window_width, 
		window_height ); 

	// setup the initial camera
	m_defaultCamera->SetColorTarget( m_defaultColorTarget ); 
	m_defaultCamera->SetDepthStencilTarget( m_defaultDepthTarget ); 

	// set our default camera to be our current camera
	SetCamera(nullptr); 

	// initialize assimp
	m_assimpLoader = new AssimpLoader();
}


void Renderer::ClearScreen(const Rgba& clearColor)
{
	// Clear all screen (backbuffer) pixels to medium-blue
	ClearColor(clearColor);
	ClearDepth();
}


void Renderer::PushMatrix()
{
	UNIMPLEMENTED();
	//glPushMatrix();
}


void Renderer::PopMatrix()
{
	UNIMPLEMENTED();
	//glPopMatrix();
}


void Renderer::Translate(const Vector2&)
{
	UNIMPLEMENTED();
	//glTranslatef(translation.x, translation.y, 0.f);
}


void Renderer::Rotate(float)
{
	UNIMPLEMENTED();
	//glRotatef(degrees, 0.f, 0.f, 1.f);
}


void Renderer::ScaleUniform(float)
{
	UNIMPLEMENTED();
	//glScalef(uniformScale, uniformScale, 1.f);
}


void Renderer::ScaleX(float)
{
	UNIMPLEMENTED();
	//glScalef(xScale, 1.f, 1.f);
}


void Renderer::ScaleY(float)
{
	UNIMPLEMENTED();
	//glScalef(1.f, yScale, 1.f);
}


void Renderer::ScaleXY(float, float)
{
	UNIMPLEMENTED();
	//glScalef(xScale, yScale, 1.f);
}


Texture* Renderer::CreateOrGetTexture(std::string fp, bool mipmap)
{
	bool fileLoaded = ( m_loadedTextures.count(fp) > 0 );

	if (!fileLoaded)
	{
		Texture* texture = new Texture(fp, mipmap);
		Sampler* sampler = new Sampler(mipmap);
		texture->SetSampler(sampler);
		m_loadedTextures.emplace( fp, texture );

		return texture;
	}
	else 
	{
		// file already loaded, return the loaded texture
		return m_loadedTextures[fp];
	}
}


Mesh* Renderer::CreateOrGetMesh(std::string meshName)
{
	bool meshLoaded = ( m_loadedMeshes.count(meshName) > 0 );

	if (!meshLoaded)
	{
		Mesh* mesh = nullptr;

		if (meshName == "cube_pcu")
		{
			mesh = Mesh::CreateCube(VERT_PCU);
		}
		else if (meshName == "quad_pcu")
		{
			mesh = Mesh::CreateQuad(VERT_PCU);
		}
		else if (meshName == "sphere_pcu")
		{
			mesh = Mesh::CreateUVSphere(VERT_PCU, 18, 36);
		}
		else if (meshName == "ship_pcu")
		{
			std::string modelPath = "Data/Models/scifi_fighter_mk6.obj";
			mesh = Mesh::CreateModel(modelPath, VERT_PCU);
		}
		else if (meshName == "point_pcu")
		{
			mesh = Mesh::CreatePoint(VERT_PCU);
		}
		else if (meshName == "disc_pcu_2d")
		{
			mesh = Mesh::CreateDisc2D(VERT_PCU);
		}
		else if (meshName == "quad_pcu_2d")
		{
			mesh = Mesh::CreateQuad2D(VERT_PCU);
		}
		else if (meshName == "ship_lit")
		{
			std::string modelPath = "Data/Models/scifi_fighter_mk6.obj";
			mesh = Mesh::CreateModel(modelPath, VERT_LIT);
		}
		else if (meshName == "quad_lit")
		{
			mesh = Mesh::CreateQuad(VERT_LIT);
		}
		else if (meshName == "sphere_lit")
		{
			mesh = Mesh::CreateUVSphere(VERT_LIT, 18, 36);
		}
		else if (meshName == "cube_lit")
		{
			mesh = Mesh::CreateCube(VERT_LIT);
		}

		m_loadedMeshes.emplace(meshName, mesh);

		return mesh;
	}
	else 
	{
		return m_loadedMeshes[meshName];
	}
}


Sprite* Renderer::CreateOrGetSprite(std::string id)
{
	bool spriteLoaded = ( m_loadedSprites.count(id) > 0 );

	if (!spriteLoaded)
	{
		Sprite* spr = new Sprite(id);
		m_loadedSprites.emplace(id, spr);

		return spr;
	}
	else
	{
		return m_loadedSprites[id];
	}
}


BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontName)
{
	const std::string fontName(bitmapFontName);
	bool fontLoaded = ( m_loadedFonts.count(fontName) > 0 );

	if (!fontLoaded)
	{
		Texture* texture = CreateOrGetTexture(fontName);
		SpriteSheet* theSheet = new SpriteSheet(texture, IntVector2(16, 16));
		BitmapFont* font = new BitmapFont(fontName, *theSheet, 1.f);
		m_loadedFonts.emplace( fontName, font );

		return font;
	}
	else
	{
		return m_loadedFonts[fontName];
	}
}


ShaderProgram* Renderer::CreateOrGetShaderProgram(const char* fileName, const char* delimited)
{
	bool shaderProgramLoaded = ( m_loadedShaderPrograms.count(fileName) > 0 );
	const std::string fNStr(fileName);

	if (!shaderProgramLoaded)
	{
		ShaderProgram* shaderProgram = new ShaderProgram();
		std::string header = "Data/Shaders/";
		bool programLinked = shaderProgram->LoadFromFiles(header.append(fileName).c_str(), delimited);
		if (!programLinked)
		{
			return m_loadedShaderPrograms["invalid"];
		}
		m_loadedShaderPrograms.emplace( fileName, shaderProgram );

		return shaderProgram;
	}
	else 
	{
		// file already loaded, return the loaded shader program
		return m_loadedShaderPrograms[fileName];
	}
}


Shader* Renderer::CreateOrGetShader(std::string shaderName)
{
	bool shaderLoaded = (m_loadedShaders.count(shaderName) > 0);

	if (!shaderLoaded)
	{
		std::string header = "Data/Shaders/" + shaderName + ".xml";
		Shader* shader = Shader::AcquireResource(header.c_str());
		m_loadedShaders.emplace(shaderName, shader);

		return shader;
	}
	else
	{
		Shader* shader = m_loadedShaders[shaderName];
		return shader;
	}
}


Material* Renderer::CreateOrGetMaterial(std::string matName)
{
	if (matName == "")
	{
		Material* material = new Material();	// empty material not stored as resource
		return material;
	}

	bool matLoaded = (m_loadedMaterials.count(matName) > 0);

	if (!matLoaded)
	{
		std::string header = "Data/Materials/" + matName + ".mat";
		Material* mat = Material::AcquireShaderResource(header.c_str());

		// Do not let block and single prop coexist

		// Note: even when there are UBO in shader, need to consider if it
		// is reasonable properties to put in a material. But usually this is
		// considered when you use material to set property, aka. mat->SetProperty()...

		// set starting properties
		mat->FillPropertyBlocks();
		mat->FillTextures();
		//mat->FillSeparateProps();

		m_loadedMaterials.emplace(matName, mat);
		return mat;
	}
	else 
	{
		return m_loadedMaterials[matName];
	}
}


Material* Renderer::CreateOrGetStagedMaterial(std::string matName)
{
	if (matName == "")
	{
		Material* material = new Material();	// empty material not stored as resource
		return material;
	}

	bool matLoaded = (m_loadedMaterials.count(matName) > 0);

	if (!matLoaded)
	{
		std::string header = "Data/Materials/" + matName + ".mat";
		Material* mat = Material::AcquireShaderChannelResource(header.c_str());

		// fill property block and texture storage in shader pass
		mat->m_channel->FillPropertyBlocks();
		mat->m_channel->FillTextures();

		m_loadedMaterials.emplace(matName, mat);
		return mat;
	}
	else 
	{
		return m_loadedMaterials[matName];
	}
}


ShaderChannel* Renderer::CreateOrGetShaderChannel(std::string channelName)
{
	bool channelLoaded = (m_loadedChannels.count(channelName) > 0);

	if (!channelLoaded)
	{
		std::string header = "Data/Shaders/" + channelName + ".xml";
		ShaderChannel* channel = ShaderChannel::AcquireResource(header.c_str());
		m_loadedChannels.emplace(channelName, channel);

		return channel;
	}
	else
	{
		ShaderChannel* channel = m_loadedChannels[channelName];
		return channel;
	}
}


void Renderer::ReloadShaders()
{
	for(std::map<std::string, ShaderProgram*>::iterator itr = m_loadedShaderPrograms.begin(); itr != m_loadedShaderPrograms.end(); itr++)
	{
		ShaderProgram* theShader = itr->second;
		if (itr->first.find("builtin") != std::string::npos)
		{
			// reload builtin shaders
			if (itr->first == "builtinDefault")
			{
				theShader->LoadFromStrings(builtinDefaultSPVS, builtinDefaultSPFS);
			}
			if (itr->first == "builtinInvalid")
			{
				theShader->LoadFromStrings(builtinInvalidSPVS, builtinInvalidSPFS);
			}

			std::string printMsg = (itr->first) + " reloaded\n";
			DebuggerPrintf(printMsg.c_str());
		}
		else
		{
			theShader->Reload((itr->first).c_str(), "");
		}
	}
}


void Renderer::BlendOne()
{
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
}


void Renderer::BlendAlpha()
{
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );	
}


void Renderer::Bind(const Texture& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture.m_textureID);
	//GL_CHECK_ERROR();
}


void Renderer::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_CHECK_ERROR();
}


void Renderer::DrawAABB2D(const AABB2& bounds, const Rgba& color, eVertexType type)
{
	//Vector2 uv = Vector2(0.f, 0.f);

	Vector2 boundBL = bounds.mins;
	Vector2 boundBR = Vector2(bounds.maxs.x, bounds.mins.y);
	Vector2 boundTR = bounds.maxs;
	Vector2 boundTL = Vector2(bounds.mins.x, bounds.maxs.y);

	Vector3 blVec3 = boundBL.ToVector3(0.f);
	Vector3 brVec3 = boundBR.ToVector3(0.f);
	Vector3 trVec3 = boundTR.ToVector3(0.f);
	Vector3 tlVec3 = boundTL.ToVector3(0.f);

	if (m_immediateMesh != nullptr)
	{
		DetachMeshImmediate();
	}
	m_immediateMesh = Mesh::CreateQuadImmediate(type, blVec3, brVec3, tlVec3, trVec3, color);
	UseShader(CreateOrGetShader("2d_direct_alpha"));

	DrawMeshImmediate();
}


void Renderer::DrawText2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight, 
	const Rgba& tint /* = Rgba::WHITE */, float aspectScale /* = 1.f */, const BitmapFont* font /* = nullptr */ )
{
	SetTexture2D(0, font->GetSpriteSheet().GetTexture());
	SetSampler2D(0, font->GetSpriteSheet().GetTexture()->GetSampler());

	if (m_immediateMesh != nullptr)
	{
		DetachMeshImmediate();
	}
	m_immediateMesh = Mesh::CreateTextImmediate(tint,
		drawMins, font, cellHeight, aspectScale, asciiText, VERT_PCU);
	UseShader(CreateOrGetShader("cutout_nonmodel"));

	DrawMeshImmediate();
}


void Renderer::DrawPoint2D(const Vector2& point, const Rgba& color, eVertexType type, float size)
{
	Texture* texture = CreateOrGetTexture("Data/Images/white.png");
	SetTexture2D(0, texture);
	SetSampler2D(0, texture->GetSampler());

	DetachMeshImmediate();

	m_immediateMesh = Mesh::CreatePointImmediate(type, point, color);
	UseShader(CreateOrGetShader("2d_direct_opague"));

	glPointSize(size);
	DrawMeshImmediate();
}

void Renderer::DrawPolygon2D(const Vector2& bl, const Vector2& br, const Vector2& tl, const Vector2& tr, const Rgba& color, eVertexType type)
{
	Texture* texture = CreateOrGetTexture("Data/Images/white.png");
	SetTexture2D(0, texture);
	SetSampler2D(0, texture->GetSampler());

	DetachMeshImmediate();

	m_immediateMesh = Mesh::CreatePolygonImmedidate(type, bl, br, tl, tr, color);
	UseShader(CreateOrGetShader("2d_direct_opague"));

	DrawMeshImmediate();
}

void Renderer::DrawLine3D( const Vector3& start, const Vector3& end, const Rgba& color, float lineThickness )
{
	Texture* texture = CreateOrGetTexture("Data/Images/white.png");
	SetTexture2D(0, texture);
	SetSampler2D(0, texture->GetSampler());

	if (m_immediateMesh != nullptr)
	{
		DetachMeshImmediate();
	}

	m_immediateMesh = Mesh::CreateLineImmediate(VERT_PCU, start, end, color);
	UseShader(CreateOrGetShader("direct"));

	glLineWidth(lineThickness);
	DrawMeshImmediate();
}


//void Renderer::DrawLine2D( const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, float lineThickness)
//{
//	Vertex_3DPCU v1 = Vertex_3DPCU(Vector3(start.x, start.y, 0.f), startColor, Vector2(0.f, 0.f));
//	Vertex_3DPCU v2 = Vertex_3DPCU(Vector3(end.x, end.y, 0.f), endColor, Vector2(0.f, 0.f));
//	const Vertex_3DPCU vertices[2] = {v1, v2};
//
//	glLineWidth(lineThickness);
//	DrawMeshImmediate(vertices, 2, DRAW_LINE);
//}
//
//
//void Renderer::DrawAABB3D( const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, const Vector3& vert4, const Rgba& color)
//{
//	Vector2 uv = Vector2(0.f, 0.f);
//	Vertex_3DPCU v1 = Vertex_3DPCU(vert1, color, uv);
//	Vertex_3DPCU v2 = Vertex_3DPCU(vert2, color, uv);
//	Vertex_3DPCU v3 = Vertex_3DPCU(vert3, color, uv);
//	Vertex_3DPCU v4 = Vertex_3DPCU(vert1, color, uv);
//	Vertex_3DPCU v5 = Vertex_3DPCU(vert3, color, uv);
//	Vertex_3DPCU v6 = Vertex_3DPCU(vert4, color, uv);
//	const Vertex_3DPCU vertices[6] = { v1, v2, v3, v4, v5, v6 };
//	DrawMeshImmediate(vertices, 6, DRAW_TRIANGLE);
//}
//
//
//void Renderer::DrawCube3D(const Vector3& center, const Vector3& dimensions, const Rgba& color /* = Rgba::WHITE */, 
//	AABB2 uv_top /* = AABB2::ZERO_TO_ONE */, AABB2 uv_side /* = AABB2::ZERO_TO_ONE */, AABB2 uv_bottom /* = AABB2::ZERO_TO_ONE */)
//{
//	Vector3 vert1 = center + Vector3(-dimensions.x / 2.f, dimensions.y / 2.f, -dimensions.z / 2.f);
//	Vector3 vert2 = center + Vector3( dimensions.x / 2.f, dimensions.y / 2.f, -dimensions.z / 2.f );
//	Vector3 vert3 = center + Vector3( -dimensions.x / 2.f, -dimensions.y / 2.f, -dimensions.z / 2.f );
//	Vector3 vert4 = center + Vector3( dimensions.x / 2.f, -dimensions.y / 2.f, -dimensions.z / 2.f );
//	Vector3 vert5 = center + Vector3( -dimensions.x / 2.f, dimensions.y / 2.f, dimensions.z / 2.f );
//	Vector3 vert6 = center + Vector3( dimensions.x / 2.f, dimensions.y / 2.f, dimensions.z / 2.f );
//	Vector3 vert7 = center + Vector3(-dimensions.x / 2.f, -dimensions.y / 2.f, dimensions.z / 2.f);
//	Vector3 vert8 = center + Vector3( dimensions.x / 2.f, -dimensions.y / 2.f, dimensions.z / 2.f );
//
//	// front
//	Vertex_3DPCU v1 = Vertex_3DPCU(vert3, color, uv_side.mins);
//	Vertex_3DPCU v2 = Vertex_3DPCU(vert4, color, Vector2(uv_side.maxs.x, uv_side.mins.y));
//	Vertex_3DPCU v3 = Vertex_3DPCU(vert2, color, uv_side.maxs);
//	Vertex_3DPCU v4 = Vertex_3DPCU(vert3, color, uv_side.mins);
//	Vertex_3DPCU v5 = Vertex_3DPCU(vert2, color, uv_side.maxs);
//	Vertex_3DPCU v6 = Vertex_3DPCU(vert1, color, Vector2(uv_side.mins.x, uv_side.maxs.y));
//
//	// top
//	Vertex_3DPCU v7 = Vertex_3DPCU(vert1, color,  Vector2(uv_top.mins.x, uv_top.maxs.y));
//	Vertex_3DPCU v8 = Vertex_3DPCU(vert2, color,  uv_top.maxs);
//	Vertex_3DPCU v9 = Vertex_3DPCU(vert6, color,  Vector2(uv_top.maxs.x, uv_top.mins.y));
//	Vertex_3DPCU v10 = Vertex_3DPCU(vert1, color, Vector2(uv_top.mins.x, uv_top.maxs.y));
//	Vertex_3DPCU v11 = Vertex_3DPCU(vert6, color, Vector2(uv_top.maxs.x, uv_top.mins.y));
//	Vertex_3DPCU v12 = Vertex_3DPCU(vert5, color, uv_top.mins);
//
//	// left
//	Vertex_3DPCU v13 = Vertex_3DPCU(vert7, color, uv_side.mins);
//	Vertex_3DPCU v14 = Vertex_3DPCU(vert3, color, Vector2(uv_side.maxs.x, uv_side.mins.y));
//	Vertex_3DPCU v15 = Vertex_3DPCU(vert1, color, uv_side.maxs);
//	Vertex_3DPCU v16 = Vertex_3DPCU(vert7, color, uv_side.mins);
//	Vertex_3DPCU v17 = Vertex_3DPCU(vert1, color, uv_side.maxs);
//	Vertex_3DPCU v18 = Vertex_3DPCU(vert5, color, Vector2(uv_side.mins.x, uv_side.maxs.y));
//
//	// back
//	Vertex_3DPCU v19 = Vertex_3DPCU(vert8, color, uv_side.mins);
//	Vertex_3DPCU v20 = Vertex_3DPCU(vert7, color, Vector2(uv_side.maxs.x, uv_side.mins.y));
//	Vertex_3DPCU v21 = Vertex_3DPCU(vert5, color, uv_side.maxs);
//	Vertex_3DPCU v22 = Vertex_3DPCU(vert8, color, uv_side.mins);
//	Vertex_3DPCU v23 = Vertex_3DPCU(vert5, color, uv_side.maxs);
//	Vertex_3DPCU v24 = Vertex_3DPCU(vert6, color, Vector2(uv_side.mins.x, uv_side.maxs.y));
//
//	// bottom
//	Vertex_3DPCU v25 = Vertex_3DPCU(vert3, color, Vector2(uv_bottom.mins.x, uv_bottom.maxs.y));
//	Vertex_3DPCU v26 = Vertex_3DPCU(vert4, color, uv_bottom.maxs);
//	Vertex_3DPCU v27 = Vertex_3DPCU(vert8, color, Vector2(uv_bottom.maxs.x, uv_bottom.mins.y));
//	Vertex_3DPCU v28 = Vertex_3DPCU(vert3, color, Vector2(uv_bottom.mins.x, uv_bottom.maxs.y));
//	Vertex_3DPCU v29 = Vertex_3DPCU(vert8, color, Vector2(uv_bottom.maxs.x, uv_bottom.mins.y));
//	Vertex_3DPCU v30 = Vertex_3DPCU(vert7, color, uv_bottom.mins);
//
//	// right
//	Vertex_3DPCU v31 = Vertex_3DPCU(vert4, color, uv_side.mins);
//	Vertex_3DPCU v32 = Vertex_3DPCU(vert8, color, Vector2(uv_side.maxs.x, uv_side.mins.y));
//	Vertex_3DPCU v33 = Vertex_3DPCU(vert6, color, uv_side.maxs);
//	Vertex_3DPCU v34 = Vertex_3DPCU(vert4, color, uv_side.mins);
//	Vertex_3DPCU v35 = Vertex_3DPCU(vert6, color, uv_side.maxs);
//	Vertex_3DPCU v36 = Vertex_3DPCU(vert2, color, Vector2(uv_side.mins.x, uv_side.maxs.y));
//
//	const Vertex_3DPCU vertices[36] = { v1, v2, v3, v4, v5, v6, 
//		v7, v8, v9, v10, v11, v12, 
//		v13, v14, v15, v16, v17, v18, 
//		v19, v20, v21, v22, v23, v24,
//		v25, v26, v27, v28, v29, v30, 
//		v31, v32, v33, v34, v35, v36 
//	};
//
//	DrawMeshImmediate(vertices, 36, DRAW_TRIANGLE);
//}
//
//
//void Renderer::DrawTriangle( const Vector2& vert1, const Vector2& vert2, const Vector2& vert3, const Rgba& color, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3 )
//{
//	Vertex_3DPCU v1 = Vertex_3DPCU(Vector3(vert1.x, vert1.y, 0.f), color, uv1);
//	Vertex_3DPCU v2 = Vertex_3DPCU(Vector3(vert2.x, vert2.y, 0.f), color, uv2);
//	Vertex_3DPCU v3 = Vertex_3DPCU(Vector3(vert3.x, vert3.y, 0.f), color, uv3);
//	const Vertex_3DPCU vertices[3] = { v1, v2, v3 };
//
//	DrawMeshImmediate(vertices, 3, DRAW_TRIANGLE);
//}


//void Renderer::DrawTexturedAABB(const AABB2& bounds, const Texture& texture, const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint)
//{
//	Bind(texture);
//
//	Vector2 bl = texCoordsAtMins;
//	Vector2 br = Vector2(texCoordsAtMaxs.x, texCoordsAtMins.y);
//	Vector2 tr = texCoordsAtMaxs;
//	Vector2 tl = Vector2(texCoordsAtMins.x, texCoordsAtMaxs.y);
//
//	Vector2 boundBL = bounds.mins;
//	Vector2 boundBR = Vector2(bounds.maxs.x, bounds.mins.y);
//	Vector2 boundTR = bounds.maxs;
//	Vector2 boundTL = Vector2(bounds.mins.x, bounds.maxs.y);
//
//	Vertex_3DPCU v1 = Vertex_3DPCU(boundBL.ToVector3(0.f), tint, bl);
//	Vertex_3DPCU v2 = Vertex_3DPCU(boundBR.ToVector3(0.f), tint, br);
//	Vertex_3DPCU v3 = Vertex_3DPCU(boundTR.ToVector3(0.f), tint, tr);
//	Vertex_3DPCU v4 = Vertex_3DPCU(boundBL.ToVector3(0.f), tint, bl);
//	Vertex_3DPCU v5 = Vertex_3DPCU(boundTR.ToVector3(0.f), tint, tr);
//	Vertex_3DPCU v6 = Vertex_3DPCU(boundTL.ToVector3(0.f), tint, tl);
//	const Vertex_3DPCU vertices[6] = { v1, v2, v3, v4, v5, v6 };
//
//	DrawMeshImmediate(vertices, 6, DRAW_TRIANGLE);
//
//	Unbind();
//}


//void Renderer::DrawOrientedQuad(const Vector3& center, const Vector3& orientation, const Texture& tex, float width, float height,
//	const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint, float scaleX, float scaleY)
//{
//	Bind(tex);
//
//	Vector2 bl;
//	Vector2 br;
//	Vector2 tr;
//	Vector2 tl;
//
//	if (scaleX == 1.f && scaleY == 1.f)
//	{
//		bl = texCoordsAtMins;
//		br = Vector2(texCoordsAtMaxs.x, texCoordsAtMins.y);
//		tr = texCoordsAtMaxs;
//		tl = Vector2(texCoordsAtMins.x, texCoordsAtMaxs.y);
//	}
//	else if (scaleX == -1.f && scaleY == 1.f)
//	{
//		bl = Vector2(texCoordsAtMaxs.x, texCoordsAtMins.y);
//		br = texCoordsAtMins;
//		tr = Vector2(texCoordsAtMins.x, texCoordsAtMaxs.y);
//		tl = texCoordsAtMaxs;
//	}
//
//	Vector3 globalUp = Vector3(0.f, 1.f, 0.f);
//	Vector3 normalizedOrientation = orientation.GetNormalized();
//	Vector3 hori = normalizedOrientation.Cross(globalUp);
//	hori.NormalizeAndGetLength();
//
//	Vector3 boundBL = center + hori * (-width / 2.f) + globalUp * (-height / 2.f);
//	Vector3 boundBR = center + hori * (width / 2.f) + globalUp * (-height / 2.f);
//	Vector3 boundTR = center + hori * (width / 2.f) + globalUp * (height / 2.f);
//	Vector3 boundTL = center + hori * (-width / 2.f) + globalUp * (height / 2.f);
//
//	Vertex_3DPCU v1 = Vertex_3DPCU(boundBL, tint, bl);
//	Vertex_3DPCU v2 = Vertex_3DPCU(boundBR, tint, br);
//	Vertex_3DPCU v3 = Vertex_3DPCU(boundTR, tint, tr);
//	Vertex_3DPCU v4 = Vertex_3DPCU(boundBL, tint, bl);
//	Vertex_3DPCU v5 = Vertex_3DPCU(boundTR, tint, tr);
//	Vertex_3DPCU v6 = Vertex_3DPCU(boundTL, tint, tl);
//	const Vertex_3DPCU vertices[6] = { v1, v2, v3, v4, v5, v6 };
//
//	DrawMeshImmediate(vertices, 6, DRAW_TRIANGLE);
//
//	Unbind();
//}


//void Renderer::DrawOrientedQuadLocal(const Vector3& center, const Vector3& orientation, const Texture& tex, float width, float height,
//	const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint, float scaleX, float scaleY)
//{
//	Bind(tex);
//
//	Vector2 bl;
//	Vector2 br;
//	Vector2 tr;
//	Vector2 tl;
//
//	if (scaleX == 1.f && scaleY == 1.f)
//	{
//		bl = texCoordsAtMins;
//		br = Vector2(texCoordsAtMaxs.x, texCoordsAtMins.y);
//		tr = texCoordsAtMaxs;
//		tl = Vector2(texCoordsAtMins.x, texCoordsAtMaxs.y);
//	}
//	else if (scaleX == -1.f && scaleY == 1.f)
//	{
//		bl = Vector2(texCoordsAtMaxs.x, texCoordsAtMins.y);
//		br = texCoordsAtMins;
//		tr = Vector2(texCoordsAtMins.x, texCoordsAtMaxs.y);
//		tl = texCoordsAtMaxs;
//	}
//
//	Vector3 globalUp = Vector3(0.f, 1.f, 0.f);
//	Vector3 normalizedOrientation = orientation.GetNormalized();
//	Vector3 hori = normalizedOrientation.Cross(globalUp);
//	hori.NormalizeAndGetLength();
//	Vector3 localUp = normalizedOrientation.Cross(-hori);
//
//	Vector3 boundBL = center + hori * (-width / 2.f) + localUp * (-height / 2.f);
//	Vector3 boundBR = center + hori * (width / 2.f) + localUp * (-height / 2.f);
//	Vector3 boundTR = center + hori * (width / 2.f) + localUp * (height / 2.f);
//	Vector3 boundTL = center + hori * (-width / 2.f) + localUp * (height / 2.f);
//
//	Vertex_3DPCU v1 = Vertex_3DPCU(boundBL, tint, bl);
//	Vertex_3DPCU v2 = Vertex_3DPCU(boundBR, tint, br);
//	Vertex_3DPCU v3 = Vertex_3DPCU(boundTR, tint, tr);
//	Vertex_3DPCU v4 = Vertex_3DPCU(boundBL, tint, bl);
//	Vertex_3DPCU v5 = Vertex_3DPCU(boundTR, tint, tr);
//	Vertex_3DPCU v6 = Vertex_3DPCU(boundTL, tint, tl);
//	const Vertex_3DPCU vertices[6] = { v1, v2, v3, v4, v5, v6 };
//
//	DrawMeshImmediate(vertices, 6, DRAW_TRIANGLE);
//
//	Unbind();
//}
//
//
//void Renderer::DrawText3D(const Vector3& up, const Vector3& right, const Vector3& drawMinsWorldPos, const std::string text, float cellHeight, 
//	const Rgba& tint /* = Rgba::WHITE */, float aspectScale /* = 1.f */, const BitmapFont* font /* = nullptr */)
//{
//	float cellWidth = cellHeight * (font->GetGlyphAspect() * aspectScale);
//	Vector3 cellHorizontalLocal = right * cellWidth;
//	Vector3 cellVertLocal = up * cellHeight;
//
//	for (size_t charIndex = 0; charIndex < text.length(); ++charIndex)
//	{
//		const char& character = text.at(charIndex);
//		Vector3 boundMin = drawMinsWorldPos + cellHorizontalLocal * (float)charIndex;
//		Vector3 boundMax = boundMin + cellHorizontalLocal * 1.f + cellVertLocal * 1.f;
//		Vector3 center = (boundMin + boundMax) / 2.f;
//		Vector3 orientation = right.Cross(-up);
//		Texture* tex = font->GetSpriteSheet().GetTexture();
//		Vector2 uvMin = font->GetUVsForGlyph(character).mins;
//		Vector2 uvMax = font->GetUVsForGlyph(character).maxs;
//
//		DrawOrientedQuadLocal(center, orientation, *tex, cellWidth, cellHeight,
//			uvMin, uvMax, tint, 1.f, 1.f);
//	}
//}


void Renderer::Draw(const Drawcall& dc)
{
	//EnableLights(dc);

	ShaderChannel* channel = dc.m_material->m_channel;
	Shader* shader = dc.m_material->m_shader;

	if (channel != nullptr)
	{
		for each (sShaderPass* pass in channel->m_shaderPasses)
		{
			if (pass != nullptr)
			{
				GLuint programHandle = pass->m_program->GetHandle();
				glUseProgram(programHandle);

				// texture
				for (std::map<int, Texture*>::iterator it = pass->m_textures.begin(); 
					it != pass->m_textures.end(); ++it)
				{
					int idx = it->first;
					Texture* texture = it->second;

					SetSampler2D(idx, texture->m_sampler);
					SetTexture2D(idx, texture);
				}

				pass->SetProperty("TINT", dc.m_tint);

				for (std::map<std::string, PropertyBlock*>::iterator it = pass->m_blocks.begin();
					it != pass->m_blocks.end(); ++it)
				{
					PropertyBlock* block = it->second;
					glBindBufferBase(GL_UNIFORM_BUFFER, block->m_blockInfo->blockIdx, block->GetHandle());

					size_t size = block->m_blockInfo->blockSize;
					block->CopyToGPU(size, block->m_dataBlock);
				}

				// set uniforms/UBO that are NOT properteis of material
				m_objectData.model = dc.m_model;
				SetObjectUBO(programHandle);
				SetCameraUBO(programHandle);		
				SetLightsUBO(programHandle);	
				SetFogInfoUBO(programHandle);
				SetDebugModeUBO(programHandle);
				SetGameTimeUBO(programHandle);

				BindRenderState(channel->m_state);

				Mesh* mesh = dc.m_mesh;
				glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vbo.GetHandle());
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ibo.GetHandle());
				BindLayoutToProgram(programHandle, mesh->GetLayout());

				// Now that it is described and bound, draw using our program
				if ( mesh->GetDrawInstruction().using_indices )
				{
					glDrawElements( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type), 
						mesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
				}
				else
				{
					glDrawArrays( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type),
						0, mesh->GetVertexCount() );
				}	
			}
		}
	}
	else if ( shader != nullptr )
	{
		// shader
		UseShader(shader);
		GLuint programHandle = m_currentShader->GetShaderProgram()->GetHandle();
		glUseProgram(programHandle);

		// texture
		for (std::map<int, Texture*>::iterator it = dc.m_material->m_textures.begin();
			it != dc.m_material->m_textures.end(); ++it)
		{
			int idx = it->first;
			Texture* tex = it->second;

			SetSampler2D(idx, tex->m_sampler);
			SetTexture2D(idx, tex);
		}

		// set color/tint
		dc.m_material->SetProperty("TINT", dc.m_tint);

		// light const should be set somewhere if wanted before material property block is passed to GPU

		// bind property blocks
		for (std::map<std::string, PropertyBlock*>::iterator it = dc.m_material->m_blocks.begin();
			it != dc.m_material->m_blocks.end(); ++it)
		{
			PropertyBlock* block = it->second;
			glBindBufferBase(GL_UNIFORM_BUFFER, block->m_blockInfo->blockIdx, block->GetHandle());

			size_t size = block->m_blockInfo->blockSize;
			block->CopyToGPU(size, block->m_dataBlock);
		}

		// set uniforms/UBO that are NOT properteis of material
		m_objectData.model = dc.m_model;
		SetObjectUBO(programHandle);
		SetCameraUBO(programHandle);		
		SetLightsUBO(programHandle);		
		SetFogInfoUBO(programHandle);
		SetDebugModeUBO(programHandle);
		SetGameTimeUBO(programHandle);

		BindRenderState(m_currentShader->m_state);

		Mesh* mesh = dc.m_mesh;
		glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vbo.GetHandle());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ibo.GetHandle());
		BindLayoutToProgram(programHandle, mesh->GetLayout());

		// Now that it is described and bound, draw using our program
		if ( mesh->GetDrawInstruction().using_indices )
		{
			glDrawElements( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type), 
				mesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
		}
		else
		{
			glDrawArrays( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type),
				0, mesh->GetVertexCount() );
		}	
	}
	else
	{
		ASSERT_OR_DIE(((shader != nullptr) || (channel != nullptr)), "both shader and channel are null pointers!");
	}

	GL_CHECK_ERROR();
}

// Not using forward path or material - for example, see the use of SetObjectColorUBO
void Renderer::DrawMesh(Mesh* mesh)
{
	//PROFILE_LOG_SCOPED_FUNCTION();
	GLuint programHandle = m_currentShader->GetShaderProgram()->GetHandle();
	glUseProgram(programHandle);

	SetObjectUBO(programHandle);
	SetCameraUBO(programHandle);
	SetLightsUBO(programHandle);
	SetDebugModeUBO(programHandle);
	//SetGameTimeUBO(programHandle);

	BindRenderState(m_currentShader->m_state);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vbo.GetHandle());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ibo.GetHandle());
	BindLayoutToProgram( programHandle, mesh->GetLayout() ); 

	// Now that it is described and bound, draw using our program
	if ( mesh->GetDrawInstruction().using_indices )
	{
		glDrawElements( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type), 
			mesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type),
			0, mesh->GetVertexCount() );
	}

	GL_CHECK_ERROR();
}


void Renderer::DrawMeshImmediate()
{
	GLuint programHandle = m_currentShader->GetShaderProgram()->GetHandle();
	glUseProgram(programHandle);

	SetObjectUBO(programHandle);
	SetCameraUBO(programHandle);
	SetLightsUBO(programHandle);
	SetColorUBO(programHandle);
	SetDebugModeUBO(programHandle);

	BindRenderState(m_currentShader->m_state);
	glBindBuffer(GL_ARRAY_BUFFER, m_immediateMesh->m_vbo.GetHandle());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_immediateMesh->m_ibo.GetHandle());
	BindLayoutToProgram( programHandle, m_immediateMesh->GetLayout() ); 

	// Now that it is described and bound, draw using our program
	if ( m_immediateMesh->GetDrawInstruction().using_indices )
	{
		glDrawElements( ToGLPrimitiveType(m_immediateMesh->GetDrawInstruction().primitive_type), 
			m_immediateMesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( ToGLPrimitiveType(m_immediateMesh->GetDrawInstruction().primitive_type),
			0, m_immediateMesh->GetVertexCount() );
	}

	GL_CHECK_ERROR();
}


void Renderer::BindRenderState(const sRenderState& state)
{
	// blend mode
	glEnable( GL_BLEND ); 
	//glBlendEquationSeparate(ToGLBlendOp(state.m_colorBlendOp), ToGLBlendOp(state.m_alphaBlendOp));
	//glBlendFuncSeparate(ToGLBlendFactor(state.m_colorSrcFactor), ToGLBlendFactor(state.m_colorDstFactor),
	//	ToGLBlendFactor(state.m_alphaSrcFactor), ToGLBlendFactor(state.m_alphaDstFactor));
	glBlendEquation(ToGLBlendOp(state.m_colorBlendOp));
	glBlendFunc( ToGLBlendFactor(state.m_colorSrcFactor),
		ToGLBlendFactor(state.m_colorDstFactor) ); 
	glBlendEquation(ToGLBlendOp(state.m_alphaBlendOp));
	glBlendFunc( ToGLBlendFactor(state.m_alphaSrcFactor),
		ToGLBlendFactor(state.m_alphaDstFactor) );

	// Depth mode ones
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(ToGLCompare(state.m_depthCompare));
	glDepthMask(state.m_depthWrite);

	// Fill mode
	glPolygonMode(GL_FRONT_AND_BACK,
		ToGLFillMode(state.m_fillMode));
	GL_CHECK_ERROR();

	// Cull mode
	glEnable(GL_CULL_FACE);
	GL_CHECK_ERROR();
	glCullFace(ToGLCullMode(state.m_cullMode));
	GL_CHECK_ERROR();

	// Winding Order
	glFrontFace(ToGLWindOrder(state.m_windOrder));
	GL_CHECK_ERROR();
}


void Renderer::SetUniform(const char* name, float f)
{
	int bindIdx = glGetUniformLocation( 
		m_currentShader->GetShaderProgram()->GetHandle(), 
		name ); 
	if (bindIdx >= 0)
	{
		glUniform1fv( bindIdx, 1, &f );
	}
}


void Renderer::SetUniform(const char* name, Vector3 v3)
{
	int bindIdx = glGetUniformLocation( 
		m_currentShader->GetShaderProgram()->GetHandle(), 
		name ); 
	if (bindIdx >= 0)
	{
		glUniform3fv( bindIdx, 1, (GLfloat*)&v3 );
	}
}


void Renderer::SetUniform(const char* name, Vector4 v4)
{
	int bindIdx = glGetUniformLocation( 
		m_currentShader->GetShaderProgram()->GetHandle(), 
		name ); 
	if (bindIdx >= 0)
	{
		glUniform4fv( bindIdx, 1, (GLfloat*)&v4 );
	}
}


void Renderer::SetUniform(const char* name, Matrix44 m44)
{
	int bindIdx = glGetUniformLocation( 
		m_currentShader->GetShaderProgram()->GetHandle(), 
		name ); 
	if (bindIdx >= 0)
	{
		glUniformMatrix4fv( bindIdx, 1, GL_FALSE, ( GLfloat* )&m44 );
	}
}


void Renderer::SetCameraUBO(GLuint handle)
{
	SetCameraUniforms();
	size_t size = sizeof(sCameraData);
	m_uboCamera->CopyToGPU(size, &m_cameraData);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_BUFFER_BIND_IDX, m_uboCamera->GetHandle());

	GLint uniformBlockIndex = glGetUniformBlockIndex(handle, "uboCamera");
	if (uniformBlockIndex >= 0)
	{
		glUniformBlockBinding(handle, uniformBlockIndex, CAMERA_BUFFER_BIND_IDX);
	}
}


void Renderer::SetLightsUBO(GLuint handle)
{
	m_uboLights->CopyToGPU(sizeof(sAllLightsData), &m_lightsData);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTS_BUFFER_BIND_IDX, m_uboLights->GetHandle());

	GLint uniformBlockIndex = glGetUniformBlockIndex(handle, "uboLights");
	if (uniformBlockIndex >= 0)
	{
		glUniformBlockBinding(handle, uniformBlockIndex, LIGHTS_BUFFER_BIND_IDX);
	}
}

void Renderer::SetObjectUBO(GLuint)
{
	m_uboObjects->CopyToGPU(sizeof(sOjbectData), &m_objectData);
	glBindBufferBase(GL_UNIFORM_BUFFER, OBJECT_BUFFER_BIND_IDX, m_uboObjects->GetHandle());
}


void Renderer::SetColorUBO( GLuint )
{
	m_uboColors->CopyToGPU(sizeof(sColorData), &m_colorData);
	glBindBufferBase(GL_UNIFORM_BUFFER, COLOR_BIND_IDX, m_uboColors->GetHandle());
}


void Renderer::SetDebugModeUBO(GLuint)
{
	m_uboDebugMode->CopyToGPU(sizeof(sDebugMode), &m_debugModeData);
	glBindBufferBase(GL_UNIFORM_BUFFER, DEBUG_MODE_BUFFER_BIND_IDX, m_uboDebugMode->GetHandle());
}


void Renderer::SetGameTimeUBO(GLuint)
{
	m_uboGameTime->CopyToGPU(sizeof(sGameTime), &m_timeData);
	glBindBufferBase(GL_UNIFORM_BUFFER, GAME_FRAME_BIND_IDX, m_uboGameTime->GetHandle());
}

void Renderer::SetFogInfoUBO(GLuint)
{
	m_uboFogInfo->CopyToGPU(sizeof(sFogInfo), &m_fogData);
	glBindBufferBase(GL_UNIFORM_BUFFER, FOG_BIND_IDX, m_uboFogInfo->GetHandle());
}

void Renderer::SetCameraUniforms()
{
	m_cameraData.eyePosition	=	m_currentCamera->GetTransform().GetWorldPosition();
	//m_cameraData.eyePosition	=	m_currentCamera->GetTransform().GetLocalPosition();
	m_cameraData.view			=	m_currentCamera->GetView();
	m_cameraData.proj			=	m_currentCamera->GetProjection();
}


void Renderer::ConfigureLightData(
	Vector3 lightPosition, Vector4 lightColor,
	Vector3 attenuation, float innerAngle, 
	Vector3 specAttenuation, float outerAngle, 
	Vector3 dir, float dirFactor,
	int lightidx,
	bool useShadow, Matrix44& shadowVP)
{
	m_lightsData.lights[lightidx].lightPosition = lightPosition;
	m_lightsData.lights[lightidx].lightColor = lightColor;
	m_lightsData.lights[lightidx].attenuation = attenuation;
	m_lightsData.lights[lightidx].innerAngle = innerAngle;
	m_lightsData.lights[lightidx].specAttenuation = specAttenuation;
	m_lightsData.lights[lightidx].outerAngle = outerAngle;
	m_lightsData.lights[lightidx].direction = dir;
	m_lightsData.lights[lightidx].dirFactor = dirFactor;
	m_lightsData.lights[lightidx].lightFlag = 1.f;		// this light is considered as part of the calling drawcall
	m_lightsData.lights[lightidx].useShadow = useShadow;
	m_lightsData.lights[lightidx].shadowVP = shadowVP;
}

/**
* Detach current immediate mesh.
* Bad practice. Have game objects store their mesh and use only when necessary.
*/
void Renderer::DetachMeshImmediate()
{
	if (m_immediateMesh != nullptr)
	{
		delete m_immediateMesh;
		m_immediateMesh = nullptr;
	}
}


void Renderer::DrawSkybox(TextureCube* cubemap, Mesh* mesh)
{
	UseShader(CreateOrGetShader("skybox"));
	GLuint programHandle = m_currentShader->GetShaderProgram()->GetHandle();
	glUseProgram(programHandle);

	SetSampler2D(SKYBOX_TEX_BIND_IDX, cubemap->m_sampler);
	SetTextureCube2D(SKYBOX_TEX_BIND_IDX, cubemap);

	m_objectData.model = Matrix44::IDENTITY;
	SetObjectUBO(programHandle);
	SetCameraUBO(programHandle);
	SetLightsUBO(programHandle);			
	SetDebugModeUBO(programHandle);

	BindRenderState(m_currentShader->m_state);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vbo.GetHandle());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ibo.GetHandle());
	BindLayoutToProgram( programHandle, mesh->GetLayout() ); 

	// Now that it is described and bound, draw using our program
	if ( mesh->GetDrawInstruction().using_indices )
	{
		glDrawElements( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type), 
			mesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( ToGLPrimitiveType(mesh->GetDrawInstruction().primitive_type),
			0, mesh->GetVertexCount() );
	}

	GL_CHECK_ERROR();
}


//void Renderer::DrawTextInBox2D(const std::string& asciiText, float xAlign, float yAlign, float cellHeight, const AABB2& box, eDrawTextMode mode,
//	const Rgba& tint /* = Rgba::WHITE */, float aspectScale /* = 1.f */, BitmapFont* font /* = nullptr */)
//{
//	switch (mode)
//	{
//		case TEXT_DRAW_SHRINK_TO_FIT:
//		{
//			float heightShrinkRatio = 1.f;
//			float widthShrinkRatio = 1.f;
//			float uniformShrinkRatio;
//
//			std::vector<std::string> lines;
//			std::istringstream input(asciiText);
//			std::string line;
//			while (std::getline(input, line))
//			{
//				lines.push_back(line);
//			}
//
//			float boxWidth = box.maxs.x - box.mins.x;
//			float boxHeight = box.maxs.y - box.mins.y;
//			float rawTextHeight = cellHeight * lines.size();
//			float rawTextMaxWidth = -1.f;
//
//			if (rawTextHeight > boxHeight)
//			{
//				heightShrinkRatio = boxHeight / rawTextHeight;
//			}
//
//			for (unsigned int lineCount = 0; lineCount < lines.size(); ++lineCount)
//			{
//				float lineWidth = font->GetStringWidth(lines[lineCount], cellHeight, aspectScale);
//				if (lineWidth > rawTextMaxWidth)
//				{
//					rawTextMaxWidth = lineWidth;
//				}
//			}
//
//			if (rawTextMaxWidth > boxWidth)
//			{
//				widthShrinkRatio = boxWidth / rawTextMaxWidth;
//			}
//
//			uniformShrinkRatio = (heightShrinkRatio > widthShrinkRatio) ? widthShrinkRatio : heightShrinkRatio;
//
//			float shrunkTextHeight = rawTextHeight * uniformShrinkRatio;
//			float shrunkCellHeight = cellHeight * uniformShrinkRatio;
//			for (unsigned int lineCount = 0; lineCount < lines.size(); ++lineCount)
//			{
//				float shrunkLineWidth = font->GetStringWidth(lines[lineCount], shrunkCellHeight, aspectScale);
//				float minX = (boxWidth - shrunkLineWidth) * xAlign;
//				float minY = ((boxHeight - shrunkTextHeight) * yAlign) + ((lines.size() - (lineCount + 1)) * shrunkCellHeight);
//				Vector2 drawMins(minX, minY);
//				drawMins = drawMins + box.mins;
//				DrawText2D(drawMins, lines[lineCount], shrunkCellHeight, tint, aspectScale, font);
//			}
//		}
//		break;
//
//		case TEXT_DRAW_WORD_WRAP:
//		{
//			float heightShrinkRatio = 1.f;
//
//			std::vector<std::string> lines;
//			std::istringstream input(asciiText);
//			std::string line;
//
//			float boxWidth = box.maxs.x - box.mins.x;
//			float boxHeight = box.maxs.y - box.mins.y;
//
//			while (std::getline(input, line))
//			{
//				float length = font->GetStringWidth(line, cellHeight, aspectScale);
//				while (length > boxWidth)
//				{
//					for (unsigned int charCount = 0; charCount < line.length(); ++charCount)
//					{
//						float trimLength = font->GetStringWidth(line.substr(0, charCount), cellHeight, aspectScale);
//						if (trimLength > boxWidth)
//						{
//							lines.push_back(line.substr(0, charCount - 1));
//							line = line.substr(charCount - 1, line.length());
//							length = font->GetStringWidth(line, cellHeight, aspectScale);
//							break;
//						}
//					}
//				}
//				lines.push_back(line);
//			}
//
//			float rawTextHeight = cellHeight * lines.size();
//			if (rawTextHeight > boxHeight)
//			{
//				heightShrinkRatio = boxHeight / rawTextHeight;
//			}
//
//			for (unsigned int lineCount = 0; lineCount < lines.size(); ++lineCount)
//			{
//				float lineWidth = font->GetStringWidth(lines[lineCount], cellHeight, aspectScale);		
//				float minX = (boxWidth - lineWidth) * xAlign;
//				float minY = (lines.size() - (lineCount + 1)) * cellHeight * heightShrinkRatio;
//				Vector2 drawMins(minX, minY);
//				drawMins = drawMins + box.mins;
//				DrawText2D(drawMins, lines[lineCount], cellHeight * heightShrinkRatio, tint, aspectScale / heightShrinkRatio, font);
//			}
//		}
//		break;
//
//		case TEXT_DRAW_OVERRUN:
//		{
//			std::vector<std::string> lines;
//			std::istringstream input(asciiText);
//			std::string line;
//			while (std::getline(input, line))
//			{
//				lines.push_back(line);
//			}
//
//			float boxWidth = box.maxs.x - box.mins.x;
//			float boxHeight = box.maxs.y - box.mins.y;
//			float rawTextHeight = cellHeight * lines.size();
//
//			for (unsigned int lineCount = 0; lineCount < lines.size(); ++lineCount)
//			{
//				float lineWidth = font->GetStringWidth(lines[lineCount], cellHeight, aspectScale);
//				float minX = (boxWidth - lineWidth) * xAlign;
//				float minY = ((boxHeight - rawTextHeight) * yAlign) + ((lines.size() - (lineCount + 1)) * cellHeight);
//				Vector2 drawMins(minX, minY);
//				drawMins = drawMins + box.mins;
//				DrawText2D(drawMins, lines[lineCount], cellHeight, tint, aspectScale, font);
//			}
//		}
//		break;
//
//		case NUM_OF_TEXT_MODE:
//		break;
//
//		default:
//		break;
//	}
//}

void Renderer::RegisterBuiltinShaders()
{
	ShaderProgram* builtinDefaultSP = new ShaderProgram();
	builtinDefaultSP->LoadFromStrings(builtinDefaultSPVS, builtinDefaultSPFS, "USE_LIGHTING;HAS_BONE_WEIGHTS;USE_FOG;MAX_LIGHTS=8;USE_MIRROR");
	m_loadedShaderPrograms.emplace( "builtinDefault", builtinDefaultSP );

	ShaderProgram* builtinInvalidSP = new ShaderProgram();
	builtinInvalidSP->LoadFromStrings(builtinInvalidSPVS, builtinInvalidSPFS, "FOG_DENSITY=0.5;USE_LIGHTING;USE_FOG;BONE_WEIGHTS=100");
	m_loadedShaderPrograms.emplace( "builtinInvalid", builtinInvalidSP );
}


void Renderer::UseShader(Shader* shader)
{
	if (shader != nullptr)
	{
		m_currentShader = shader;
	}
	else 
	{
		m_currentShader = CreateOrGetShader("mvp_tex");
	}
}

//------------------------------------------------------------------------
Texture* Renderer::CreateRenderTarget( uint width, uint height, eTextureFormat fmt )
{
	Texture *tex = new Texture();
	tex->CreateRenderTarget( width, height, fmt );    

	return tex;
}

//------------------------------------------------------------------------
Texture* Renderer::CreateDepthStencilTarget( uint width, uint height ) 
{
	return CreateRenderTarget( width, height, TEXTURE_FORMAT_D24S8 ); 
}

//------------------------------------------------------------------------
void Renderer::SetCamera( Camera *camera ) 
{
	if (camera == nullptr)
		camera = m_defaultCamera; 

	camera->Finalize();			// make sure the framebuffer is finished being setup; 
	m_currentCamera = camera;
}


bool Renderer::CopyFrameBuffer( FrameBuffer *dst, FrameBuffer *src )
{
	// we need at least the src.
	if (src == nullptr) 
		return false; 

	// Get the handles - NULL refers to the "default" or back buffer FBO
	GLuint src_fbo = src->GetHandle();
	GLuint dst_fbo = NULL; 
	if (dst != nullptr) {
		dst_fbo = dst->GetHandle(); 
	}

	// can't copy onto ourselves
	if (dst_fbo == src_fbo)
		return false; 

	// the GL_READ_FRAMEBUFFER is where we copy from
	glBindFramebuffer( GL_READ_FRAMEBUFFER, src_fbo ); 

	// what are we copying to?
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, dst_fbo ); 

	// blit it over - get teh size
	// (we'll assume dst matches for now - but to be safe,
	// you should get dst_width and dst_height using either
	// dst or the window depending if dst was nullptr or not
	uint width = static_cast<uint>(Window::GetInstance()->GetWindowWidth());
	uint height = static_cast<uint>(Window::GetInstance()->GetWindowHeight());

	// Copy it over
	glBlitFramebuffer( 0, 0, // src start pixel
		width, height,        // src size
		0, 0,                 // dst start pixel
		width, height,        // dst size
		GL_COLOR_BUFFER_BIT,  // what are we copying (just colour)
		GL_NEAREST );         // resize filtering rule (in case src/dst don't match)

	// cleanup after ourselves
	glBindFramebuffer( GL_READ_FRAMEBUFFER, NULL ); 
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, NULL ); 

	//GL_CHECK_ERROR();

	return true;
}


void Renderer::EnableDepth(eCompare compare, bool overwrite)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(ToGLCompare(compare));
	glDepthMask(overwrite ? GL_TRUE : GL_FALSE);

	//GL_CHECK_ERROR();
}


void Renderer::DisableDepth()
{
	EnableDepth(COMPARE_ALWAYS, false);
}


void Renderer::ClearDepth(float depth)
{
	glDepthMask(GL_TRUE);
	glClearDepthf(depth);
	glClear(GL_DEPTH_BUFFER_BIT);

	GL_CHECK_ERROR();
}


void Renderer::ClearColor(Rgba color)
{
	glClearColor( color.r, color.g, color.b, color.a);
	glClear( GL_COLOR_BUFFER_BIT);

	GL_CHECK_ERROR();
}


void Renderer::ApplyEffects(ShaderProgram*)
{
	if (m_effectTarget == nullptr)
		m_effectTarget = m_defaultColorTarget;
	if (m_effectScratch == nullptr)
		m_effectScratch = Texture::CreateCompatible(m_effectTarget);

	// I want to draw a full screen quad
	m_effectCamera->SetColorTarget(m_effectScratch);
	SetCamera(m_effectCamera);

	//UseShaderProgram(sp);

	SetTexture2D(0, m_effectTarget);
	SetSampler2D(0, m_pointSampler);

	SetTexture2D(1, m_defaultDepthTarget);
	SetSampler2D(1, m_pointSampler);

	//DrawAABB(AABB2(-Vector2::ONE, Vector2::ONE), Rgba::WHITE);

	std::swap(m_effectTarget, m_effectScratch);
}


void Renderer::ApplyEffect(Material*)
{
	// use shader program to apply effects

}


void Renderer::FinishEffects()
{
	if (m_effectTarget == nullptr)
		return;

	if (m_effectTarget != m_defaultColorTarget)
	{
		CopyTexture2D(m_defaultColorTarget, m_effectTarget);
		m_effectScratch = m_effectTarget;
	}

	m_effectTarget = nullptr;
}


bool Renderer::CopyTexture2D(Texture* dst, Texture* src)
{
	FrameBuffer src_fbo;
	FrameBuffer dst_fbo;

	src_fbo.SetColorTarget(src);
	src_fbo.Finalize();

	if (dst == nullptr)
		return CopyFrameBuffer(nullptr, &src_fbo);
	else
	{
		dst_fbo.SetColorTarget(dst);
		dst_fbo.Finalize();

		return CopyFrameBuffer(&dst_fbo, &src_fbo);
	}
}

void Renderer::SetTexture2D(uint texIndex, Texture* texture)
{
	glActiveTexture(GL_TEXTURE0 + texIndex);
	glBindTexture(GL_TEXTURE_2D, texture->m_textureID);
}


void Renderer::SetTextureCube2D(uint texIndex, TextureCube* texCube)
{
	glActiveTexture(GL_TEXTURE0 + texIndex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texCube->m_handle);
}


void Renderer::SetSampler2D(uint idx, Sampler* sampler)
{
	m_defaultSampler = sampler;
	glBindSampler(idx, m_defaultSampler->GetHandle());
}

void Renderer::FillBlockProperties(sPropertyBlockInfo* out, GLuint handle, GLint blockIndex)
{
	GLint activeUniforms;
	glGetActiveUniformBlockiv( handle, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniforms ); 
	if (activeUniforms <= 0) {
		return; 
	}

	std::vector<GLint> indices(activeUniforms);
	glGetActiveUniformBlockiv( handle, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data() ); 

	std::vector<GLint> offsets(activeUniforms);
	std::vector<GLint> types(activeUniforms);
	std::vector<GLint> counts(activeUniforms);

	glGetActiveUniformsiv( handle, activeUniforms, (GLuint*)indices.data(), GL_UNIFORM_OFFSET,	offsets.data() ); 
	glGetActiveUniformsiv( handle, activeUniforms, (GLuint*)indices.data(), GL_UNIFORM_TYPE,	types.data() ); 
	glGetActiveUniformsiv( handle, activeUniforms, (GLuint*)indices.data(), GL_UNIFORM_SIZE,	counts.data() ); 

	out->SetCount(activeUniforms);
	for (GLint i = 0; i < activeUniforms; ++i) {
		char name[64]; 
		GLint len = 0; 
		glGetActiveUniformName( handle, (GLuint)indices[i], sizeof(name), &len, name ); 

		float typeSize = ToDataSize(types[i]);
		GLint dataCount = counts[i];
		uint propsize = (uint)(typeSize * dataCount);

		out->properties[i]->name = name;
		out->properties[i]->offset = offsets[i];
		out->properties[i]->size = propsize;
		out->properties[i]->m_owningBlock = out;
	}
}


void Renderer::FillInfo(ShaderProgramInfo* out, GLuint handle)
{
	out->Clear();

	glUseProgram(handle);

	GLint count;

	const GLsizei maxLen = 64;
	char name[maxLen];

	// property blocks, or UBO
	glGetProgramiv(handle, GL_ACTIVE_UNIFORM_BLOCKS, &count);
	GLsizei blockNameLength = 0;

	for (GLint bi = 0; bi < count; ++bi)
	{
		blockNameLength = 0;
		glGetActiveUniformBlockName(handle, bi, maxLen, &blockNameLength, name);
		if (blockNameLength > 0)
		{
			sPropertyBlockInfo* blockInfo = new sPropertyBlockInfo();

			// block name
			blockInfo->blockName = name;

			// block binding point
			GLint binding = -1;
			glGetActiveUniformBlockiv(handle, bi, GL_UNIFORM_BLOCK_BINDING, &binding);
			ASSERT_RECOVERABLE(binding != -1, "binding should be specified by shader");
			blockInfo->blockIdx = (uint)binding;

			// block size
			GLint blockSize = 0U;
			glGetActiveUniformBlockiv(handle, bi, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
			blockInfo->blockSize = (uint)blockSize;

			// fill properties
			FillBlockProperties(blockInfo, handle, bi);

			out->m_blockInfos.push_back(blockInfo);
		}
	}
}


bool Renderer::SetupShadowTarget()
{
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint framebufferName = 0;
	glGenFramebuffers(1, &framebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	return true;
}

void Renderer::AssimpDraw()
{
	if (m_assimpLoader != nullptr)
	{
		std::vector<Submesh*>& aiEntries = m_assimpLoader->GetEntries();
		std::vector<Texture*>& aiTextures = m_assimpLoader->GetTextures();

		for (uint i = 0; i < aiEntries.size(); ++i)
		{
			Submesh* subMesh = aiEntries[i];

			GLuint programHandle = m_currentShader->GetShaderProgram()->GetHandle();
			glUseProgram(programHandle);

			SetObjectUBO(programHandle);
			SetCameraUBO(programHandle);
			SetLightsUBO(programHandle);
			SetDebugModeUBO(programHandle);

			BindRenderState(m_currentShader->m_state);
			glBindBuffer(GL_ARRAY_BUFFER, subMesh->m_vbo.GetHandle());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh->m_ibo.GetHandle());
			BindLayoutToProgram( programHandle, subMesh->GetLayout() ); 

			const uint matIndex = subMesh->GetMatIndex();

			if (matIndex < aiTextures.size() && aiTextures[matIndex]) {
				Texture* bindTex = aiTextures[matIndex];
				SetTexture2D(0, bindTex);
				SetSampler2D(0, bindTex->GetSampler());
			}

			// Now that it is described and bound, draw using our program
			if ( subMesh->UseIndices() )
			{
				glDrawElements( ToGLPrimitiveType(subMesh->GetDrawInstruction().primitive_type), 
					subMesh->GetIndexCount(), GL_UNSIGNED_INT, 0 );
			}
			else
			{
				glDrawArrays( ToGLPrimitiveType(subMesh->GetDrawInstruction().primitive_type),
					0, subMesh->GetVertexCount() );
			}

			TODO("need glDisableVertexAttribArray(bind)?");
		}
	}
}

void Renderer::BindMaterial(const Drawcall& dc)
{
	BindShader(dc);
	BindTexture(dc);
	BindProperties(dc);
}


void Renderer::BindModelMatrix(const Drawcall& dc)
{
	// set model property
	Material* mat = dc.m_material;
	mat->SetProperty("MODEL", dc.m_model);
}


void Renderer::BindShader(const Drawcall& dc)
{
	UseShader(dc.m_material->m_shader);
}


void Renderer::BindTexture(const Drawcall& dc)
{
	for (std::map<int, Texture*>::iterator it = dc.m_material->m_textures.begin();
		it != dc.m_material->m_textures.end(); ++it)
	{
		int bindIdx = it->first;
		Texture* texture = it->second;

		SetTexture2D(bindIdx, texture);
		SetSampler2D(bindIdx, texture->GetSampler());
	}
}


void Renderer::BindProperties(const Drawcall& dc)
{
	for (std::map<std::string, PropertyBlock*>::iterator it = dc.m_material->m_blocks.begin();
		it != dc.m_material->m_blocks.end(); ++it)
	{
		PropertyBlock* block = it->second;
		glBindBufferBase(GL_UNIFORM_BUFFER, block->m_blockInfo->blockIdx, block->GetHandle());

		size_t size = block->m_blockInfo->blockSize;
		block->CopyToGPU(size, block->m_dataBlock);
	}
}


void Renderer::ClearLights()
{
	for (int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx)
	{
		Light* light = m_lights[lightIdx];

		if (light != nullptr)
		{
			m_lights[lightIdx] = nullptr;
		}
	}
}


void Renderer::ResetLightDataFlag()
{
	for (int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx)
	{
		m_lightsData.lights[lightIdx].lightFlag = 0.f;
	}
}