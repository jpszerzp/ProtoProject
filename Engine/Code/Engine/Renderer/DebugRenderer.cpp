#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Cube.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Renderer/Camera.hpp"
static std::vector<DebugRenderTask*> g_debugRenderTasks;
static Camera* g_2DCamera; 
static Camera* g_3DCamera; 
static bool g_debugrenderOn = false;

void DebugRenderStartup()
{
	// By default, debug render mode is on
	g_debugrenderOn = true;
}


void DebugRenderShutdown()
{
	g_debugrenderOn = false;

	DebugRenderTasksFlush();
}

void DebugRenderUpdate(float deltaTime)
{
	if (g_debugrenderOn)
	{
		for each (DebugRenderTask* task in g_debugRenderTasks)
		{
			task->Update(deltaTime);
		}

		// Check if task is dead
		for (int taskIdx = (int)(g_debugRenderTasks.size() - 1U);
			taskIdx >= 0; --taskIdx)
		{
			DebugRenderTask* task = g_debugRenderTasks[taskIdx];

			if (task->IsDead())
			{
				delete task;

				// Fast removal in vector
				// Replace with last element
				g_debugRenderTasks[taskIdx] = g_debugRenderTasks[g_debugRenderTasks.size() - 1U];
				g_debugRenderTasks.pop_back();
			}
		}
	}
}


void DebugRenderRenderAll(Renderer* renderer)
{
	if (g_debugrenderOn)
	{
		if (!g_debugRenderTasks.empty())
		{
			for each (DebugRenderTask* task in g_debugRenderTasks)
			{
				task->Render(renderer);
			}
		}
	}
}


void DebugRenderRender3D(Renderer* renderer)
{
	if (g_debugrenderOn)
	{
		if (!g_debugRenderTasks.empty())
		{
			for each (DebugRenderTask* task in g_debugRenderTasks)
			{
				if (!task->m_2D)
				{
					task->Render(renderer);
				}
			}
		}
	}
}


void DebugRenderRender2D(Renderer* renderer)
{
	if (g_debugrenderOn)
	{
		if (!g_debugRenderTasks.empty())
		{
			for each (DebugRenderTask* task in g_debugRenderTasks)
			{
				if (task->m_2D)
				{
					task->Render(renderer);
				}
			}
		}
	}
}


void DebugRenderTasksFlush()
{
	for (DebugRenderTask* task : g_debugRenderTasks)
	{
		delete task;
		task = nullptr;
	}

	g_debugRenderTasks.clear();
}


void DebugRenderToggle()
{
	g_debugrenderOn = !g_debugrenderOn;
}


void SetDebugRenderOn(bool on)
{
	g_debugrenderOn = on;
}


bool DebugRenderOn()
{
	return g_debugrenderOn;
}


void DebugRenderSet3DCamera( Camera* camera )
{
	g_3DCamera = camera;
}


void DebugRenderSet2DCamera( Camera* camera )
{
	g_2DCamera = camera;
}

Camera* GetDebugRender3DCamera()
{
	return g_3DCamera;
}

Camera* GetDebugRender2DCamera()
{
	return g_2DCamera;
}

DebugRenderTask::~DebugRenderTask()
{
	delete m_debugObj;
	m_debugObj = nullptr;
}


void DebugRenderTask::Age(float deltaTime)
{
	m_timeTillDeath -= deltaTime;
}


bool DebugRenderTask::IsDead()
{
	return (m_timeTillDeath <= 0.f);
}


float DebugRenderTask::GetNormalizedAge()
{
	if (m_debugOption.m_lifeTime <= 0.f )
	{
		return 1.f;
	}

	float alive = m_timeTillDeath / m_debugOption.m_lifeTime;
	float dead = 1.f - alive;

	return ClampZeroToOne(dead);
}


DebugRenderTaskAABB3::DebugRenderTaskAABB3(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	m_debugOption = sDebugRenderOption(startTimeColor, endTimeColor, lifeTime, mode);
	m_timeTillDeath = lifeTime;
	m_2D = false;
}


void DebugRenderTaskAABB3::SetDebugObject(AABB3 bounds)
{
	Renderer* renderer = Renderer::GetInstance();

	Vector3 center	= (bounds.m_min	  + bounds.m_max)	/ 2.f;
	float	scaleX	= (bounds.m_max.x - bounds.m_min.x) / 1.f;
	float	scaleY	= (bounds.m_max.y - bounds.m_min.y) / 1.f;
	float	scaleZ	= (bounds.m_max.z - bounds.m_min.z) / 1.f;

	m_debugObj = new Cube(center, Vector3::ZERO, Vector3(scaleX, scaleY, scaleZ), Rgba::WHITE, "cube_pcu", "", MOVE_STATIC, BODY_PARTICLE);

	// fill texture and shader of material only for debug object
	// this is because debug obj material is not defined from .mat
	Material* material = m_debugObj->m_renderable->GetMaterial();				// empty material
	Shader* shader = renderer->CreateOrGetShader("wireframe");
	material->m_shader = shader;
	material->FillPropertyBlocks();
	material->FillTextures();
}


void DebugRenderTaskAABB3::Update(float deltaTime)
{
	m_debugObj->Update(deltaTime);

	// Lerp color
	float timePassed = GetNormalizedAge();
	Rgba tint = Interpolate(m_debugOption.m_startTimeColor, m_debugOption.m_endTimeColor, timePassed);
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);
	m_debugObj->m_renderable->m_tint = tintVec4;

	Age(deltaTime);
}


void DebugRenderTaskAABB3::Render(Renderer* renderer)
{
	renderer->SetCamera(g_3DCamera);
	m_debugObj->Render(renderer);
}


DebugRenderTaskQuad::DebugRenderTaskQuad(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	m_debugOption = sDebugRenderOption(startTimeColor, endTimeColor, lifeTime, mode);
	m_timeTillDeath = lifeTime;
}


void DebugRenderTaskQuad::SetDebugObject(Vector3 drawmin, Vector3 up, Vector3 right, float width, float height, std::string shaderName, bool isFont)
{

}


void DebugRenderTaskQuad::SetDebugObject(Vector3 drawmin, Vector3 up, Vector3 right, float width, float height, std::string shaderName, bool isFont, Vector2 uvBL, Vector2 uvBR, Vector2 uvTL, Vector2 uvTR)
{

}


void DebugRenderTaskQuad::Update(float deltaTime)
{
	m_debugObj->Update(deltaTime);

	// Lerp color
	float timePassed = GetNormalizedAge();
	Rgba tint = Interpolate(m_debugOption.m_startTimeColor, m_debugOption.m_endTimeColor, timePassed);
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);
	m_debugObj->m_renderable->m_tint = tintVec4;

	Age(deltaTime);
}


void DebugRenderTaskQuad::Render(Renderer* renderer)
{
	if (!m_2D)
	{
		renderer->SetCamera(g_3DCamera);
	}
	else
	{
		renderer->SetCamera(g_2DCamera);
	}

	m_debugObj->Render(renderer);
}


DebugRenderTaskPoint::DebugRenderTaskPoint(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	m_debugOption = sDebugRenderOption(startTimeColor, endTimeColor, lifeTime, mode);
	m_timeTillDeath = lifeTime;
}

void DebugRenderTaskPoint::SetDebugObject(Vector3 position, Vector3 rot, float size, std::string shaderName)
{

}


void DebugRenderTaskPoint::Update(float deltaTime)
{
	m_debugObj->Update(deltaTime);

	float timePassed = GetNormalizedAge();
	Rgba cubeTint = Interpolate(m_debugOption.m_startTimeColor, m_debugOption.m_endTimeColor, timePassed);
	Vector4 tintVec4;
	cubeTint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);
	m_debugObj->m_renderable->m_tint = tintVec4;

	Age(deltaTime);
}


void DebugRenderTaskPoint::Render(Renderer* renderer)
{
	if (!m_2D)
		renderer->SetCamera(g_3DCamera);
	else
		renderer->SetCamera(g_2DCamera);

	m_debugObj->Render(renderer);
}


DebugRenderTaskLine::DebugRenderTaskLine(float lifeTime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	m_debugOption = sDebugRenderOption(startTimeColor, endTimeColor, lifeTime, mode);
	m_timeTillDeath = lifeTime;
}


void DebugRenderTaskLine::SetDebugInfo(Vector3 start, Vector3 end, float lineWidth)
{
	m_start = start;
	m_end = end;
	m_thickness = lineWidth;
}

void DebugRenderTaskLine::Update(float deltaTime)
{
	float timePassed = GetNormalizedAge();
	m_tint = Interpolate(m_debugOption.m_startTimeColor, m_debugOption.m_endTimeColor, timePassed);
	Age(deltaTime);
}


void DebugRenderTaskLine::Render(Renderer* renderer)
{
	renderer->SetCamera(g_3DCamera);
	renderer->DrawLine3D(m_start, m_end, m_tint, m_thickness);
}


DebugRenderTaskText2D::DebugRenderTaskText2D(float lifetime, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	m_debugOption = sDebugRenderOption(startTimeColor, endTimeColor, lifetime, mode);
	m_timeTillDeath = lifetime;
}


DebugRenderTaskText2D::~DebugRenderTaskText2D()
{
	// mesh is immediate and needs to be destroyed
	delete m_textMesh;
	m_textMesh = nullptr;
}

void DebugRenderTaskText2D::SetDebugInfo(std::string text, Vector2 drawmin, float cellHeight, float aspectScale)
{
	m_text = text;
	m_drawmin = drawmin;
	m_cellHeight = cellHeight;
	m_aspectScale = aspectScale;

	Renderer* renderer = Renderer::GetInstance();
	m_font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	m_textMesh = Mesh::CreateTextImmediate(m_tint, m_drawmin, m_font,
		m_cellHeight, m_aspectScale, m_text, VERT_PCU);
}


void DebugRenderTaskText2D::Update(float deltaTime)
{
	float timePassed = GetNormalizedAge();
	m_tint = Interpolate(m_debugOption.m_startTimeColor, m_debugOption.m_endTimeColor, timePassed);

	if (m_textMesh != nullptr)
	{
		delete m_textMesh;
		m_textMesh = nullptr;
	}
	m_textMesh = Mesh::CreateTextImmediate(m_tint, m_drawmin, m_font,
		m_cellHeight, m_aspectScale, m_text, VERT_PCU);

	Age(deltaTime);
}


void DebugRenderTaskText2D::Render(Renderer* renderer)
{
	renderer->SetCamera(g_2DCamera);
	if (m_textMesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->DrawMesh(m_textMesh);
	}
}


void DebugRenderWireAABB3( float lifetime, const AABB3& bounds, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode )
{
	// set time countdown and debug render option 
	DebugRenderTaskAABB3* aabb3Task = new DebugRenderTaskAABB3(lifetime, startTimeColor, endTimeColor, mode);
	aabb3Task->m_2D = false;

	// set debug object 
	aabb3Task->SetDebugObject(bounds);

	g_debugRenderTasks.push_back(aabb3Task);
}


void DebugRenderQuad(float lifetime, Vector3 drawmin, Vector3 up, Vector3 right, float width, float height, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode, bool isFont)
{
	DebugRenderTaskQuad* quadTask = new DebugRenderTaskQuad(lifetime, startTimeColor, endTimeColor, mode);
	quadTask->m_2D = false;

	quadTask->SetDebugObject(drawmin, up, right, width, height, "default", isFont);

	g_debugRenderTasks.push_back(quadTask);
}


void DebugRenderLine(float lifeTime, Vector3 start, Vector3 end, float lineWidth, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	DebugRenderTaskLine* lineTask = new DebugRenderTaskLine(lifeTime, startTimeColor, endTimeColor, mode);
	lineTask->m_2D = false;

	lineTask->SetDebugInfo(start, end, lineWidth);

	g_debugRenderTasks.push_back(lineTask);
}


void DebugRenderBasis(float lifetime, Vector3 start, Vector3 endX, Vector3 endY, Vector3 endZ, float lineWidth, eDebugDrawMode mode)
{
	DebugRenderTaskLine* lineTaskX = new DebugRenderTaskLine(lifetime, Rgba::RED, Rgba::RED, mode);
	lineTaskX->m_2D = false;
	DebugRenderTaskLine* lineTaskY = new DebugRenderTaskLine(lifetime, Rgba::GREEN, Rgba::GREEN, mode);
	lineTaskY->m_2D = false;
	DebugRenderTaskLine* lineTaskZ = new DebugRenderTaskLine(lifetime, Rgba::BLUE, Rgba::BLUE, mode);
	lineTaskZ->m_2D = false;

	lineTaskX->SetDebugInfo(start, endX, lineWidth);
	lineTaskY->SetDebugInfo(start, endY, lineWidth);
	lineTaskZ->SetDebugInfo(start, endZ, lineWidth);

	g_debugRenderTasks.push_back(lineTaskX);
	g_debugRenderTasks.push_back(lineTaskY);
	g_debugRenderTasks.push_back(lineTaskZ);
}

void DebugRenderPlaneGrid(float lifetime, Vector3 bl, Vector3 tl, Vector3 tr, Vector3 br, float intervalX, float intervalY, float lineWidth, eDebugDrawMode mode)
{
	Vector3 right = br - bl;
	Vector3 forward = tl - bl;

	Vector3 rightN = right.GetNormalized();
	Vector3 forwardN = forward.GetNormalized();
	
	float rightLength = right.GetLength();
	float forwardLength = forward.GetLength();
	
	int numVertX = (int)(rightLength / intervalX) + 1;
	int numVertY = (int)(forwardLength / intervalY) + 1;

	// vertical lines
	for (int i = 0; i < numVertX; ++i)
	{
		DebugRenderTaskLine* line = new DebugRenderTaskLine(lifetime, Rgba::WHITE, Rgba::WHITE, mode);
		line->m_2D = false;

		Vector3 start;
		Vector3 end;
		if (i == (numVertX - 1))
		{
			start = br;
			end = br + (tl - bl);
		}
		else
		{
			start = bl + (rightN * intervalX) * (float)i;
			end = start + (tl - bl);
		}

		line->SetDebugInfo(start, end, lineWidth);
		
		g_debugRenderTasks.push_back(line);
	}

	// horizontal lines
	for (int j = 0; j < numVertY; ++j)
	{
		DebugRenderTaskLine* line = new DebugRenderTaskLine(lifetime, Rgba::WHITE, Rgba::WHITE, mode);
		line->m_2D = false;

		Vector3 start;
		Vector3 end;
		if (j == (numVertY - 1))
		{
			start = tl;
			end = tl + (br - bl);
		}
		else
		{
			start = bl + (forwardN * intervalY) * (float)j;
			end = start + (br - bl);
		}

		line->SetDebugInfo(start, end, lineWidth);

		g_debugRenderTasks.push_back(line);
	}
}


void DebugRenderPoint( float lifetime,
	float size,
	Vector3 const &position, 
	Rgba const &startTimeColor, 
	Rgba const &endTimeColor,
	eDebugDrawMode const mode )
{
	DebugRenderTaskPoint* pointTask = new DebugRenderTaskPoint(lifetime, startTimeColor, endTimeColor, mode);
	pointTask->m_2D = false;

	pointTask->SetDebugObject(position, Vector3::ZERO, size, "plain_white");

	g_debugRenderTasks.push_back(pointTask);
}

void DebugRender2DQuad( float lifetime, Vector2 drawmin, float width, float height, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode, std::string shaderName, bool isFont)
{
	DebugRenderTaskQuad* quadTask2D = new DebugRenderTaskQuad(lifetime, startTimeColor, endTimeColor, mode);
	quadTask2D->m_2D = true;

	Vector3 drawmin3D = drawmin.ToVector3(0.f);
	Vector3 up3D = Vector3(0.f, 1.f, 0.f);
	Vector3 right3D = Vector3(-1.f, 0.f, 0.f);

	quadTask2D->SetDebugObject(drawmin3D, up3D, right3D, width, height, shaderName, isFont);

	g_debugRenderTasks.push_back(quadTask2D);
}


void DebugRender2DText(std::string text, float lifetime, Vector2 drawmin, float cellHeight, 
	float aspectScale, const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode)
{
	DebugRenderTaskText2D* text2DTask = new DebugRenderTaskText2D(lifetime, startTimeColor, endTimeColor, mode);
	text2DTask->m_2D = true;

	text2DTask->SetDebugInfo(text, drawmin, cellHeight, aspectScale);

	g_debugRenderTasks.push_back(text2DTask);
}

/*
void DebugRender2DChar(char character, float lifetime, Vector2 drawmin, float cellHeight, float aspectScale,
	const Rgba& startTimeColor, const Rgba& endTimeColor, eDebugDrawMode mode, BitmapFont* font)
{
	AABB2 uvBound = font->GetUVsForGlyph(character);
	std::vector<Vector2> corners = uvBound.GetCornersFromBLInCounterClockwise();
	Vector2 uvBL = corners[0];
	Vector2 uvBR = corners[1];
	Vector2 uvTL = corners[3];
	Vector2 uvTR = corners[2];

	float width = aspectScale * cellHeight;
	DebugRender2DQuadTextured(lifetime, drawmin, width, cellHeight, startTimeColor, endTimeColor, mode, "cutout", uvBL, uvBR, uvTL, uvTR, true);
}

void DebugRender2DQuadTextured(float lifetime, Vector2 drawmin, float width, float height, Rgba const &startTimeColor, Rgba const &endTimeColor,
	eDebugDrawMode mode, std::string shaderName, Vector2 uvBL, Vector2 uvBR, Vector2 uvTL, Vector2 uvTR, bool isFont)
{
	DebugRenderTaskQuad* quadTask2D = new DebugRenderTaskQuad(lifetime, startTimeColor, endTimeColor, mode);
	quadTask2D->m_2D = true;

	Vector3 drawmin3D = drawmin.ToVector3(0.f);
	Vector3 up3D = Vector3(0.f, 1.f, 0.f);
	Vector3 right3D = Vector3(-1.f, 0.f, 0.f);

	quadTask2D->SetDebugObject(drawmin3D, up3D, right3D, width, height, shaderName, isFont, uvBL, uvBR, uvTL, uvTR);

	g_debugRenderTasks.push_back(quadTask2D);
}
*/