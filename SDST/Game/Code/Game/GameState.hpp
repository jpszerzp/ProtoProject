#pragma once

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/ForwardRenderPath.hpp"
#include "Engine/Renderer/RenderSceneGraph.hpp"
#include "Engine/Core/GameObject.hpp"

enum ePrimitiveControl
{
	CONTROL_CUBE,
	CONTROL_SHPERE,
	NUM_PRIMITIVE_CONTROL
};

class GameState
{
public:
	Camera* m_camera = nullptr;
	Camera* m_UICamera = nullptr;

	float m_cameraRotationSpd;
	Vector3 m_cameraInitialPos;

	ForwardRenderPath* m_forwardPath;
	RenderSceneGraph*  m_sceneGraph;

	std::vector<GameObject*> m_gameObjects;
	std::vector<GameObject*> m_rigidBodyObjects;

	ePrimitiveControl m_control;

public:
	GameState(){}
	~GameState();

	virtual void Update(float deltaTime) = 0;
	virtual void UpdateMouse(float deltaTime) = 0;
	virtual void UpdateKeyboard(float deltaTime) = 0;

	virtual void Render(Renderer* renderer) = 0;

	virtual void FadeIn(float duration);
	virtual void FadeOut(float duration);

	void DeleteGameobjectsNonRenderables();
	void DeleteGameobjectsRenderables();
};