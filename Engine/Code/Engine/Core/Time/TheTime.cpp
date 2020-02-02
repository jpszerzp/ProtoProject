// You will need <windows.h> for these calls.         
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Engine/Core/Time/TheTime.hpp"

//------------------------------------------------------------------------
// local to time.cpp (could also use static variables)
class LocalTimeData 
{
public:
	LocalTimeData() 
	{
		::QueryPerformanceFrequency((LARGE_INTEGER*)&m_hpc_per_second); 
		m_seconds_per_hpc = 1.0 / (double)m_hpc_per_second; 
	}

public:
	uint64_t m_hpc_per_second; 
	double m_seconds_per_hpc; 
}; 

//------------------------------------------------------------------------
// Declaring on the global scope - will cause constructor to be called before our entry function. 
static LocalTimeData gLocalTimeData;


//------------------------------------------------------------------------
// Getting the performance counter
uint64_t GetPerformanceCounter() 
{
	uint64_t hpc;
	::QueryPerformanceCounter((LARGE_INTEGER*)&hpc); 
	return hpc; 
}


//------------------------------------------------------------------------
// Converting to seconds; relies on gLocalTimeData existing;
double PerformanceCountToSeconds(uint64_t hpc) 
{
	return (double)(hpc * gLocalTimeData.m_seconds_per_hpc); 
}

// Converting to hpc
uint64_t SecondsToPerformanceCount(double seconds)
{
	return (uint64_t)(seconds * gLocalTimeData.m_hpc_per_second);
}