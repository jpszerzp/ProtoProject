#include "Engine/Core/Profiler/ProfilerNode.hpp"
#include "Engine/Core/Time/TheTime.hpp"

void ProfilerHistory::Clear()
{
	for (std::map<ProfilerNode*, UInt64Vector2>::iterator it = m_nodeHistory.begin();
		it != m_nodeHistory.end(); ++it)
	{
		ProfilerNode* node = it->first;

		if (node != nullptr)
		{
			delete node;
			node = nullptr;
		}
	}

	m_nodeHistory.clear();
}

// vec2 in format of (totaltime, selftime)
Vector2 ProfilerHistory::InspectNodeTime(ProfilerNode* node)
{
	float totalTime = 0.f;
	float selfTime = 0.f;
	float timeInChildren = 0.f;

	for (std::map<ProfilerNode*, UInt64Vector2>::iterator it = m_nodeHistory.begin();
		it != m_nodeHistory.end(); ++it)
	{
		ProfilerNode* historyNode = it->first;
		UInt64Vector2 lifeSpan = it->second;

		if (historyNode->m_tag == node->m_tag)
		{
			ProfilerNode* parent = node->m_parent;
			ProfilerNode* historyParent = historyNode->m_parent;

			if (parent == nullptr)
			{
				uint64_t hpc = (lifeSpan.uy - lifeSpan.ux);
				float ms = (float)PerformanceCountToSeconds(hpc) * 1000.f;
				totalTime = ms;

				GetTimeInChildren(timeInChildren, historyNode);

				selfTime = totalTime - timeInChildren;

				return Vector2(totalTime, selfTime);
			}

			if (historyParent->m_tag == parent->m_tag)
			{
				uint64_t hpc = (lifeSpan.uy - lifeSpan.ux);
				float ms = (float)PerformanceCountToSeconds(hpc) * 1000.f;
				totalTime += ms;

				GetTimeInChildren(timeInChildren, historyNode);
			}
		}
	}

	selfTime = totalTime - timeInChildren;

	return Vector2(totalTime, selfTime);
}

void ProfilerHistory::GetTimeInChildren(float& time, ProfilerNode* node)
{
	for each (ProfilerNode* child in node->m_childNodes)
	{
		UInt64Vector2 childLife = m_nodeHistory.at(child);
		uint64_t childHpc = childLife.uy - childLife.ux;
		float childms = (float)PerformanceCountToSeconds(childHpc) * 1000.f;
		time += childms;
	}
}
