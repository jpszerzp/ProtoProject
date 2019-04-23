#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Input/InputSystem.hpp"

#include <algorithm>

uint64_t g_start_frame_hpc = 0U;
uint64_t g_end_frame_hpc = 0U;
uint64_t g_last_frame_hpc = 0U;
uint64_t g_total_hpc = 0U; 
//uint64_t g_max_hpc = 0LLU;	// sets to max uint64_t
//uint64_t g_min_hpc = ~0LLU; 

Profiler* Profiler::m_profilerInstance = nullptr;

bool TotalTimeSort(ProfilerNode*, ProfilerNode*)
{
	/*
	ProfilerHistory& history = Profiler::ProfileGetPreviousFrame(0);
	Vector2 t1 = history.InspectNodeTime(n1);
	Vector2 t2 = history.InspectNodeTime(n2);
	//Vector2 t1 = g_gameConfigBlackboard->m_history.InspectNodeTime(n1);
	//Vector2 t2 = g_gameConfigBlackboard->m_history.InspectNodeTime(n2);

	float total1 = t1.x;
	float total2 = t2.x;

	return total1 > total2;
	*/
	return false;
}

bool SelfTimeSort(ProfilerNode*, ProfilerNode*)
{
	/*
	ProfilerHistory& history = Profiler::ProfileGetPreviousFrame(0);
	Vector2 t1 = history.InspectNodeTime(n1);
	Vector2 t2 = history.InspectNodeTime(n2);

	float self1 = t1.y;
	float self2 = t2.y;

	return self1 > self2;
	*/
	return false;
}

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
	ConfigureSections();
	ConfigureMeshes();
	ConfigureTextMap();

	Renderer* renderer = Renderer::GetInstance();

	float width = m_profilerBox.GetDimensions().x;
	float height = m_profilerBox.GetDimensions().y;
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
}

Profiler::~Profiler()
{
	delete m_profilerCamera;
	m_profilerCamera = nullptr;

	delete m_backgroundMesh;
	m_backgroundMesh = nullptr;

	delete m_frameGraphMesh;
	m_frameGraphMesh = nullptr;
}

void Profiler::ProfileMarkFrame()
{
	g_start_frame_hpc = GetPerformanceCounter();
	//uint64_t this_hpc = GetPerformanceCounter();
	//uint64_t last_hpc = this_hpc - g_total_hpc;
	//g_last_frame_time = (float)PerformanceCountToSeconds(last_hpc) * 1000.f;	// in ms
	//g_total_hpc += last_hpc;

	//g_gameConfigBlackboard->MarkEndFrame();

	//// clear map
	//g_gameConfigBlackboard->ClearMap();

	//// set blackboard indices
	//g_gameConfigBlackboard->ClearIndex();

	//// unregister tree (or an array)
	//g_gameConfigBlackboard->ClearTreeArray();
}

void Profiler::ProfileMarkEndFrame()
{
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

/*
ProfilerHistory& Profiler::ProfileGetPreviousFrame(uint skipCount)
{
	//return g_gameConfigBlackboard->m_history[skipCount];
}
*/

void Profiler::ConfigureSections()
{
	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	Vector2 min = Vector2(-width / 2.f, -height / 2.f);
	Vector2 max = Vector2(width / 2.f, height / 2.f);
	m_profilerBox = AABB2(min, max);

	Vector2 frameGraphMin = Vector2(-width / 4.f, height / 4.f) + Vector2(128.f, -128.f);
	Vector2 frameGraphMax = frameGraphMin + Vector2(FRAME_GRAPH_WIDTH, FRAME_GRAPH_HEIGHT);
	m_frameGraphBox = AABB2(frameGraphMin, frameGraphMax);

	Vector2 frameTextMin = Vector2(-width / 2.f, height / 4.f) + Vector2(32.f, -128.f);
	Vector2 frameTextMax = frameTextMin + Vector2(FRAME_TEXT_WIDTH, FRAME_TEXT_HEIGHT);
	m_frameTextBox = AABB2(frameTextMin, frameTextMax);

	Vector2 treeMin = Vector2(-width / 2.f, -height / 2.f) + Vector2(32.f, 32.f);
	Vector2 treeMax = Vector2(width / 2.f, height / 4.f) + Vector2(-32.f, -32.f);
	m_frameTreeBox = AABB2(treeMin, treeMax);

	Vector2 propertyMin = Vector2(-width / 4.f, height / 4.f) + Vector2(128.f, -160.f);
	Vector2 propertyMax = propertyMin + Vector2(FRAME_PROPERTY_WIDTH, FRAME_PROPERTY_HEIGHT);
	m_framePropertyBox = AABB2(propertyMin, propertyMax);
}

void Profiler::ConfigureMeshes()
{
	Vector3 bl = m_profilerBox.mins.ToVector3(0.f);
	Vector3 br = bl + Vector3(m_profilerBox.GetDimensions().x, 0.f, 0.f);
	Vector3 tl = bl + Vector3(0.f, m_profilerBox.GetDimensions().y, 0.f);
	Vector3 tr = m_profilerBox.maxs.ToVector3(0.f);
	Rgba tint = Rgba::BLUE_HALF_OPACITY;
	m_backgroundMesh = Mesh::CreateQuadImmediate(VERT_PCU, bl, br, tl, tr, tint);

	Vector3 frameGraphBL = m_frameGraphBox.mins.ToVector3(0.f);
	Vector3 frameGraphBR = frameGraphBL + Vector3(FRAME_GRAPH_WIDTH, 0.f, 0.f);
	Vector3 frameGraphTL = frameGraphBL + Vector3(0.f, FRAME_GRAPH_HEIGHT, 0.f);
	Vector3 frameGraphTR = m_frameGraphBox.maxs.ToVector3(0.f);
	Rgba frameGraphTint = Rgba::WHITE_HALF_OPACITY;
	m_frameGraphMesh = Mesh::CreateQuadImmediate(VERT_PCU, frameGraphBL, frameGraphBR,
		frameGraphTL, frameGraphTR, frameGraphTint);

	m_framePropertyMesh = MakeTextMesh(16.f, "______Calls______%Total______(Time)______%Self______(Time)", m_framePropertyBox.mins);
}

void Profiler::ConfigureTextMap()
{
	m_frameTextMeshes.emplace("frame_time", nullptr);
	//m_frameTextMeshes.emplace("sample", nullptr);
	m_frameTextMeshes.emplace("fps", nullptr);
}

void Profiler::Update()
{
	if (m_on)
	{
		UpdateInput();
		if (!m_paused)
		{
			UpdateFrameText();
			//UpdateTreeText();
			//UpdateFramePoints();
		}
	}
}

void Profiler::UpdateInput()
{
	InputSystem* input = InputSystem::GetInstance();

	/*
	// process input
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_V))
	{
		m_treeView = !m_treeView;
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_L))
	{
		// toggle sort mode
		m_totalSort = !m_totalSort;
	}
	*/

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
	int count = 0;
	for (std::map<std::string, Mesh*>::iterator it = m_frameTextMeshes.begin();
		it != m_frameTextMeshes.end(); ++it)
	{
		std::string purpose = it->first;
		Mesh* mesh = it->second;

		if (purpose == "frame_time")
		{
			if (mesh != nullptr)
			{
				delete mesh;
				mesh = nullptr;
			}

			Vector2 frameTextBL = m_frameTextBox.mins + Vector2(0.f, 20.f * count);

			std::string text;
			//float lastFrameTime = g_gameConfigBlackboard->m_lastFrameTime;
			if (g_last_frame_hpc == -INFINITY)
			{
				// first frame has not finished, print N/A
				text = "Last Frame Time: N/A";
			}

			double seconds = PerformanceCountToSeconds(g_last_frame_hpc);
			text = Stringf("Last Frame Time (ms): %f", seconds * 1000.f);

			m_frameTextMeshes[purpose] = MakeTextMesh(16.f, text, frameTextBL);
		}

		//else if (purpose == "sample")
		//{
		//	if (mesh != nullptr)
		//	{
		//		delete mesh;
		//		mesh = nullptr;
		//	}

		//	Vector2 frameTextBL = m_frameTextBox.mins + Vector2(0.f, 20.f * count);
		//	std::string text = Stringf("Samples Number: %i", 0);
		//	m_frameTextMeshes[purpose] = MakeTextMesh(16.f, text, frameTextBL);
		//}

		else if (purpose == "fps")
		{
			if (mesh != nullptr)
			{
				delete mesh;
				mesh = nullptr;
			}
			
			Vector2 frameTextBL = m_frameTextBox.mins + Vector2(0.f, 20.f * count);
			//float lastFrameTime = g_gameConfigBlackboard->m_lastFrameTime * 0.001f;			// in s
			//float lastFrameTime = g_last_frame_hpc * 0.001f;			// in s
			double seconds = PerformanceCountToSeconds(g_last_frame_hpc);
			float fps = 1.f / (float)(seconds);
			std::string text = Stringf("FPS: %.4f", fps);
			m_frameTextMeshes[purpose] = MakeTextMesh(16.f, text, frameTextBL);
		}

		count++;
	}
}

void Profiler::UpdateTreeText()
{
	/*
	std::stack<ProfilerNode*> traversal;
	traversal.push(g_gameConfigBlackboard->m_profiledFunctionTree[0]);

	ProfilerNode* currentNode;
	int nodeProcessed = 0;
	while (!traversal.empty())
	{
		currentNode = traversal.top();
		traversal.pop();
		nodeProcessed += 1;

		bool makeText = true;
		for (std::map<ProfilerNode*, Mesh*>::iterator it = m_treeMeshes.begin(); 
			it != m_treeMeshes.end(); ++it)
		{
			ProfilerNode* node = it->first;
			if (node->m_tag == currentNode->m_tag)
			{
				if (node->m_parent == currentNode->m_parent)
				{
					nodeProcessed -= 1;
					makeText = false;
					break;
				}
			}
		}

		if (makeText)
		{
			int callCount = 0;
			// in the map, count all nodes with same tag and parent
			for (std::map<ProfilerNode*, UInt64Vector2>::iterator it =
				g_gameConfigBlackboard->m_functionHpcInfoPairs.begin(); 
				it != g_gameConfigBlackboard->m_functionHpcInfoPairs.end(); ++it)
			{
				ProfilerNode* node = it->first;

				if (currentNode != node)
				{
					if (node->m_tag == currentNode->m_tag
						&& node->m_parent == currentNode->m_parent)
					{
						callCount++;
					}
				}
				else
				{
					callCount++;
				}
			}
			ProfilerHistory& history = ProfileGetPreviousFrame(0);
			Vector2 theTime = history.InspectNodeTime(currentNode);
			//Vector2 theTime = g_gameConfigBlackboard->m_history.InspectNodeTime(currentNode);
			float totalTime = theTime.x;
			float selfTime = theTime.y;
			float tpercentage = (totalTime / g_gameConfigBlackboard->m_lastFrameTime) * 100.f;
			float spercentage = (selfTime / g_gameConfigBlackboard->m_lastFrameTime) * 100.f;

			std::string tag = currentNode->m_tag;
			std::string callFormat = Stringf("______%i", callCount);
			std::string totalPercentage = Stringf("______%f", tpercentage);
			std::string total = Stringf("______%f", totalTime);
			std::string selfPercentage = Stringf("______%f", spercentage);
			std::string self = Stringf("______%f", selfTime);
			std::string text = tag + callFormat + totalPercentage + total + selfPercentage + self;				// all info in one text

			Vector2 textBL;
			if (m_treeView)
			{
				textBL = m_frameTreeBox.mins + Vector2(32.f * currentNode->m_layer, 20.f * (nodeProcessed - 1));
			}
			else
			{
				textBL = m_frameTreeBox.mins + Vector2(0.f, 20.f * (nodeProcessed - 1));
			}
			Mesh* mesh = MakeTextMesh(16.f, text, textBL);
			m_treeMeshes.emplace(currentNode, mesh);
		}

		for (int childIdx = (int)(currentNode->m_childNodes.size() - 1U); childIdx >= 0; --childIdx)
		{
			ProfilerNode* childNode = currentNode->m_childNodes[childIdx];
			childNode->m_layer = currentNode->m_layer + 1;					// dfs layer

			if (childNode != nullptr)
			{
				traversal.push(childNode);
			}
		}
	}
	*/
}

void Profiler::UpdateFramePoints()
{
	/*
	// store largest frame time so far
	if (g_gameConfigBlackboard->m_lastFrameTime > g_gameConfigBlackboard->m_largestTime)
	{
		g_gameConfigBlackboard->m_largestTime = g_gameConfigBlackboard->m_lastFrameTime;
	}

	float pointSpeed = FRAME_GRAPH_WIDTH / 127.f;			// 128 frame points at most

															// every frame add a new point
	if (g_gameConfigBlackboard->m_lastFrameTime != -INFINITY)
	{
		//m_testPoint = true;

		float value = g_gameConfigBlackboard->m_lastFrameTime;
		float percentage = value / g_gameConfigBlackboard->m_largestTime;

		float baseHeight = m_frameGraphBox.mins.y;
		float pointHeight = baseHeight + percentage * FRAME_GRAPH_HEIGHT;
		float pointX = m_frameGraphBox.maxs.x + pointSpeed;
		Vector2 pointPos = Vector2(pointX, pointHeight);

		m_framePoints.push_back(pointPos);
		if (m_framePoints.size() == 128U + 1U)
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
	*/
}

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

void Profiler::Render(Renderer* renderer)
{
	if (m_on)
	{
		renderer->SetCamera(m_profilerCamera);

		RenderFrameText(renderer);
		//RenderTreeText(renderer);
		//RenderFramePoints(renderer);

		//DrawCutoutText(renderer, m_framePropertyMesh);
		//DrawGraphAlpha(renderer, m_frameGraphMesh);
		DrawGraphAlpha(renderer, m_backgroundMesh);
	}
}

void Profiler::RenderFrameText(Renderer* renderer)
{
	for (std::map<std::string, Mesh*>::iterator it = m_frameTextMeshes.begin();
		it != m_frameTextMeshes.end(); ++it)
	{
		Mesh* mesh = it->second;
		DrawCutoutText(renderer, mesh);
	}
}

void Profiler::RenderTreeText(Renderer* renderer)
{
	for (std::map<ProfilerNode*, Mesh*>::iterator it = m_treeMeshes.begin();
		it != m_treeMeshes.end(); ++it)
	{
		Mesh* mesh = it->second;
		DrawCutoutText(renderer, mesh);
	}
	ClearTreeMesh();

	/*
	if (m_treeView)
	{
		for (std::map<ProfilerNode*, Mesh*>::iterator it = m_treeMeshes.begin();
			it != m_treeMeshes.end(); ++it)
		{
			Mesh* mesh = it->second;
			DrawCutoutText(renderer, mesh);
		}
		ClearTreeMesh();
	}
	else
	{
		if (m_totalSort)
		{
			std::vector<ProfilerNode*> appendum;
			std::map<ProfilerNode*, Mesh*> sortedMeshes;

			for (std::map<ProfilerNode*, Mesh*>::iterator it = m_treeMeshes.begin();
				it != m_treeMeshes.end(); ++it)
			{
				ProfilerNode* node = it->first;
				appendum.push_back(node);
			}

			std::sort(appendum.begin(), appendum.end(), TotalTimeSort);

			for each (ProfilerNode* node in appendum)
			{
				Mesh* mesh = m_treeMeshes.at(node);
				sortedMeshes.emplace(node, mesh);			// from large to small total time
			}

			for (std::map<ProfilerNode*, Mesh*>::iterator it = sortedMeshes.begin();
				it != sortedMeshes.end(); ++it)
			{
				Mesh* mesh = it->second;
				DrawCutoutText(renderer, mesh);
			}

			ClearTreeMesh();
			appendum.clear();
			sortedMeshes.clear();
		}
		else
		{
			std::vector<ProfilerNode*> appendum;
			std::map<ProfilerNode*, Mesh*> sortedMeshes;

			for (std::map<ProfilerNode*, Mesh*>::iterator it = m_treeMeshes.begin();
				it != m_treeMeshes.end(); ++it)
			{
				ProfilerNode* node = it->first;
				appendum.push_back(node);
			}

			std::sort(appendum.begin(), appendum.end(), SelfTimeSort);

			for each (ProfilerNode* node in appendum)
			{
				Mesh* mesh = m_treeMeshes.at(node);
				sortedMeshes.emplace(node, mesh);			// from large to small total time
			}

			for (std::map<ProfilerNode*, Mesh*>::iterator it = sortedMeshes.begin();
				it != sortedMeshes.end(); ++it)
			{
				Mesh* mesh = it->second;
				DrawCutoutText(renderer, mesh);
			}

			ClearTreeMesh();
			appendum.clear();
			sortedMeshes.clear();
		}
	}
	*/
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

void Profiler::ClearTreeMesh()
{
	for (std::map<ProfilerNode*, Mesh*>::iterator it = m_treeMeshes.begin();
		it != m_treeMeshes.end(); ++it)
	{
		ProfilerNode* treeNode = it->first;
		treeNode = nullptr;

		Mesh* nodeMesh = it->second;
		delete nodeMesh;
		nodeMesh = nullptr;
	}

	m_treeMeshes.clear();
}
