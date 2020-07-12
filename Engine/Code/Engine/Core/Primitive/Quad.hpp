#pragma once

#include "Engine/Core/GameObject.hpp"

class Quad : public GameObject
{
public:
	Quad();
	~Quad();

public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;
};