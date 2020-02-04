#pragma once

#include "Game/Scene/GameState.hpp"

class PrototypeState : public GameState
{
public:
	PrototypeState();
	~PrototypeState();

	void Update(float dt) override;
	void UpdateMouse(float dt) override;
	void UpdateKeyboard(float dt) override;
	
	void Render(Renderer* renderer) override;

	void DrawImmediateTitles(Mesh* mesh);

public:
	Mesh* m_title_mesh = nullptr;
};