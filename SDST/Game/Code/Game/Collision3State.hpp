#pragma once

#include "Game/GameState.hpp"
#include "Game/ControlGroup.hpp"

class Collision3State : public GameState
{

public:
	Collision3State();
	~Collision3State();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override; 
	void UpdateUI(float deltaTime);
	void UpdateFocusedGroup(float deltaTime);
	void UpdateDebugDraw(float deltaTime);

	void Render(Renderer* renderer) override;

private:
	std::vector<ControlGroup*> m_controlGrps;
	ControlGroup* m_focusedGrp = nullptr;
	int m_focusedIndex;

	// mesh
	std::vector<Mesh*> m_cpView;
	Mesh* m_title = nullptr;
	float m_textHeight = 0.f;
	Vector2 m_titleMin = Vector2::ZERO;
};