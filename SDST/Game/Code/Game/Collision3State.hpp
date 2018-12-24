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
	void UpdateFocusedGroup(float deltaTime);
	void UpdateDebugDraw(float deltaTime);

	void Render(Renderer* renderer) override;

private:
	std::vector<ControlGroup*> m_controlGrps;
	ControlGroup* m_focusedGrp = nullptr;
	int m_focusedIndex;
	Mesh* m_origin_mesh =nullptr;
};