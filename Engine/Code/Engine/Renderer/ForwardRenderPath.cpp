#include "Engine/Renderer/ForwardRenderPath.hpp"
#include "Engine/Renderer/Drawcall.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Light/Light.hpp"

#include <algorithm>
#include <iterator>

bool LayerCompare(Drawcall* dc_1, Drawcall* dc_2)
{
	return (dc_1->GetLayer() < dc_2->GetLayer());
}


bool QueueDepthCompare(Drawcall* dc_1, Drawcall* dc_2)
{
	return (dc_1->GetQueue() < dc_2->GetQueue());
}


bool LightIndexDistanceTupleDepthCompare(Vector2 v1, Vector2 v2)
{
	return (v1.y < v2.y);
}


void ForwardRenderPath::RenderScene(RenderSceneGraph* scene)
{
	// sort camera by material and state
	//scene->SortCamera();

	// generate shadow map
	/*
	for each (Light* light in scene->m_lights)
	{
		if (light->m_shadowCast)
		{
			RenderShadowCastForLight(light, scene);
		}
	}
	*/

	for each(Camera* cam in scene->m_cameras)
		RenderSceneForCamera(cam, scene);

	//Camera* debugCamera2D = GetDebugRender2DCamera();
	RenderDebug2DObjects(scene->m_camera2D);
}


void ForwardRenderPath::RenderSceneForCamera(Camera* camera, RenderSceneGraph* scene)
{
	std::vector<Drawcall*> dcs;

	for each (Renderable* rdb in scene->m_renderables)
	{
		Drawcall* dc = rdb->ComposeDrawcall();

		// only consider the single effective light
		dc->m_light_mat_ambient = scene->m_single_light->m_mat_amb;
		dc->m_light_mat_diff = scene->m_single_light->m_mat_diff;
		dc->m_light_mat_spec = scene->m_single_light->m_mat_spec;
		dc->m_light_pos = scene->m_single_light->GetWorldPosition();
		dc->m_light_dir = scene->m_single_light->GetDirection();
		dc->m_light_atten_const = scene->m_single_light->GetAttenConst();
		dc->m_light_atten_linear = scene->m_single_light->GetAttenLinear();
		dc->m_light_atten_quad = scene->m_single_light->GetAttenQuadratic();
		dc->m_cutoff = scene->m_single_light->GetCutOff();
		dc->m_outerCutoff = scene->m_single_light->GetOuterCutoff();

		dcs.push_back(dc);
	}

	for each (Drawcall* dc in dcs)
		m_renderer->Draw(*dc);

	for each (Drawcall* dc in dcs)
	{
		delete dc;
		dc = nullptr;
	}
	dcs.clear();

	/*
	//ClearBasedOnCameraOptions(camera);
	std::vector<Drawcall*> drawcalls;

	for each(Renderable* renderable in scene->m_renderables)
	{
		// Get data from renderable
		Drawcall* dc = new Drawcall();

		// set up basic info - mesh, mat and model matrix
		dc->m_mesh = renderable->GetMesh();
		dc->m_model = renderable->m_transform.GetWorldMatrix();
		dc->m_material = renderable->GetMaterial();
		dc->m_tint = renderable->GetTint();

		// setup lights
		if (renderable->GetMaterial()->UseLight())
		{
			// Store most effective lights into drawcall
			//ComputeMostContributingLights(dc->m_lightCount, dc->m_lightIndices, renderable->GetWorldPosition(), scene);
			ComputeMostContributingLights(dc->m_lightCount, dc->m_lightIndices, renderable->m_transform.GetWorldPosition(), scene);
		}

		drawcalls.push_back(dc);
	}

	// sort draw call
	std::vector<Drawcall*> sortedDrawcalls = SortDrawcalls(drawcalls);

	// draw call 
	for each (Drawcall* dc in sortedDrawcalls)
	{
		m_renderer->ClearLights();

		// for this draw call update renderer's light array 
		// considering what are the most contributing lights
		for (uint idx = 0; idx < MAX_LIGHTS; ++idx)
		{
			uint lightIdx = dc->m_lightIndices[idx];

			// if light is invalid (index is -1), do not consider it
			if (lightIdx != -1)
			{
				Light* light = scene->m_lights[lightIdx];
				m_renderer->m_lights[idx] = light;
			}
		}

		m_renderer->ResetLightDataFlag();			// 0 for light data, NOT light itself

		for (uint idx = 0; idx < MAX_LIGHTS; ++idx)
		{
			Light* light = m_renderer->m_lights[idx];

			if (light != nullptr)
			{
				Vector3 lightPosition = light->m_renderable->m_transform.GetWorldPosition();

				Rgba lightColor = light->GetColor();
				Vector4 lightColorVec4;
				lightColor.GetAsFloats(lightColorVec4.x, lightColorVec4.y, lightColorVec4.z, lightColorVec4.w);

				Vector3 attenuation = light->m_attenuation;
				Vector3 specAttenuation = light->m_specAttenuation;
				Vector3 dir = light->m_direction;
				float innerAng = light->m_innerAngle;
				float outerAng = light->m_outerAngle;
				float dirFactor = light->m_directionFac;
				bool useShadow = light->m_shadowCast;
				Matrix44 shadowVP = light->m_shadowVP;

				m_renderer->ConfigureLightData(lightPosition, lightColorVec4,
					attenuation, innerAng, specAttenuation, outerAng,
					dir, dirFactor, (int)idx, useShadow, shadowVP);
			}
		}

		m_renderer->Draw(*dc);		// one drawcall for one renderable
	}

	for each (Drawcall* dc in sortedDrawcalls)
	{
		delete dc;
		dc = nullptr;
	}
	sortedDrawcalls.clear();

	// apply effects with materials
	//for each (Material* effect in camera->m_effectMats)
	//{
	//	m_renderer->ApplyEffect(effect);
	//}
	//m_renderer->FinishEffects();
	*/

	//Camera* debugCamera3D = GetDebugRender3DCamera();
	//RenderDebug3DObjectsForCamera(debugCamera3D);
	RenderDebug3DObjectsForCamera(camera);
}

TODO("shadow");
/*
void ForwardRenderPath::RenderShadowCastForLight(Light* light, RenderSceneGraph* scene)
{
	Camera camera;

	switch (light->m_type)
	{
	case DIRECTIONAL_LIGHT:
		break;
	case SPOT_LIGHT:
	case POINT_LIGHT:
	default:
		ASSERT_OR_DIE(false, "Light type not supported for shadows. %u", light->m_type);
		break;
	}
}
*/

void ForwardRenderPath::RenderDebug3DObjectsForCamera(Camera* camera)
{
	DebugRenderSet3DCamera(camera);
	DebugRenderRender3D(m_renderer);
}


void ForwardRenderPath::RenderDebug2DObjects(Camera* camera)
{
	DebugRenderSet2DCamera(camera);
	DebugRenderRender2D(m_renderer);
}


void ForwardRenderPath::ClearBasedOnCameraOptions(Camera*)
{

}


void ForwardRenderPath::ComputeMostContributingLights(uint& lightCount, int* lightIndices, Vector3 renderablePos, RenderSceneGraph* scene)
{
	std::vector<Vector2> lightIndexAndDistanceTuples;

	for (uint lightidx = 0; lightidx < scene->m_lights.size(); ++lightidx)
	{
		Light* light = scene->m_lights[lightidx];

		//Vector3 lightPos = light->m_renderable->GetWorldPosition();
		Vector3 lightPos = light->m_renderable->m_transform.GetWorldPosition();
		float dist = (lightPos - renderablePos).GetLength();
		Vector2 tuple = Vector2((float)lightidx, dist);

		lightIndexAndDistanceTuples.push_back(tuple);
	}

	std::sort(lightIndexAndDistanceTuples.begin(), lightIndexAndDistanceTuples.end(), LightIndexDistanceTupleDepthCompare);

	// fill drawcall's light info 
	for each (Vector2 tuple in lightIndexAndDistanceTuples)
	{
		lightIndices[lightCount] = (int)(tuple.x);
		lightCount++;

		if (lightCount == MAX_LIGHTS)
		{
			break;
		}
	}
}


std::vector<Drawcall*> ForwardRenderPath::SortDrawcalls(std::vector<Drawcall*>& drawcalls)
{
	// sort with layer
	std::sort(drawcalls.begin(), drawcalls.end(), LayerCompare);

	int layerIdxRef = LAYER_START;					// layers start with 0
	std::vector<Drawcall*> DCs;
	std::map<std::string, std::vector<Drawcall*>> layeredDraws;

	for each (Drawcall* dc in drawcalls)
	{
		int layerIdx = dc->GetLayer();
		if (layerIdx != layerIdxRef)
		{
			std::string header = "layer_" + layerIdxRef;
			layeredDraws.emplace(header, DCs);
			
			// go to next layer
			layerIdxRef = layerIdx;
			DCs.clear();

			DCs.push_back(dc);
		}
		else
		{
			DCs.push_back(dc);
		}
	}
	std::string header = "layer_" + layerIdxRef;
	layeredDraws.emplace(header, DCs);

	// sort with queue
	for (std::map<std::string, std::vector<Drawcall*>>::iterator it = layeredDraws.begin();
		it != layeredDraws.end(); ++it)
	{
		std::vector<Drawcall*>& layeredDC = it->second;

		std::sort(layeredDC.begin(), layeredDC.end(), QueueDepthCompare);
	}

	std::vector<Drawcall*> res;

	for (std::map<std::string, std::vector<Drawcall*>>::iterator it = layeredDraws.begin();
		it != layeredDraws.end(); ++it)
	{
		std::vector<Drawcall*>& layeredDC = it->second;

		std::copy(layeredDC.begin(), layeredDC.end(), back_inserter(res));
	}

	return res;
}
