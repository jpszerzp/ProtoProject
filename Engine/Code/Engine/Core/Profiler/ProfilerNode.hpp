#pragma once

//#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/IntVector2.hpp"

#include <string>
#include <vector>
#include <map>

class ProfilerNode
{
public:
	int m_idx;
	int m_layer = 0;
	std::string m_tag;
	ProfilerNode* m_parent;
	std::vector<ProfilerNode*> m_childNodes;

public:
	ProfilerNode(){}
	ProfilerNode(int idx, std::string tag, ProfilerNode* parent)
		: m_idx(idx), m_tag(tag), m_parent(parent) {}
	~ProfilerNode(){}
};

class ProfilerHistory
{
public:
	std::map<ProfilerNode*, UInt64Vector2>		m_nodeHistory;

public:
	ProfilerHistory(std::map<ProfilerNode*, UInt64Vector2> nodeHistory)
		: m_nodeHistory(nodeHistory){}

	void Clear();

	Vector2 InspectNodeTime(ProfilerNode* node);
	void GetTimeInChildren(float& time, ProfilerNode* node);
};