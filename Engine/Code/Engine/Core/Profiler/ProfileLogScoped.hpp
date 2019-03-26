#pragma once

#include <stdint.h>

#include "Engine/Core/Profiler/ProfilerNode.hpp"

//////////////////////////////////////////////////////////////////////////
// This profiler is customized to fit the physics engine thesis project.
// It assumes two profiler tree used in profiler system, one for my API and the other for PhysX.
// That said, it may need modifications to fit more general use case.
//////////////////////////////////////////////////////////////////////////

class ProfileLogScoped
{
public:
	ProfileLogScoped(){}
	ProfileLogScoped(const char* tag, bool my_physics = true); 
	~ProfileLogScoped(); 

	// my api
	void ProfilePush();
	void ProfilePop();

	// physx
	void ProfilePushPhysX();
	void ProfilePopPhysX();

public:
	uint64_t		m_hpcStart; 
	uint64_t		m_hpcEnd;
	bool			m_my_physics;
	const char*		m_tag;
};