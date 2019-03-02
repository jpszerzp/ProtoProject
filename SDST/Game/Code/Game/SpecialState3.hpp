/*
#pragma once

#include "Game/GameState.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

class SpecialState3 : public GameState
{
public:
	SpecialState3();
	~SpecialState3();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override; 
	void UpdateObjects(float deltaTime);
	void UpdateContacts(float deltaTime);
	void UpdateDebugDraw(float deltaTime);
	void UpdateUI(float deltaTime);

	void Render(Renderer* renderer) override;

public:
	const static uint MAX_CONTACT_NUM = 256;

	CollisionKeep m_keep;
	Collision m_storage[MAX_CONTACT_NUM];

	Mesh* m_title_ui;
};
*/