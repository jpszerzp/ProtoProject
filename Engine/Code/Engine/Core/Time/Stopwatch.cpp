#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Core/Time/TheTime.hpp"

Stopwatch::Stopwatch(float interval, Clock* clock)
{
	if (clock == nullptr)
	{
		m_reference = g_masterClock;
	}
	else
	{
		m_reference = clock;
	}
	
	//m_start = 0.f;
	m_startHpc = m_reference->GetLastFrameHpc();
	m_interval = interval;
	//m_intervalHpc = SecondsToPerformanceCount(interval);
}


void Stopwatch::SetClock(Clock* clock)
{
	m_reference = clock;
}


void Stopwatch::SetInterval(float seconds)
{
	//m_intervalHpc = SecondsToPerformanceCount(seconds);
	m_interval = seconds;
}


float Stopwatch::GetElapsedTime()
{
	// hpc since start
	uint64_t currentHpc = m_reference->GetLastFrameHpc();
	uint64_t elapsedHpc = currentHpc - m_startHpc;

	return (float)PerformanceCountToSeconds(elapsedHpc);
}


float Stopwatch::GetNormalizedElapsedTime()
{
	float elapsed = GetElapsedTime();
	//float interval = (float)(PerformanceCountToSeconds(m_intervalHpc));

	return elapsed / m_interval;
}


bool Stopwatch::HasElapsed()
{
	float elapsed = GetElapsedTime();
	//float interval = (float)(PerformanceCountToSeconds(m_intervalHpc));

	return elapsed > m_interval;
}


void Stopwatch::Reset(float interval)
{
	m_startHpc = m_reference->GetLastFrameHpc();
	m_interval = interval;
	//m_intervalHpc = SecondsToPerformanceCount(interval);
}


bool Stopwatch::CheckAndReset()
{
	if (HasElapsed())
	{
		Reset();
		return true;
	}
	else
	{
		return false;
	}
}


bool Stopwatch::DecrementOnce()
{
	bool hasElapsed = HasElapsed();

	if (hasElapsed)
	{
		// advance start hpc
		uint64_t intervalHpc = SecondsToPerformanceCount(m_interval);
		m_startHpc += intervalHpc;
	}
	
	return hasElapsed;
}


uint Stopwatch::DecrementAll()
{
	float elapsed = GetElapsedTime();
	//float interval = (float)(PerformanceCountToSeconds(m_intervalHpc));

	int iteration = (int)(trunc(elapsed / m_interval));

	while (HasElapsed())
	{
		DecrementOnce();
	}

	return iteration;
}