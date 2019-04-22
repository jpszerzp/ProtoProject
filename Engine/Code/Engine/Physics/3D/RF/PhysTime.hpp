#pragma once

#include "Engine/Core/EngineCommon.hpp"

class PhysTimeSystem
{
	PhysTimeSystem() {}
	PhysTimeSystem(const PhysTimeSystem&) {}

public:
	uint m_frames;

	uint m_frame_stamp;
	uint m_frame_duration;

	unsigned long m_hpc_stamp;
	unsigned long m_hpc_tick;

	float m_fps;

	bool m_paused;

	double m_average;

	static void InitTimeSystem();
	static PhysTimeSystem& GetTimeSystem();
	static void DestroyTimeSystem();

	void UpdateTime();
	void LerpTime();

	uint GetTime();
	unsigned long GetClock();

	float GetTimeSeconds();
	float GetTimeDurationSeconds();
};