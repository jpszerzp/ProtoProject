
#pragma once

#include "Engine/Core/GameObject.hpp"

class Box : public GameObject
{
public:
	Box();
	~Box();

public:
	void Update(float deltaTime) override;
};