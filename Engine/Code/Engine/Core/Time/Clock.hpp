#pragma once

#include <vector>

#include "Engine/Core/EngineCommon.hpp"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
struct sTimeUnit
{
	uint64_t hpc; 
	double hpSeconds;   // high precision seconds
	float seconds;      // convenience float seconds

	void Reset()
	{
		memset(this, 0, sizeof(sTimeUnit));
	}
};


//------------------------------------------------------------------------
//------------------------------------------------------------------------
class Clock 
{
public:
	Clock( Clock* parent = nullptr );
	Clock( const Clock& c ) = delete;
	~Clock(); // make sure to cleanup the heirachy

	// resets the clock - everything should be zeroes out
	// and the reference hpc variables set 
	// to the current hpc.
	void Reset(); 

	// used on a root clock
	// calcualtes elapsed time since last call
	// and calls advance.
	void BeginFrame(); 

	// advance the clock by the given hpc
	void Advance( const uint64_t hpc ); 

	// adds a clock as a child to this clock
	void AddChild( Clock* child ); 

	// returns time since this clock 
	// using the current hpc, and our start hpc
	// to get the most up-to-date time.
	double GetCurrentTime() const; 

	// manipulation
	void SetPaused( bool paused ) { m_paused = paused; }
	void SetScale( float scale ) { m_timeScale = scale; }
	void SetParent( Clock* parent ) { m_parent = parent; }

	bool IsPaused() const { return m_paused; }

	// add whatever other convenience methods you may want; 
	// ...
	uint64_t GetLastFrameHpc() const { return total.hpc; }

public:
	// I don't use "m_" here as 
	// the intended usage is to be able to go 
	// clock->total.seconds -or- clock->frame.seconds; 
	// these are only updated at begin_frame.
	sTimeUnit frame; 
	sTimeUnit total; 

private:
	// local data we need to track for the clock to work; 
	uint64_t m_startHpc;      // hpc when the clock was last reset (made)
	uint64_t m_lastFrameHpc;  // hpc during last begin_frame call

	bool m_paused;          
	uint m_frameCount;        // if you're interested in number of frames your game has processes
	double m_timeScale; 

	// For the hierarchy 
	Clock* m_parent;
	std::vector<Clock*> m_children; 
}; 


//------------------------------------------------------------------------
// C Funtions
//------------------------------------------------------------------------
// could calso be static Clock* Clock::GetMaster(); 
Clock* GetMasterClock(); 

// convenience - calls begin frame on the master clock
void ClockSystemBeginFrame();

// I now move this here - as this now refers to the master clock
// who is keeping track of the starting reference point. 
double GetClockCurrentTime(); 

// I usually also add convenience functions so I'm not constantly fetching the master clock, like
float GetDeltaTime();
float GetFrameStartTime();