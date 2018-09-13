#include "Engine/Core/Profiler/ProfileLogScoped.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Core/Profiler/ProfilerNode.hpp"

ProfileLogScoped::ProfileLogScoped(const char* tag)
{
	m_tag = tag;

	// initialize start hpc
	m_hpcStart = GetPerformanceCounter();

	PROFILE_LOG_SCOPED_PUSH();
}

ProfileLogScoped::ProfileLogScoped()
{

}

ProfileLogScoped::~ProfileLogScoped()
{
	// get end hpc
	m_hpcEnd = GetPerformanceCounter();

	PROFILE_LOG_SCOPED_POP();
}

void ProfileLogScoped::ProfilePush()
{
	// form tree represented by an array
	int idx = g_gameConfigBlackboard->m_profiledFunctionIdx;
	if (g_gameConfigBlackboard->m_functionHpcInfoPairs.empty())
	{
		// stack starts empty
		m_node = new ProfilerNode(idx, m_tag, nullptr);
	}
	else
	{
		int scopeIdx = g_gameConfigBlackboard->m_scopeIdx;
		ProfilerNode* parentNode = g_gameConfigBlackboard->m_profiledFunctionTree[scopeIdx];

		m_node = new ProfilerNode(idx, m_tag, parentNode);
		parentNode->m_childNodes.push_back(m_node);
		g_gameConfigBlackboard->m_scopeIdx += 1;
	}
	g_gameConfigBlackboard->m_profiledFunctionTree[idx] = m_node;
	g_gameConfigBlackboard->m_profiledFunctionIdx += 1;			// note: 1 larger than the meaningful idx is supposed to be

	// update of map
	UInt64Vector2 startOnly = UInt64Vector2(m_hpcStart, 0);
	g_gameConfigBlackboard->m_functionHpcInfoPairs.emplace(m_node, startOnly);
}

void ProfileLogScoped::ProfilePop()
{
	std::map<ProfilerNode*, UInt64Vector2>::iterator it =
		g_gameConfigBlackboard->m_functionHpcInfoPairs.find(m_node);
	it->second.uy = m_hpcEnd;

	// return the scope control to parent
	g_gameConfigBlackboard->m_scopeIdx -= 1;			
	// when poping root, this effectively set scope idx to -1, so need to reset every frame
}
