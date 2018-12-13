#pragma once

#include "Game/GameState.hpp"
#include "Game/Ship.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Cube.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Core/Primitive/Line.hpp"

#include <vector>

class ProtoState : public GameState
{
public:
	ProtoState();
	~ProtoState();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override;

	void Render(Renderer* renderer) override;

public:
	Cube* c_0 = nullptr;
	Cube* c_1 = nullptr;
	Quad* q_0 = nullptr;
	Sphere* s_0 = nullptr;
	Point* p_0 = nullptr;
	Line* l_0 = nullptr;
	Ship* ship_0 = nullptr;
};