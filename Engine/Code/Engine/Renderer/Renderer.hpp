#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/ShaderProperties.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Vertex.hpp"


#include <string>
#include <map>
#include <vector>

#define MAX_LIGHTS 8

enum eDrawPrimitiveType
{
	DRAW_POINT,
	DRAW_LINE,
	DRAW_QUAD,
	DRAW_TRIANGLE,
	NUM_OF_PRIMITIVE_TYPE
};

enum eDrawTextMode
{
	TEXT_DRAW_SHRINK_TO_FIT,
	TEXT_DRAW_WORD_WRAP,
	TEXT_DRAW_OVERRUN,
	NUM_OF_TEXT_MODE
};

enum eDebugMode
{
	DEBUG_NONE, 
	DEBUG_COLOR,
	DEBUG_UV, 
	DEBUG_SURFACE_NORMAL,
	DEBUG_WORLD_NORMAL,
	DEBUG_VERT_NORMAL,
	DEBUG_VERT_TANGENT,
	DEBUG_VERT_BITANGENT,
	DEBUG_DIFFUSE,
	DEBUG_SPECULAR,
	DEBUG_DIFFUSE_SPECULAR,		// 10
	DEBUG_FINAL,				// 11
	DEBUG_NONLINEAR_DEPTH,		// 12
	DEBUG_LINEAR_DEPTH,			// 13
	NUM_DEBUG_MODE
};

struct sDrawInstruction
{
	eDrawPrimitiveType primitive_type; 
	uint start_index; 
	uint elem_count; 
	bool using_indices; 
};

////////////////////////////////////// UBO DATA /////////////////////////////////
struct sCameraData
{
	Vector3 eyePosition;
	float pad00;
	Matrix44 view;
	Matrix44 proj;
};

struct sLightData
{
	Vector4  lightColor;

	Vector3  lightPosition;
	float	 useShadow;

	Vector3  direction;
	float	 dirFactor;

	Vector3  attenuation;
	float	 innerAngle;

	Vector3  specAttenuation;
	float	 outerAngle;

	Vector3  pad03;
	float	 lightFlag;

	Matrix44 shadowVP;
};

struct sAllLightsData
{
	Vector4 ambience;
	sLightData lights[MAX_LIGHTS];
};

struct sLightConstData
{
	float specularAmount;
	float specularPower;
	Vector2 pad02;
};

struct sOjbectData
{
	Matrix44 model;
};

struct sColorData
{
	Vector4 rgba;
};

struct sDebugMode
{
	Vector4 mode;
};

struct sFogInfo 
{
	Vector4 fogColor;
	float fogNearPlane;
	float fogNearFactor;
	float fogFarPlane;
	float fogFarFactor;
};

struct sGameTime
{
	float gameTime;
	float gameDeltaTime;
	float appTime;
	float appDeltaTime;
};
////////////////////////////////////// UBO DATA END /////////////////////////////////

class AssimpLoader;
class Mesh;
class Light;
class Renderable;
class Drawcall;

class Renderer
{
public:
	Renderer();
	~Renderer();

	void BeginFrame();
	void EndFrame();
	void RegisterBuiltinShaders();
	void PostStartup(float startupWidth, float startupHeight);

	void ClearLights ();
	void ResetLightDataFlag();

	static Renderer* GetInstance();
	static void DestroyInstance();
	
	void ClearScreen(const Rgba& clearColor);
	void ClearDepth(float depth = 1.f);
	void EnableDepth(eCompare compare, bool overwrite);
	void DisableDepth();
	void ClearColor(Rgba color = Rgba::BLACK);

	// Camera
	Camera* GetDefaultCamera() const { return m_defaultCamera; }
	Camera* GetCurrentCamera() const { return m_currentCamera; }
	Camera* GetEffectCamera() const { return m_effectCamera; }

	void PushMatrix();
	void PopMatrix();
	void Translate(const Vector2& translation);
	void Rotate(float degrees);
	void ScaleUniform(float uniformScale);
	void ScaleX(float xScale);
	void ScaleY(float yScale);
	void ScaleXY(float xScale, float yScale);

	void DrawAABB2D( const AABB2& bounds, const Rgba& color, eVertexType type );
	void DrawText2D( const Vector2& drawMins, const std::string& asciiText, float cellHeight,
		const Rgba& tint = Rgba::WHITE, float aspectScale = 1.f, const BitmapFont* font = nullptr );
	void DrawPoint2D(const Vector2& point, const Rgba& color, eVertexType type, float size);
	void DrawPolygon2D(const Vector2& bl, const Vector2& br, const Vector2& tl,
		const Vector2& tr, const Rgba& color, eVertexType type);
	void DrawLine3D( const Vector3& start, const Vector3& end, const Rgba& color, float lineThickness );

	//void DrawLine2D( const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, float lineThickness);
	//void DrawAABB3D( const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, const Vector3& vert4, const Rgba& color);
	//void DrawTriangle( const Vector2& vert1, const Vector2& vert2, const Vector2& vert3, const Rgba& color, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3 );
	//void DrawTexturedAABB(const AABB2& bounds, const Texture& texture,
	//	const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint);
	//void DrawTexturedAABB(AABB2& bounds, Texture* texture, const Rgba& tint);
	//void DrawText3D( const Vector3& up, const Vector3& right, const Vector3& drawMinsWorldPos, const std::string text, float cellHeight,
	//	const Rgba& tint = Rgba::WHITE, float aspectScale = 1.f, const BitmapFont* font = nullptr);
	//void DrawTextInBox2D(const std::string& asciiText, float xAlign, float yAlign, float cellHeight, const AABB2& box, eDrawTextMode mode,
	//	const Rgba& tint = Rgba::WHITE, float aspectScale = 1.f, BitmapFont* font = nullptr);
	//void DrawCube3D(const Vector3& center, const Vector3& dimensions, const Rgba& color = Rgba::WHITE, 
	//	AABB2 uv_top = AABB2::ZERO_TO_ONE, AABB2 uv_side = AABB2::ZERO_TO_ONE, AABB2 uv_bottom = AABB2::ZERO_TO_ONE);
	//void DrawOrientedQuad(const Vector3& center, const Vector3& orientation, const Texture& tex, float width, float height,
	//	const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint, float scaleX = 1.f, float scaleY = 1.f);
	//void DrawOrientedQuadLocal(const Vector3& center, const Vector3& orientation, const Texture& tex, float width, float height,
	//	const Vector2& texCoordsAtMins, const Vector2& texCoordsAtMaxs, const Rgba& tint, float scaleX = 1.f, float scaleY = 1.f);

	void Draw(const Drawcall& dc);
	void DrawMesh(Mesh* mesh, bool culling = true, bool depth_test = true);
	void DrawMeshImmediate();
	void DetachMeshImmediate();
	void DrawSkybox(TextureCube* skybox, Mesh* mesh);

	void Bind(const Texture& texture);
	void Unbind();
	void BlendOne();
	void BlendAlpha();

	Texture*		CreateOrGetTexture(std::string fp, bool mipmap = false);
	Mesh*			CreateOrGetMesh(std::string meshName);
	Sprite*			CreateOrGetSprite(std::string id);
	BitmapFont*		CreateOrGetBitmapFont(const char* bitmapFontName);
	ShaderProgram*	CreateOrGetShaderProgram(const char* fileName, const char* delimited = "");
	Shader*			CreateOrGetShader(std::string shaderName);
	Material*		CreateOrGetMaterial(std::string matName);
	Material*		CreateOrGetStagedMaterial(std::string matName);
	ShaderChannel*  CreateOrGetShaderChannel(std::string channelName);

	void ReloadShaders();
	void UseShader( Shader *shader );

	bool	 CopyFrameBuffer( FrameBuffer *dst, FrameBuffer *src );
	Texture* CreateRenderTarget(uint width, uint height, eTextureFormat fmt = TEXTURE_FORMAT_RGBA8);
	Texture* CreateDepthStencilTarget( uint width, uint height ); 
	Texture* GetDefaultColorTarget() const { return m_defaultColorTarget; }
	Texture* GetDefaultDepthTarget() const { return m_defaultDepthTarget; }

	void SetCamera(Camera* cam);
	void SetEffectCamera(Camera* cam) { m_effectCamera = cam; }

	// Effects
	void ApplyEffects(ShaderProgram* sp);
	void ApplyEffect(Material* mat);
	void FinishEffects();
	void SetTexture2D(uint texIndex, Texture* texture);
	void SetTextureCube2D(uint texIndex, TextureCube* texCube);
	void SetSampler2D(uint texIndex, Sampler* sampler);
	bool CopyTexture2D(Texture* dst, Texture* src);

	// Drawcall
	void BindMaterial(const Drawcall& dc);
	void BindModelMatrix(const Drawcall& dc);
	void BindShader(const Drawcall& dc);
	void BindTexture(const Drawcall& dc);
	void BindProperties(const Drawcall& dc);

	// Shader/uniforms
	void BindRenderState(const sRenderState& state, bool culling = true, bool depth_test = true);
	void SetImmediateMesh(Mesh* mesh) { m_immediateMesh = mesh; }
	void SetUniform(const char* name, float f);
	void SetUniform(const char* name, Vector3 v3);
	void SetUniform(const char* name, Vector4 v4);
	void SetUniform(const char* name, Matrix44 m44 );
	void SetCameraUBO(GLuint handle);
	void SetLightsUBO(GLuint handle);
	void SetObjectUBO(GLuint handle);
	void SetColorUBO(GLuint handle);
	void SetDebugModeUBO(GLuint handle);
	void SetGameTimeUBO(GLuint handle);
	void SetFogInfoUBO(GLuint handle);
	void SetCameraUniforms();
	void SetAmbience(Vector4 ambience) { m_lightsData.ambience = ambience; }
	void ConfigureLightData(Vector3 lightPosition, Vector4 lightColor,
		Vector3 attenuation, float innerAngle, 
		Vector3 specAttenuation, float outerAngle, 
		Vector3 dir, float dirFactor, int lightidx,
		bool useShadow, Matrix44& shadowVP);
	Shader* GetCurrentShader() { return m_currentShader; }

	// Light
	Vector4 GetAmbience() const { return m_lightsData.ambience; }

	void FillBlockProperties(sPropertyBlockInfo* out, GLuint handle, GLint blockIndex);
	void FillInfo(ShaderProgramInfo* out, GLuint handle);

	// shadows
	bool SetupShadowTarget();

	// assimp
	void DrawModel(AssimpLoader* loader);

public:
	const static int DIFFUSE_MAP_BIND_IDX = 0;
	const static int NORMAL_MAP_BIND_IDX = 1;
	const static int SHADOW_MAP_BIND_IDX = 11;

	const static int CAMERA_BUFFER_BIND_IDX = 2;
	const static int LIGHTS_BUFFER_BIND_IDX = 3;
	const static int LIGHT_CONST_BUFFER_BIND_IDX = 4;
	const static int OBJECT_BUFFER_BIND_IDX = 5;
	const static int DEBUG_MODE_BUFFER_BIND_IDX = 6;
	const static int COLOR_BIND_IDX = 7;
	const static int SKYBOX_TEX_BIND_IDX = 8;
	const static int GAME_FRAME_BIND_IDX = 9;
	const static int FOG_BIND_IDX = 10;

	std::map<std::string, Texture*>			m_loadedTextures;
	std::map<std::string, Sprite*>			m_loadedSprites;
	std::map<std::string, BitmapFont*>		m_loadedFonts;
	std::map<std::string, ShaderProgram*>	m_loadedShaderPrograms;
	std::map<std::string, Mesh*>			m_loadedMeshes;
	std::map<std::string, Shader*>			m_loadedShaders;
	std::map<std::string, ShaderChannel*>	m_loadedChannels;
	std::map<std::string, Material*>		m_loadedMaterials;

	Shader*			m_currentShader;
	Shader*			m_defaultShader;
	RenderBuffer*   m_immediateBuffer;
	Sampler*		m_defaultSampler;
	Sampler*		m_pointSampler;
	GLuint			m_defaultVAO;

	Camera*  m_defaultCamera;
	Camera*  m_currentCamera;
	Camera*  m_effectCamera;
	Texture* m_defaultColorTarget;
	Texture* m_defaultDepthTarget;
	Texture* m_effectTarget;
	Texture* m_effectScratch;
	Mesh*    m_immediateMesh;

	static Renderer* m_rendererInstance;

	UniformBuffer* m_uboCamera;
	UniformBuffer* m_uboLights;
	UniformBuffer* m_uboObjects;
	UniformBuffer* m_uboColors;
	UniformBuffer* m_uboDebugMode;
	UniformBuffer* m_uboGameTime;
	UniformBuffer* m_uboFogInfo;

	sCameraData		m_cameraData;			
	sAllLightsData  m_lightsData;
	sLightConstData m_lightConstData;
	sOjbectData		m_objectData;
	sColorData		m_colorData;
	sDebugMode		m_debugModeData;
	sGameTime		m_timeData;
	sFogInfo		m_fogData;

	Light* m_lights[MAX_LIGHTS];
};