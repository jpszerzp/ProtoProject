#include "Engine/Physics/3D/RF/PhysTime.hpp"

static bool m_pc_flag;

#define TIME_WINDOWS 1

#include <windows.h>
#include <mmsystem.h>
#include <ctime>
#pragma comment(lib, "winmm.lib")  

static double m_pc_frequency;
static TimingData* the_time = nullptr;

unsigned GetSysTime()
{
	if (m_pc_flag)
	{
		static LONGLONG pc_millis_per_tick;
		QueryPerformanceCounter((LARGE_INTEGER*)&pc_millis_per_tick);
		return (unsigned)(pc_millis_per_tick * m_pc_frequency);
	}
	else
		return unsigned(timeGetTime());
}

unsigned TimingData::GetTime()
{
	return GetSysTime();
}

unsigned long GetSysClock()
{
	return __rdtsc();
}

unsigned long TimingData::GetClock()
{
	return GetSysClock();
}

float TimingData::GetTimeSeconds()
{
	return (float)(TimingData::Get().m_last_frame_time_stamp * .001f);
}

float TimingData::GetTimeDurationSeconds()
{
	return (float)(TimingData::Get().m_last_frame_duration * .001f);
}

void InitTime()
{
	LONGLONG time;

	m_pc_flag = (QueryPerformanceFrequency((LARGE_INTEGER*)&time) > 0);

	if (m_pc_flag)
		m_pc_frequency = 1000.f / time;
}

TimingData& TimingData::Get()
{
	return (TimingData&)*the_time;
}

void TimingData::UpdateTime()
{
	if (!the_time)
		return;

	if (!the_time->m_paused)
		the_time->m_frame_num++;

	unsigned this_time = GetTime();
	the_time->m_last_frame_duration = this_time - the_time->m_last_frame_time_stamp;
	the_time->m_last_frame_time_stamp = this_time;

	unsigned long this_clock = GetClock();
	the_time->m_last_frame_clock_tick = this_clock - the_time->m_last_frame_clock_stamp;
	the_time->m_last_frame_clock_stamp = this_clock;

	if (the_time->m_frame_num > 1) {
		if (the_time->m_average_duration <= 0)
			the_time->m_average_duration = (double)the_time->m_last_frame_duration;
		else
		{
			the_time->m_average_duration *= 0.99;
			the_time->m_average_duration += 0.01 * (double)the_time->m_last_frame_duration;

			the_time->m_fps = (float)(1000.0 / the_time->m_average_duration);
		}
	}
}

void TimingData::Init()
{
	InitTime();

	if (!the_time)
		the_time = new TimingData();

	the_time->m_frame_num = 0;

	the_time->m_last_frame_time_stamp = GetTime();
	the_time->m_last_frame_duration = 0;

	the_time->m_last_frame_clock_stamp = GetClock();
	the_time->m_last_frame_clock_tick = 0;

	the_time->m_paused = false;

	the_time->m_average_duration = 0;
	the_time->m_fps = 0;
}

void TimingData::Release()
{
	delete the_time;
	the_time = nullptr;
}
