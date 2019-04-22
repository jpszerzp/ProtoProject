#pragma once

#include "Engine/Core/EngineCommon.hpp"

struct TimingData
{
	/** The current render frame. This simply increments. */
	uint m_frame_num;

	/**
	* The timestamp when the last frame ended. Times are
	* given in milliseconds since some undefined time.
	*/
	uint m_last_frame_time_stamp;

	/**
	* The duration of the last frame in milliseconds.
	*/
	uint m_last_frame_duration;

	/**
	* The clockstamp of the end of the last frame.
	*/
	unsigned long m_last_frame_clock_stamp;

	/**
	* The duration of the last frame in clock ticks.
	*/
	unsigned long m_last_frame_clock_tick;

	/**
	* Keeps track of whether the rendering is paused.
	*/
	bool m_paused;

	// Calculated data

	/**
	* This is a recency weighted average of the frame time, calculated
	* from frame durations.
	*/
	double m_average_duration;

	/**
	* The reciprocal of the average frame duration giving the mean
	* fps over a recency weighted average.
	*/
	float m_fps;

	/**
	* Gets the global timing data object.
	*/
	static TimingData& Get();

	/**
	* Updates the timing system, should be called once per frame.
	*/
	static void UpdateTime();

	/**
	* Initialises the frame information system. Use the overall
	* init function to set up all modules.
	*/
	static void Init();

	/**
	* Deinitialises the frame information system.
	*/
	static void Release();

	/**
	* Gets the global system time, in the best resolution possible.
	* Timing is in milliseconds.
	*/
	static unsigned GetTime();

	/**
	* Gets the clock ticks since process start.
	*/
	static unsigned long GetClock();

	static float GetTimeSeconds();
	static float GetTimeDurationSeconds();

private:
	// These are private to stop instances being created: use get().
	TimingData() {}
	TimingData(const TimingData&) {}
	//TimingData& operator=(const TimingData&);
};