#pragma once

#include "Engine/Math/Vector2.hpp"


struct AnalogJoyStick
{
	Vector2 m_rawCartesianPosition;			
	float m_innerDeadZoneFraction = 0.16f;
	float m_outerDeadZoneFraction = 0.95f;

	float m_correctedRadius;
	float m_angleDegree;
};