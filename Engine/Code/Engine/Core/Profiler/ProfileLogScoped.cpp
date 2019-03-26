#include "Engine/Core/Profiler/ProfileLogScoped.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/Profiler/ProfilerNode.hpp"

ProfileLogScoped::ProfileLogScoped(const char* tag, bool my_physics)
{
	m_tag = tag;

	m_my_physics = my_physics;

	// initialize start hpc
	m_hpcStart = GetPerformanceCounter();

	if (m_my_physics)
		PROFILE_LOG_SCOPED_PUSH();		
	else
		PROFILE_LOG_SCOPED_PUSH_PHYSX();
}

ProfileLogScoped::~ProfileLogScoped()
{
	// get end hpc
	m_hpcEnd = GetPerformanceCounter();

	if (m_my_physics)
		PROFILE_LOG_SCOPED_POP();		
	else
		PROFILE_LOG_SCOPED_POP_PHYSX();
}

void ProfileLogScoped::ProfilePush()
{
	// push to normal tree
	// ...

	// this scope has hpc start set
	ProfilerNode* n = new ProfilerNode(m_tag, g_my_tree->GetCurrent(), m_hpcStart);
	g_my_tree->AddNode(n);
	g_my_tree->SetCurrent(n);
}

void ProfileLogScoped::ProfilePop()
{
	// pop from normal tree
	// ...

	// by pop we really mean to record end hpc
	ProfilerNode* current = g_my_tree->GetCurrent();
	current->SetEnd(m_hpcEnd);
	
	// move current to parent
	g_my_tree->SetCurrent(current->GetParent());
}

void ProfileLogScoped::ProfilePushPhysX()
{
	// push to physx tree
	// ...

	ProfilerNode* n = new ProfilerNode(m_tag, g_phys_tree->GetCurrent(), m_hpcStart);
	g_phys_tree->AddNode(n);
	g_phys_tree->SetCurrent(n);
}

void ProfileLogScoped::ProfilePopPhysX()
{
	// pop to physx tree
	// ...

	ProfilerNode* current = g_phys_tree->GetCurrent();
	current->SetEnd(m_hpcEnd);

	// move current to parent
	g_phys_tree->SetCurrent(current->GetParent());
}
