#pragma once

#include "Engine/Core/GameObject.hpp"

#include <vector>

class ControlGroup
{

private:
	std::vector<GameObject*> m_gos;

public:
	void ProcessInput();

};