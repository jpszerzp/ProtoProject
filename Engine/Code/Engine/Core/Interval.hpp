#pragma once

#include "Engine/Math/Vector3.hpp"

struct DirectionalInterval
{
	Vector3 m_min;
	Vector3 m_max;

	DirectionalInterval(const Vector3& theMin, const Vector3& theMax)
		: m_min(theMin), m_max(theMax) {}

	bool IsPointSmaller(const Vector3& p1, const Vector3& p2) const;
	float GetIntervalLength() const { return (m_max - m_min).GetLength(); }
};