#pragma once

#include "Engine/Core/GameObject.hpp"

class Point : public GameObject
{
public:
	Point();
	~Point();

public:
	virtual void Render(Renderer* renderer) override;
	virtual void Update(float deltaTime) override;
};