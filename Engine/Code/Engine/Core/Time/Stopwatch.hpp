#pragma once

#include "Engine/Core/Time/Clock.hpp"

class Stopwatch
{
public:
	Stopwatch(float interval, Clock* clock = nullptr);

	void SetClock(Clock* clock);
	void SetInterval(float seconds);

	float GetElapsedTime();
	float GetNormalizedElapsedTime();
	bool  HasElapsed();
	bool  CheckAndReset();
	void  Reset(float interval = 0.f);

	bool  DecrementOnce();
	uint  DecrementAll();

public:
	Clock* m_reference;

	//float m_start;
	uint64_t m_startHpc;
	float m_interval;
	//uint64_t m_intervalHpc;
};