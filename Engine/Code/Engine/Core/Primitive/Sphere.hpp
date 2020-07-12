#pragma once

#include "Engine/Core/GameObject.hpp"

class Sphere : public GameObject
{
public:
	Sphere();
	~Sphere();

public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	float GetRadius() const;
};