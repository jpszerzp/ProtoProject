#include "Engine/Renderer/RenderSceneGraph.hpp"
#include "Engine/Core/Light/Light.hpp"


void RenderSceneGraph::AddCamera(Camera* camera)
{
	m_cameras.push_back(camera);
}

void RenderSceneGraph::AddLight(Light* light)
{
	m_lights.push_back(light);
}

void RenderSceneGraph::AddRenderable(GameObject* object)
{
	object->m_isInForwardPath = true;
	m_renderables.push_back(object->m_renderable);
}

void RenderSceneGraph::RemoveRenderables()
{
	m_renderables.clear();
}

void RenderSceneGraph::RemoveLights()
{
	m_lights.clear();
}

void RenderSceneGraph::RemoveCameras()
{
	m_cameras.clear();
}

void RenderSceneGraph::RemoveCameras2D()
{
	m_camera2D = nullptr;
}

void RenderSceneGraph::SortCamera()
{
}

// deprecated
/*
bool RenderSceneGraph::IsLightAppropriate(Light* light, Vector3 renderablePos)
{
	Vector3 displacement = light->m_renderable->GetLocalPosition() - renderablePos;

	if (displacement.GetLength() < 50.f)
	{
		return true;
	}
	
	return false;
}
*/