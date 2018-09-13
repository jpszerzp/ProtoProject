#pragma once

#include "Engine/Renderer/RenderSceneGraph.hpp"

class ForwardRenderPath
{
public:
	void RenderScene(RenderSceneGraph* scene);
	void RenderSceneForCamera(Camera* camera, RenderSceneGraph* scene);
	//void RenderShadowCastForLight(Light* light, RenderSceneGraph* scene);

	void RenderDebug3DObjectsForCamera(Camera* camera);
	void RenderDebug2DObjects(Camera* camera);

	void ClearBasedOnCameraOptions(Camera* camera);
	void ComputeMostContributingLights(uint& lightCount, int* lightIndices, Vector3 renderablePos, RenderSceneGraph* scene);

	std::vector<Drawcall*> SortDrawcalls(std::vector<Drawcall*>& drawcalls);

public:
	Renderer* m_renderer;

	static const int LAYER_START = 0;
};