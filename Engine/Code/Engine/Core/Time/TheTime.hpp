#pragma once

#include <stdint.h>

// gets raw performance counter
uint64_t GetPerformanceCounter(); 

// converts a performance count the seconds it represents
double PerformanceCountToSeconds( uint64_t hpc );

uint64_t SecondsToPerformanceCount(double seconds);
