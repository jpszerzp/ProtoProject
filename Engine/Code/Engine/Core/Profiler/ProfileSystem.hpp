//#pragma once
//#include "Engine/Renderer/Renderer.hpp"
//#include "Engine/Renderer/Mesh.hpp"
//#include "Engine/Core/Profiler/ProfilerNode.hpp"
//
//#include <vector>
//#include <deque>
//
//// states query
//bool IsProfilerOn();
//bool IsProfilerPaused();			// profiler needs to be on first
//
//
//class Profiler
//{
//public:
//	Profiler();
//	~Profiler();
//
//	void ProfileMarkFrame();
//	void ProfileMarkEndFrame();
//	void ProfilePause();
//	void ProfileResume();
//	//static ProfilerHistory& ProfileGetPreviousFrame(uint skipCount);
//
//	void ConfigureSections();
//	void ConfigureMeshes();
//	void ConfigureTextMap();
//
//	void Update();
//	void UpdateInput();
//	void UpdateFrameText();
//	void UpdateTreeText();
//	void UpdateFramePoints();
//
//	Mesh* MakeTextMesh(float textHeight, std::string text, Vector2 drawmin);
//
//	void Render (Renderer* renderer);
//	void RenderFrameText(Renderer* renderer);
//	void RenderTreeText(Renderer* renderer);
//	void RenderFramePoints(Renderer* renderer);
//	void DrawGraphAlpha(Renderer* renderer, Mesh* mesh);
//	void DrawCutoutText(Renderer* renderer, Mesh* mesh);
//
//	static Profiler* GetInstance();
//	static void		 DestroyInstance();
//
//	void ClearTreeMesh();
//	//void ClearMeshMap(std::map<ProfilerNode*, Mesh*>& map);
//
//public:
//	static Profiler* m_profilerInstance;
//
//	static constexpr float FRAME_GRAPH_WIDTH = 896.f;			// 896 / 128 = 7
//	static constexpr float FRAME_GRAPH_HEIGHT = 128.f;
//	static constexpr float FRAME_TEXT_WIDTH = 512.f;
//	static constexpr float FRAME_TEXT_HEIGHT = 128.f;
//	static constexpr float FRAME_PROPERTY_WIDTH = 896.f;
//	static constexpr float FRAME_PROPERTY_HEIGHT = 32.f;
//
//	AABB2 m_profilerBox;			// overall profiler bound
//	AABB2 m_memoryTextBox;			// bound for memory text
//	AABB2 m_memoryGraphBox;			// bound for memory graph
//	AABB2 m_frameTextBox;			// bound for frame text
//	AABB2 m_frameGraphBox;			// bound for frame graph
//	AABB2 m_frameTreeBox;			// bound for frame tree
//	AABB2 m_framePropertyBox;		// bound for frame property titles
//
//	Mesh* m_backgroundMesh = nullptr;
//	Mesh* m_memoryGraphMesh = nullptr;
//	Mesh* m_frameGraphMesh = nullptr;
//	Mesh* m_framePropertyMesh = nullptr;
//
//	Camera* m_profilerCamera = nullptr;
//
//	// vector of text mesh...
//	std::map<ProfilerNode*, Mesh*> m_treeMeshes;
//	std::map<std::string, Mesh*> m_frameTextMeshes;
//	//std::vector<Mesh*> m_memoryTextMeshes;
//
//	// points of last 128 frames
//	std::deque<Vector2> m_framePoints;
//
//	//bool m_testPoint = false;
//	bool m_on = false;
//	bool m_paused = false;
//	bool m_treeView = true;
//	bool m_totalSort = true;
//};