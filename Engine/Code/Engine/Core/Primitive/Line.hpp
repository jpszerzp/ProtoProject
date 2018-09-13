#pragma once

#include "Engine/Core/GameObject.hpp"

class Line : public GameObject
{
public:
	void Render(Renderer* renderer) override;
	void Update(float deltaTime) override;

	Line();
	Line(Vector3 start, Vector3 end, float thickness, Rgba tint, 
		std::string materialName, bool multipass = false);
	~Line();

public:
	float m_thickness;
	Vector3 m_start;
	Vector3 m_end;
};