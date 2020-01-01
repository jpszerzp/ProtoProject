#pragma once

#include "Game/Scene/GameState.hpp"

class AIPrototypeState : public GameState
{
public:
	AIPrototypeState();
	~AIPrototypeState();

	void Update(float dt) override;
	void UpdateMouse(float dt) override;
	void UpdateKeyboard(float dt) override;
	
	void Render(Renderer* renderer) override;

	void DrawImmediateTitles(Mesh* mesh);

public:
	Mesh* m_titleMesh = nullptr;
};