#pragma once

#include "Game/Scene/GameState.hpp"
#include "Engine/Physics/2D/DiscEntity.hpp"
#include "Engine/Physics/2D/RectangleEntity.hpp"
#include "Engine/Physics/2D/QuadEntity.hpp"
#include "Engine/Physics/2D/PhysicsScene.hpp"

class PachinkoState : public GameState
{
public:
	PachinkoState();
	~PachinkoState();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override;
	void UpdateUI(float deltaTime);
	void UpdateDelete();

	void Render(Renderer* renderer) override;
	void DrawImmediateTitles(Mesh* mesh);

public:
	PhysicsScene* m_physicsScene;

	DiscEntity* m_d0 = nullptr;
	DiscEntity* m_d1 = nullptr;

	std::vector<DiscEntity*> m_dynamicDiscs;
	std::vector<RectangleEntity*> m_dynamicRecs;
	std::vector<QuadEntity*>	  m_dynamicQuads;

	Mesh* m_titleMesh = nullptr;
	Mesh* m_gravTextMesh = nullptr;
};