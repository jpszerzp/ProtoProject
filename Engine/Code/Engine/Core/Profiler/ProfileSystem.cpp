#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Core/Util/RenderUtil.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/InputSystem.hpp"

#include <algorithm>

// These are data subject to the one and only "Profiler".
// These are not for profiler tree...
uint64_t g_start_frame_hpc = 0U;
uint64_t g_end_frame_hpc = 0U;
uint64_t g_last_frame_hpc = 0U;
uint64_t g_total_hpc = 0U; 
uint64_t g_max_hpc = 0LLU;	// sets to max uint64_t
//uint64_t g_min_hpc = ~0LLU; 

uint64_t g_my_start_hpc = 0U;
uint64_t g_my_end_hpc = 0U;
uint64_t g_my_last_hpc = 0U;
uint64_t g_my_total_hpc = 0U;
uint64_t g_my_max_hpc = 0LLU;

uint64_t g_physx_start_hpc = 0U;
uint64_t g_physx_end_hpc = 0U;
uint64_t g_physx_last_hpc = 0U;
uint64_t g_physx_total_hpc = 0U;
uint64_t g_physx_max_hpc = 0LLU;

Profiler* Profiler::m_profilerInstance = nullptr;

const Vector2 Profiler::VERT_DISP = Vector2(0.f, Profiler::TEXT_HEIGHT);

bool IsProfilerOn()
{
	Profiler* profiler = Profiler::GetInstance();
	return profiler->m_on;
}

bool IsProfilerPaused()
{
	Profiler* profiler = Profiler::GetInstance();
	return profiler->m_paused;
}

Profiler::Profiler()
{
	// section box
	Renderer* renderer = Renderer::GetInstance();
	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	PROFILER_WIDTH = width - 2 * FRAME_GRAPH_MARGIN;
	PROFILER_HEIGHT = height;
	Vector2 min = Vector2(-width / 2.f, -height / 2.f);
	Vector2 max = Vector2(width / 2.f, height / 2.f);
	m_profilerBox = AABB2(min, max);

	Vector2 frameGraphMin = Vector2(-width / 2.f + FRAME_GRAPH_MARGIN, height / 2.f  - 
		FRAME_TEXT_LINE * TEXT_HEIGHT - TEXT_HEIGHT - 
		FRAME_GRAPH_LINE * TEXT_HEIGHT);
	Vector2 frameGraphMax = frameGraphMin + Vector2(width - 2 * FRAME_GRAPH_MARGIN, FRAME_GRAPH_LINE * TEXT_HEIGHT);
	m_frameGraphBox = AABB2(frameGraphMin, frameGraphMax);

	SUB_PROFILER_WIDTH = width / 2.f - 2 * FRAME_GRAPH_MARGIN;
	Vector2 my_frameGraphMin = frameGraphMin - Vector2(0.f, TEXT_HEIGHT + FRAME_GRAPH_LINE * TEXT_HEIGHT);
	Vector2 my_frameGraphMax = my_frameGraphMin + Vector2(SUB_PROFILER_WIDTH, FRAME_GRAPH_LINE * TEXT_HEIGHT);
	m_my_frame_box = AABB2(my_frameGraphMin, my_frameGraphMax);

	Vector2 physx_frameGraphMin = my_frameGraphMin + Vector2(width / 2.f, 0.f);
	Vector2 physx_frameGraphMax = physx_frameGraphMin + Vector2(SUB_PROFILER_WIDTH, FRAME_GRAPH_LINE * TEXT_HEIGHT);
	m_physx_frame_box = AABB2(physx_frameGraphMin, physx_frameGraphMax);

	// mesh
	Vector3 bl = m_profilerBox.mins.ToVector3(0.f);
	Vector3 br = bl + Vector3(m_profilerBox.GetDimensions().x, 0.f, 0.f);
	Vector3 tl = bl + Vector3(0.f, m_profilerBox.GetDimensions().y, 0.f);
	Vector3 tr = m_profilerBox.maxs.ToVector3(0.f);
	Rgba tint = Rgba::BLUE_HALF_OPACITY;
	m_backgroundMesh = Mesh::CreateQuadImmediate(VERT_PCU, bl, br, tl, tr, tint);

	Vector3 frameGraphBL = m_frameGraphBox.mins.ToVector3(0.f);
	Vector3 frameGraphBR = frameGraphBL + Vector3(width - 2 * FRAME_GRAPH_MARGIN, 0.f, 0.f);
	Vector3 frameGraphTL = frameGraphBL + Vector3(0.f, FRAME_GRAPH_LINE * TEXT_HEIGHT, 0.f);
	Vector3 frameGraphTR = m_frameGraphBox.maxs.ToVector3(0.f);
	Rgba frameGraphTint = Rgba::WHITE_HALF_OPACITY;
	m_frameGraphMesh = Mesh::CreateQuadImmediate(VERT_PCU, frameGraphBL, frameGraphBR,
		frameGraphTL, frameGraphTR, frameGraphTint);

	frameGraphBL = m_my_frame_box.mins.ToVector3(0.f);
	frameGraphBR = frameGraphBL + Vector3(m_my_frame_box.GetDimensions().x, 0.f, 0.f);
	frameGraphTL = frameGraphBL + Vector3(0.f, m_my_frame_box.GetDimensions().y, 0.f);
	frameGraphTR = m_my_frame_box.maxs.ToVector3(0.f);
	m_my_frame_mesh = Mesh::CreateQuadImmediate(VERT_PCU, frameGraphBL, frameGraphBR,
		frameGraphTL, frameGraphTR, frameGraphTint);

	frameGraphBL = m_physx_frame_box.mins.ToVector3(0.f);
	frameGraphBR = frameGraphBL + Vector3(m_physx_frame_box.GetDimensions().x, 0.f, 0.f);
	frameGraphTL = frameGraphBL + Vector3(0.f, m_physx_frame_box.GetDimensions().y, 0.f);
	frameGraphTR = m_physx_frame_box.maxs.ToVector3(0.f);
	m_physx_frame_mesh = Mesh::CreateQuadImmediate(VERT_PCU, frameGraphBL, frameGraphBR,
		frameGraphTL, frameGraphTR, frameGraphTint);

	std::string my_title = Stringf("Inc(%%)");
	my_title.insert(my_title.begin(), 45 - my_title.size(), ' ');
	my_title = "Name" + my_title;
	Vector2 my_title_min = my_frameGraphMin - Vector2(0.f, 2 * TEXT_HEIGHT);
	m_my_title = MakeTextMesh(TEXT_HEIGHT, my_title, my_title_min);

	Vector2 physx_title_min = physx_frameGraphMin - Vector2(0.f, 2 * TEXT_HEIGHT);
	m_physx_title = MakeTextMesh(TEXT_HEIGHT, my_title, physx_title_min);

	float aspect = width / height;

	if (m_profilerCamera == nullptr)
	{
		m_profilerCamera = new Camera();
		m_profilerCamera->SetColorTarget(renderer->GetDefaultColorTarget());
		m_profilerCamera->SetDepthStencilTarget(renderer->GetDefaultDepthTarget());
		m_profilerCamera->SetAspect(aspect);

		// ortho projection
		m_profilerCamera->SetProjectionOrtho(width, height, 0.f, 100.f);
	}

	m_frame_text_tl = Vector2(-width / 2.f, height / 2.f);
	m_profiled_text_tl = my_title_min;
	m_profiled_text_tl_physx = physx_title_min;

	g_my_tree = new ProfilerTree();
	g_phys_tree = new ProfilerTree();
}

Profiler::~Profiler()
{
	delete m_profilerCamera;
	m_profilerCamera = nullptr;

	delete m_backgroundMesh;
	m_backgroundMesh = nullptr;

	delete m_frameGraphMesh;
	m_frameGraphMesh = nullptr;

	delete m_my_frame_mesh;
	m_my_frame_mesh = nullptr;

	delete m_physx_frame_mesh;
	m_physx_frame_mesh = nullptr;

	delete m_my_title;
	m_my_title = nullptr;

	delete m_physx_title;
	m_physx_title = nullptr;

	delete g_my_tree;
	g_my_tree = nullptr;

	delete g_phys_tree;
	g_phys_tree = nullptr;
}

void Profiler::ProfileMarkFrame()
{
	g_start_frame_hpc = GetPerformanceCounter();
}

void Profiler::ProfileMarkEndFrame()
{
	// at frame end, all trees should only have root
	g_my_tree->Trim();
	g_phys_tree->Trim();

	g_end_frame_hpc = GetPerformanceCounter();
	g_last_frame_hpc = g_end_frame_hpc - g_start_frame_hpc;
	g_total_hpc += g_last_frame_hpc;
}

void Profiler::ProfilePause()
{
	m_paused = true;
}

void Profiler::ProfileResume()
{
	m_paused = false;
}

void Profiler::ProfileMyMarkFrame()
{
	g_my_start_hpc = GetPerformanceCounter();
}

void Profiler::ProfileMyMarkEndFrame()
{
	g_my_end_hpc = GetPerformanceCounter();
	g_my_last_hpc = g_my_end_hpc - g_my_start_hpc;
	g_my_total_hpc += g_my_last_hpc;
}

void Profiler::ProfilePhysXMarkFrame()
{
	g_physx_start_hpc = GetPerformanceCounter();
}

void Profiler::ProfilePhysXMarkEndFrame()
{
	g_physx_end_hpc = GetPerformanceCounter();
	g_physx_last_hpc = g_physx_end_hpc - g_physx_start_hpc;
	g_physx_total_hpc += g_physx_last_hpc;
}

void Profiler::Update()
{
	if (m_on)
	{
		UpdateInput();

		if (!m_paused)
		{
			UpdateFrameText();
			UpdateFramePoints();
		}
	}
}

void Profiler::UpdateInput()
{
	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_U))
	{
		if (!m_paused)
		{
			ProfilePause();
		}
		else
		{
			ProfileResume();
		}
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_M))
	{
		if (input->m_mouseLock)
		{
			input->MouseShowCursor(true);
			input->MouseLockCursor(false);

			input->m_mouseLock = false;
		}
		else
		{
			input->MouseShowCursor(false);
			input->MouseLockCursor(true);

			input->m_mouseLock = true;
		}
	}
}

void Profiler::UpdateFrameText()
{
	DeleteVector(m_frame_text_mesh);
	DeleteVector(m_profiled_mesh);
	DeleteVector(m_profiled_mesh_physx);

	// data you wish to update
	double last_frame_seconds = PerformanceCountToSeconds(g_last_frame_hpc);
	float fps = 1.f / last_frame_seconds;

	std::string text = Stringf("Last Frame Time (ms): %f\nFPS: %.4f", 
		last_frame_seconds * 1000.f, fps);
	
	m_frame_text_mesh = MakeTextMeshLines(TEXT_HEIGHT, text, m_frame_text_tl);

	text = g_my_tree->ProduceText();
	m_profiled_mesh = MakeTextMeshLines(.7f * TEXT_HEIGHT, text, m_profiled_text_tl);

	text = g_phys_tree->ProduceText();
	m_profiled_mesh_physx = MakeTextMeshLines(.7f * TEXT_HEIGHT, text, m_profiled_text_tl_physx);
}

void Profiler::UpdateFramePoints()
{
	// update largest
	if (g_last_frame_hpc > g_max_hpc)
		g_max_hpc = g_last_frame_hpc;

	float max_ms = PerformanceCountToMilliseconds(g_max_hpc);
	float last_frame_ms = PerformanceCountToMilliseconds(g_last_frame_hpc);
	
	// there are 255 segments 
	float pointSpeed = PROFILER_WIDTH / 255.f;

	// if last frame is valid, show it
	if (last_frame_ms > 0.f)
	{
		float percentage = last_frame_ms / max_ms;

		float baseHeight = m_frameGraphBox.mins.y;
		float pointHeight = baseHeight + percentage * FRAME_GRAPH_LINE * TEXT_HEIGHT;
		float pointX = m_frameGraphBox.maxs.x + pointSpeed;
		Vector2 pointPos = Vector2(pointX, pointHeight);

		// if reaching max num of points, pop from front
		m_framePoints.push_back(pointPos);
		if (m_framePoints.size() == 256U + 1U)
		{
			m_framePoints.pop_front();
		}
	}

	// after adding, update point queue - translate them to left
	for (std::deque<Vector2>::iterator it = m_framePoints.begin();
		it != m_framePoints.end(); ++it)
	{
		float x = it->x;
		x -= pointSpeed;
		it->x = x;
	}

	// my api
	if (g_my_last_hpc > g_my_max_hpc)
		g_my_max_hpc = g_my_last_hpc;

	max_ms = PerformanceCountToMilliseconds(g_my_max_hpc);
	last_frame_ms = PerformanceCountToMilliseconds(g_my_last_hpc);

	pointSpeed = SUB_PROFILER_WIDTH / 127.f;

	if (last_frame_ms > 0.f)
	{
		float percentage = last_frame_ms / max_ms;

		float baseHeight = m_my_frame_box.mins.y;
		float pointHeight = baseHeight + percentage * FRAME_GRAPH_LINE * TEXT_HEIGHT;
		float pointX = m_my_frame_box.maxs.x + pointSpeed;
		Vector2 pointPos = Vector2(pointX, pointHeight);

		m_my_frame_pts.push_back(pointPos);
		if (m_my_frame_pts.size() == 128U + 1U)
		{
			m_my_frame_pts.pop_front();
		}
	}

	for (std::deque<Vector2>::iterator it = m_my_frame_pts.begin();
		it != m_my_frame_pts.end(); ++it)
	{
		float x = it->x;
		x -= pointSpeed;
		it->x = x;
	}

	// physx api
	if (g_physx_last_hpc > g_physx_max_hpc)
		g_physx_max_hpc = g_physx_last_hpc;

	max_ms = PerformanceCountToMilliseconds(g_physx_max_hpc);
	last_frame_ms = PerformanceCountToMilliseconds(g_physx_last_hpc);

	if (last_frame_ms > 0.f)
	{
		float percentage = last_frame_ms / max_ms;

		float baseHeight = m_physx_frame_box.mins.y;
		float pointHeight = baseHeight + percentage * FRAME_GRAPH_LINE * TEXT_HEIGHT;
		float pointX = m_physx_frame_box.maxs.x + pointSpeed;
		Vector2 pointPos = Vector2(pointX, pointHeight);

		m_physx_frame_pts.push_back(pointPos);
		if (m_physx_frame_pts.size() == 128U + 1U)
		{
			m_physx_frame_pts.pop_front();
		}
	}

	for (std::deque<Vector2>::iterator it = m_physx_frame_pts.begin();
		it != m_physx_frame_pts.end(); ++it)
	{
		float x = it->x;
		x -= pointSpeed;
		it->x = x;
	}
}

// single line 
Mesh* Profiler::MakeTextMesh(float textHeight, std::string text, Vector2 drawmin)
{
	Rgba frameTextTint = Rgba::WHITE;
	Renderer* renderer = Renderer::GetInstance();
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
	Mesh* mesh = Mesh::CreateTextImmediate(frameTextTint,
		drawmin, font, textHeight,
		0.7f, text, VERT_PCU);
	return mesh;
}

// multiple lines
std::vector<Mesh*> Profiler::MakeTextMeshLines(float textHeight, std::string text, Vector2 draw_tl)
{
	std::vector<Mesh*> res;
	Vector2 bl = draw_tl - VERT_DISP;

	Renderer* rdr = Renderer::GetInstance();
	BitmapFont* font = rdr->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	while (!text.empty())
	{
		std::string prev;
		std::string latter;
		StringSplitTwo(text, "\n", prev, latter);
		text = latter;
	
		Mesh* mesh = Mesh::CreateTextImmediate(Rgba::WHITE, bl, font, textHeight, .7f, prev, VERT_PCU);
		res.push_back(mesh);

		bl -= VERT_DISP;
	}

	return res;
}

void Profiler::Render(Renderer* renderer)
{
	if (m_on)
	{
		renderer->SetCamera(m_profilerCamera);

		RenderFrameText(renderer);
		RenderFramePoints(renderer);

		// bgs
		DrawGraphAlpha(renderer, m_frameGraphMesh);
		DrawGraphAlpha(renderer, m_my_frame_mesh);
		DrawGraphAlpha(renderer, m_physx_frame_mesh);
		DrawGraphAlpha(renderer, m_backgroundMesh);
	}
}

void Profiler::RenderFrameText(Renderer* renderer)
{
	DrawTexts(m_frame_text_mesh);
	DrawTexts(m_profiled_mesh);
	DrawTexts(m_profiled_mesh_physx);
	DrawTextCut(m_my_title);
	DrawTextCut(m_physx_title);
}

void Profiler::RenderFramePoints(Renderer* renderer)
{
	for (std::deque<Vector2>::iterator it = m_framePoints.begin();
		it != m_framePoints.end(); ++it)
	{
		std::deque<Vector2>::iterator nextIt = std::next(it, 1);
		if (nextIt != m_framePoints.end())
		{
			Vector2 bl = Vector2(it->x, m_frameGraphBox.mins.y);
			Vector2 br = Vector2(nextIt->x, m_frameGraphBox.mins.y);
			Vector2 tl = Vector2(it->x, it->y);
			Vector2 tr = Vector2(nextIt->x, nextIt->y);

			renderer->DrawPolygon2D(bl, br, tl, tr, Rgba::YELLOW, VERT_PCU);
		}
	}

	for (std::deque<Vector2>::iterator it = m_my_frame_pts.begin();
		it != m_my_frame_pts.end(); ++it)
	{
		std::deque<Vector2>::iterator nextIt = std::next(it, 1);
		if (nextIt != m_my_frame_pts.end())
		{
			Vector2 bl = Vector2(it->x, m_my_frame_box.mins.y);
			Vector2 br = Vector2(nextIt->x, m_my_frame_box.mins.y);
			Vector2 tl = Vector2(it->x, it->y);
			Vector2 tr = Vector2(nextIt->x, nextIt->y);

			renderer->DrawPolygon2D(bl, br, tl, tr, Rgba::GREEN, VERT_PCU);
		}
	}

	for (std::deque<Vector2>::iterator it = m_physx_frame_pts.begin();
		it != m_physx_frame_pts.end(); ++it)
	{
		std::deque<Vector2>::iterator nextIt = std::next(it, 1);
		if (nextIt != m_physx_frame_pts.end())
		{
			Vector2 bl = Vector2(it->x, m_physx_frame_box.mins.y);
			Vector2 br = Vector2(nextIt->x, m_physx_frame_box.mins.y);
			Vector2 tl = Vector2(it->x, it->y);
			Vector2 tr = Vector2(nextIt->x, nextIt->y);

			renderer->DrawPolygon2D(bl, br, tl, tr, Rgba::RED, VERT_PCU);
		}
	}
}

void Profiler::DrawGraphAlpha(Renderer* renderer, Mesh* mesh)
{
	if (mesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("2d_direct_alpha");
		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		renderer->DrawMesh(mesh);
	}
}

void Profiler::DrawCutoutText(Renderer* renderer, Mesh* mesh)
{
	if (mesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		renderer->DrawMesh(mesh);
	}
}

Profiler* Profiler::GetInstance()
{
	if (m_profilerInstance == nullptr)
	{
		m_profilerInstance = new Profiler();
	}

	return m_profilerInstance;
}

void Profiler::DestroyInstance()
{
	delete m_profilerInstance;
	m_profilerInstance = nullptr;
}

void ProfilerTree::Initialize()
{
	m_root = new ProfilerNode("root", nullptr, 0.f);
	m_current = m_root;
}

void ProfilerTree::Destroy()
{
	delete m_root;
	m_root = nullptr;
}

void ProfilerTree::AddNode(ProfilerNode* n)
{
	// adding node means to add it as child of current node of tree
	m_current->AddChild(n);
}

void ProfilerTree::Trim()
{
	// trim all nodes except for root
	m_root->DeleteChildren();
	
	// reset current node
	m_current = m_root;
}

std::string ProfilerTree::ProduceText()
{
	// produce profiled data in string...
	// in format of: name | inc percentage
	std::string res;
	m_root->ProduceText(res, g_last_frame_hpc);		// make sure this tree is also from last frame
	return res;
}

ProfilerTree::ProfilerTree()
{
	Initialize();
}

ProfilerTree::~ProfilerTree()
{
	Destroy();
}
