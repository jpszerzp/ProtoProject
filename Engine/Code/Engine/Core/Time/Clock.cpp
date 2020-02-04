#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


Clock::Clock(Clock* parent)
{
	m_parent = nullptr;

	Reset();
	m_timeScale = 1.f;
	m_paused = false;

	if (parent != nullptr)
	{
		parent->AddChild(this);
	}
}

Clock::~Clock()
{
	if (!m_children.empty())
	{
		for (std::vector<Clock*>::size_type clockCount = 0; clockCount < m_children.size(); ++clockCount)
		{
			Clock* theClock = m_children[clockCount];
			if (theClock)
			{
				delete theClock;
				theClock = nullptr;
			}
		}

		m_children.clear();				// safe to clear after deleting children
	}
}

void Clock::AddChild(Clock* child)
{
	m_children.push_back(child);
}

//float Clock::GetLerpedFPS()
//{
//	if (m_lerped_ms <= 0.0)
//	{
//		m_lerped_ms = frame.mseconds;
//	}
//	else
//	{
//		m_lerped_ms *= 0.99;
//		m_lerped_ms += 0.01 * frame.mseconds;
//		m_lerped_ms =
//	}
//}

//------------------------------------------------------------------------
// resets the clock (called in constructor after
// rest of setup happens)
void Clock::Reset() 
{
	m_startHpc = GetPerformanceCounter();
	m_lastFrameHpc = m_startHpc; 

	frame.Reset(); 
	total.Reset(); 

	// no need to reset children (why?)
	m_frameCount = 0;

	m_lerped_ms = 0.0;
	m_fps = 0.0;
}

//------------------------------------------------------------------------
// only gets called on clocks that have no parent (are root clocks)
void Clock::BeginFrame()
{
	ASSERT_OR_DIE( m_parent == nullptr, "begin frame of clock is called on a clock whose has parent!" ); 
	uint64_t hpc = GetPerformanceCounter(); 
	uint64_t delta = hpc - m_lastFrameHpc; 
	m_lastFrameHpc = hpc;
	Advance(delta); 
}

//------------------------------------------------------------------------
// advance the clock
void Clock::Advance(uint64_t delta)
{
	// Step 0: Update frame count
	// I personally increment frame_count whether it is paused or notas I 
	// use it mostly for reporting, but a good argument could also be made
	// for it respecting paused (for example: using it to make sure something 
	// only happens once a frame, and you treat being paused as staying on the 
	// same frame).  
	++m_frameCount; 

	// Step 1:  Scale/Modify the delta based on internal state
	// ... 
	if (m_paused)
	{
		delta = 0;
	}
	else 
	{
		delta = (uint64_t)((double)delta * m_timeScale);
	}

	// Step 2: Update frame and total variables.  
	// ...
	double elapsed_seconds = PerformanceCountToSeconds(delta);
	frame.seconds = static_cast<float>(elapsed_seconds);
	frame.hpc = delta;
	frame.mseconds = elapsed_seconds * 1000.f;

	total.seconds += frame.seconds;
	total.hpc +=  frame.hpc;
	total.mseconds += frame.mseconds;

	// fps update
	if (m_lerped_ms <= 0.0)
	{
		m_lerped_ms = frame.mseconds;
	}
	else
	{
		m_lerped_ms *= 0.99;
		m_lerped_ms += 0.01 * frame.mseconds;
		m_fps = 1000.0 / m_lerped_ms;
	}

	// Step 3: Call advance on all children 
	// ...
	for(Clock* iter : m_children)
	{
		iter->Advance(delta);
	}
}

Clock* GetMasterClock()
{
	return g_masterClock;
}

void ClockSystemBeginFrame()
{
	g_masterClock->BeginFrame();
}

double GetClockCurrentTime()
{
	return g_masterClock->total.mseconds;
}

float GetDeltaTime()
{
	return g_masterClock->frame.seconds;
}

float GetFrameStartTime()
{
	float startTime = static_cast<float>(PerformanceCountToSeconds(g_masterClock->GetLastFrameHpc()));
	return startTime;
}