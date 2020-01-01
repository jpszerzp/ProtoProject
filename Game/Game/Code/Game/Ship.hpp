#pragma once

#include "Engine/Core/GameObject.hpp"

class Ship : public GameObject
{
public:
	Ship(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint, std::string meshName, std::string materialName, bool multipass = false);
	~Ship();

	void Update (float deltaTime);
	void Render (Renderer* renderer);
};