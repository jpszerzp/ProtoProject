#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/GameObject.hpp"

///************************************************************************/
///*                                                                      */
///* STRUCTS                                                              */
///*                                                                      */
///************************************************************************/
enum eDebugDrawMode 
{
	DEBUG_RENDER_IGNORE_DEPTH, // will always draw and be visible 
	DEBUG_RENDER_USE_DEPTH,    // draw using normal depth rules
	DEBUG_RENDER_HIDDEN,       // only draws if it would be hidden by depth
	DEBUG_RENDER_XRAY,         // always draws, but hidden area will be drawn differently
	NUM_DEBUG_RENDER_MODE
};


struct sDebugRenderOption
{
	sDebugRenderOption()
		: m_startTimeColor(Rgba::WHITE)
		, m_endTimeColor(Rgba::WHITE)
		, m_lifeTime(0.0f)
		, m_mode(DEBUG_RENDER_USE_DEPTH)
	{}

	sDebugRenderOption( Rgba const &startTimeColor,
		Rgba const &endTimeColor,
		float lifeTime,
		eDebugDrawMode mode = DEBUG_RENDER_USE_DEPTH)
		: m_startTimeColor(startTimeColor),
		m_endTimeColor(endTimeColor),
		m_lifeTime(lifeTime),
		m_mode(mode)
	{}

	float			m_lifeTime; 
	Rgba			m_startTimeColor; 
	Rgba			m_endTimeColor; 
	eDebugDrawMode	m_mode; 
}; 
///************************************************************************/
///*                                                                      */
///* FUNCTIONS                                                            */
///*                                                                      */
///************************************************************************/
//
///************************************************************************/
///* Debug Render System                                                  */
///************************************************************************/
//
//// Setup the system - allocate whatever internal structures,
//// cameras, and materials are needed
void DebugRenderStartup();
//
//// Cleanup the system
void DebugRenderShutdown();
//
//// Called every frame when I want to
//// to have the rendering applied
void DebugRenderUpdate(float deltaTime);
void DebugRenderRenderAll(Renderer* renderer);
void DebugRenderRender3D(Renderer* renderer);
void DebugRenderRender2D(Renderer* renderer);

void DebugRenderTasksFlush();
void DebugRenderToggle();
void SetDebugRenderOn(bool on);
bool DebugRenderOn();

void DebugRenderSet3DCamera( Camera* camera ); 
void DebugRenderSet2DCamera( Camera* camera );

Camera* GetDebugRender3DCamera();
Camera* GetDebugRender2DCamera();

class DebugRenderTask
{
public:
	~DebugRenderTask();

	virtual void Render(Renderer* renderer) = 0;
	virtual void Update(float deltaTime) = 0;

	void	Age(float deltaTime);
	bool	IsDead();
	float	GetNormalizedAge();	// 0 when created; 1 when dead

public:
	bool				m_2D;
	float				m_timeTillDeath;
	sDebugRenderOption	m_debugOption;
	GameObject*			m_debugObj;
};

class DebugRenderTaskQuad : public DebugRenderTask
{
public:
	void SetDebugObject(Vector3 drawmin, Vector3 up, Vector3 right, float width, float height, std::string shaderName, bool isFont);
	void SetDebugObject(Vector3 drawmin, Vector3 up, Vector3 right, float width, float height, std::string shaderName, bool isFont, Vector2 uvBL, Vector2 uvBR, Vector2 uvTL, Vector2 uvTR);
	
	virtual void Render(Renderer* renderer) override;
	virtual void Update(float deltaTime) override;

	DebugRenderTaskQuad(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode);
};

// causing memroy leak for now
class DebugRenderTaskPoint : public DebugRenderTask
{
public:
	void SetDebugObject(Vector3 position, Vector3 rot, float size, std::string shaderName);

	virtual void Render(Renderer* renderer) override;
	virtual void Update(float deltaTime) override;

	DebugRenderTaskPoint(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode);
};

class DebugRenderTaskLine : public DebugRenderTask
{
public:
	void SetDebugInfo(Vector3 start, Vector3 end, float lineWidth);

	virtual void Render(Renderer* renderer) override;
	virtual void Update(float deltaTime) override;

	DebugRenderTaskLine(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode);

public:
	Rgba    m_tint;
	Vector3 m_start;
	Vector3 m_end;
	float   m_thickness;
};

class DebugRenderTaskText2D : public DebugRenderTask
{
public:
	Rgba		m_tint;
	std::string m_text;
	Vector2		m_drawmin;
	float		m_cellHeight;
	float		m_aspectScale;
	Mesh*		m_textMesh = nullptr;
	BitmapFont* m_font = nullptr;

public:
	void SetDebugInfo(std::string text, Vector2 drawmin, float cellHeight, float aspectScale);

	virtual void Render(Renderer* renderer) override;
	virtual void Update(float deltaTime) override;

	DebugRenderTaskText2D(float lifetime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode);
	~DebugRenderTaskText2D();
};

void DebugRenderQuad(float lifetime,
	Vector3 drawmin, Vector3 up, Vector3 right,
	float width, float height,
	Rgba const &startTimeColor,
	Rgba const &endTimeColor,
	eDebugDrawMode mode,
	bool isFont = false);

void DebugRenderPoint( float lifetime, 
	float size,
	Vector3 const &position, 
	Rgba const &startTimeColor, 
	Rgba const &endTimeColor,
	eDebugDrawMode const mode ); 

void DebugRenderLine(float lifeTime, 
	Vector3 start, Vector3 end, float lineWidth, 
	const Rgba& startTimeColor, const Rgba& endTimeColor, 
	eDebugDrawMode mode);

void DebugRenderBasis(float lifetime,
	Vector3 start, Vector3 endX, Vector3 endY, Vector3 endZ,
	float lineWidth, eDebugDrawMode mode);

void DebugRenderPlaneGrid(float lifetime,
	Vector3 bl, Vector3 tl, Vector3 tr, Vector3 br,
	float intervalX, float intervalY, float lineWidth, eDebugDrawMode mode);

void DebugRender2DQuad( 
	float lifetime, 
	Vector2 drawmin, float width, float height,
	Rgba const &startTimeColor, 
	Rgba const &endTimeColor,
	eDebugDrawMode mode,
	std::string shaderName,
	bool isFont = false); 

void DebugRender2DText(std::string text, float lifetime, Vector2 drawmin, float cellHeight, 
	float aspectScale, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode);