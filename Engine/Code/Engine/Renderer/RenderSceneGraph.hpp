#pragma once

#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Core/GameObject.hpp"

class RenderSceneGraph
{
public:
	RenderSceneGraph() {}
	~RenderSceneGraph() {}

	void AddRenderable(GameObject* object);
	void AddLight(Light* light);
	void AddCamera(Camera* camera);

	void RemoveRenderables();
	void RemoveLights();
	void RemoveCameras();
	void RemoveCameras2D();

	void SortCamera();

public:
	std::vector<Renderable*> m_renderables;
	std::vector<Light*> m_lights;

	std::vector<Camera*> m_cameras;	
	Camera* m_camera2D = nullptr;
};