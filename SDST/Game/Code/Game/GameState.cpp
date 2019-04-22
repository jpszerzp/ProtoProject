#include "Game/GameState.hpp"

GameState::~GameState()
{
	delete m_camera;
	m_camera = nullptr;

	delete m_UICamera;
	m_UICamera = nullptr;

	DeleteGameobjectsNonRenderables();
}


void GameState::FadeIn(float)
{

}


void GameState::FadeOut(float)
{

}


void GameState::DeleteGameobjectsNonRenderables()
{
	for each (GameObject* gameobject in m_gameObjects)
	{
		delete gameobject;
		gameobject = nullptr;
	}

	m_gameObjects.clear();
}

void GameState::DeleteGameobjectsRenderables()
{
	for (int gameobjectIdx = (int)(m_gameObjects.size() - 1U); gameobjectIdx >= 0; --gameobjectIdx)
	{
		GameObject* object = m_gameObjects[gameobjectIdx];

		if (object->m_dead)
		{
			object->m_renderableDelayedDelete = true;
			object->m_renderable->m_delete = true;
		}
	}

	for (int renderableIdx = (int)(m_sceneGraph->m_renderables.size() - 1U); renderableIdx >= 0; --renderableIdx)
	{
		Renderable* renderable = m_sceneGraph->m_renderables[renderableIdx];

		if (renderable->m_delete)
		{
			delete renderable;

			m_sceneGraph->m_renderables[renderableIdx] = m_sceneGraph->m_renderables[m_sceneGraph->m_renderables.size() - 1U];
			m_sceneGraph->m_renderables.pop_back();
		}
	}

	for (int gameobjectIdx = (int)(m_gameObjects.size() - 1U); gameobjectIdx >= 0; --gameobjectIdx)
	{
		GameObject* object = m_gameObjects[gameobjectIdx];

		if (object->m_dead)
		{
			delete object;

			m_gameObjects[gameobjectIdx] = m_gameObjects[m_gameObjects.size() - 1U];
			m_gameObjects.pop_back();
		}
	}
}
