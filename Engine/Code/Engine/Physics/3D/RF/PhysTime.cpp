#include "Engine/Physics/3D/RF/PhysTime.hpp"


#include <windows.h>
#include <mmsystem.h>
#include <ctime>
#pragma comment(lib, "winmm.lib")  

static bool m_flag;
static double m_frequency;
static PhysTimeSystem* theTime = nullptr;

uint GetSysTime()
{
	if (m_flag)
	{
		static LONGLONG time_per_tick;
		QueryPerformanceCounter((LARGE_INTEGER*)&time_per_tick);
		return (uint)(time_per_tick * m_frequency);
	}
	else
		return (uint)(timeGetTime());
}

uint PhysTimeSystem::GetTime()
{
	return GetSysTime();
}

unsigned long GetSysClock()
{
	return (unsigned long)(__rdtsc());
}

unsigned long PhysTimeSystem::GetClock()
{
	return GetSysClock();
}

float PhysTimeSystem::GetTimeSeconds()
{
	
	return (float)(theTime->m_frame_stamp * .001f);
}

float PhysTimeSystem::GetTimeDurationSeconds()
{
	return (float)(theTime->m_frame_duration * .001f);
}

void InitTime()
{
	LONGLONG time;

	m_flag = (QueryPerformanceFrequency((LARGE_INTEGER*)&time) > 0);

	if (m_flag)
		m_frequency = 1000.f / time;
}

PhysTimeSystem& PhysTimeSystem::GetTimeSystem()
{
	return (PhysTimeSystem&)*theTime;
}

void PhysTimeSystem::UpdateTime()
{
	if (!theTime)
		return;

	if (!theTime->m_paused)
		theTime->m_frames++;

	uint this_time = GetTime();
	theTime->m_frame_duration = this_time - theTime->m_frame_stamp;
	theTime->m_frame_stamp = this_time;

	unsigned long this_clock = GetClock();
	theTime->m_hpc_tick = this_clock - theTime->m_hpc_stamp;
	theTime->m_hpc_stamp = this_clock;

	LerpTime();
}

void PhysTimeSystem::InitTimeSystem()
{
	InitTime();

	if (!theTime)
		theTime = new PhysTimeSystem();

	theTime->m_frames = 0;

	theTime->m_frame_stamp = theTime->GetTime();
	theTime->m_frame_duration = 0;

	theTime->m_hpc_stamp = theTime->GetClock();
	theTime->m_hpc_tick = 0;

	theTime->m_paused = false;

	theTime->m_average = 0;
	theTime->m_fps = 0;
}

void PhysTimeSystem::DestroyTimeSystem()
{
	delete theTime;
	theTime = nullptr;
}

void PhysTimeSystem::LerpTime()
{
	if (theTime->m_frames > 1) {
		if (theTime->m_average <= 0)
			theTime->m_average = (double)theTime->m_frame_duration;
		else
		{
			theTime->m_average *= 0.99;
			theTime->m_average += 0.01 * (double)theTime->m_frame_duration;

			theTime->m_fps = (float)(1000.0 / theTime->m_average);
		}
	}
}