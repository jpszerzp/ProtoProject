#pragma once

#include <stdint.h>

#include "Engine/Core/Profiler/ProfilerNode.hpp"

class ProfileLogScoped
{
public:
	ProfileLogScoped();
	ProfileLogScoped( const char* tag ); 
	~ProfileLogScoped(); 

	void ProfilePush();
	void ProfilePop();

public:
	uint64_t		m_hpcStart; 
	uint64_t		m_hpcEnd;
	const char*		m_tag;
	ProfilerNode*   m_node;
};