#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/Profiler/ProfilerNode.hpp"

#include <vector>
#include <deque>

// states query
bool IsProfilerOn();
bool IsProfilerPaused();			// profiler needs to be on first

class ProfilerTree
{
	ProfilerNode* m_current;
	ProfilerNode* m_root;

public:
	void Initialize();
	void Destroy();

	void AddNode(ProfilerNode* n);
	void Trim();
	std::string ProduceText();

	ProfilerTree();
	~ProfilerTree();

	ProfilerNode* GetCurrent() const { return m_current; }

	void SetCurrent(ProfilerNode* value) { m_current = value; }
};

class Profiler
{
public:
	Profiler();
	~Profiler();

	void ProfileMarkFrame();
	void ProfileMarkEndFrame();
	void ProfilePause();
	void ProfileResume();

	// todo: this make the profiler more like game code...
	void ProfileMyMarkFrame();
	void ProfileMyMarkEndFrame();
	void ProfilePhysXMarkFrame();
	void ProfilePhysXMarkEndFrame();

	void Update();
	void UpdateInput();
	void UpdateFrameText();
	void UpdateFramePoints();

	Mesh* MakeTextMesh(float textHeight, std::string text, Vector2 drawmin);
	std::vector<Mesh*> MakeTextMeshLines(float textHeight, std::string text, Vector2 draw_tl);

	void Render (Renderer* renderer);
	void RenderFrameText(Renderer* renderer);
	void RenderFramePoints(Renderer* renderer);

	void DrawGraphAlpha(Renderer* renderer, Mesh* mesh);
	void DrawCutoutText(Renderer* renderer, Mesh* mesh);

	static Profiler* GetInstance();
	static void		 DestroyInstance();

public:
	static Profiler* m_profilerInstance;

	AABB2 m_profilerBox;			// overall profiler bound
	AABB2 m_frameGraphBox;			// bound for frame graph
	AABB2 m_my_frame_box;
	AABB2 m_physx_frame_box;
	//AABB2 m_my_title_box;
	//AABB2 m_physx_title_box;

	Mesh* m_backgroundMesh = nullptr;
	Mesh* m_frameGraphMesh = nullptr;
	Mesh* m_my_frame_mesh = nullptr;
	Mesh* m_physx_frame_mesh = nullptr;
	Mesh* m_my_title = nullptr;
	Mesh* m_physx_title = nullptr;

	Camera* m_profilerCamera = nullptr;

	// points of last 128 frames
	std::deque<Vector2> m_framePoints;
	std::deque<Vector2> m_my_frame_pts;
	std::deque<Vector2> m_physx_frame_pts;

	bool m_on = false;
	bool m_paused = false;

	constexpr static float TEXT_HEIGHT = 20.f;
	const static Vector2 VERT_DISP;

	Vector2 m_frame_text_tl;
	std::vector<Mesh*> m_frame_text_mesh;
	
	Vector2 m_profiled_text_tl;
	std::vector<Mesh*> m_profiled_mesh;

	Vector2 m_profiled_text_tl_physx;
	std::vector<Mesh*> m_profiled_mesh_physx;

	static const int FRAME_TEXT_LINE = 2;
	static const int FRAME_GRAPH_LINE = 5;
	static const int FRAME_GRAPH_MARGIN = 20;
	float PROFILER_HEIGHT;
	float PROFILER_WIDTH;
	float SUB_PROFILER_WIDTH;
	
	//ProfilerTree* m_my_api_tree = nullptr;
	//ProfilerTree* m_physx_tree = nullptr;
};