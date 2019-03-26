#pragma once

#include "Engine/Math/IntVector2.hpp"

#include <string>
#include <vector>
#include <map>

class ProfilerNode
{
public:
	std::string m_tag;
	ProfilerNode* m_parent;
	std::vector<ProfilerNode*> m_childNodes;

	uint64_t m_start;
	uint64_t m_end;

public:
	ProfilerNode(){}
	ProfilerNode(std::string tag, ProfilerNode* parent, uint64_t start)
		: m_tag(tag), m_parent(parent), m_start(start) {}
	~ProfilerNode();

	void AddChild(ProfilerNode* node);
	void DeleteChildren();
	void ProduceText(std::string& res, uint64_t total);

	ProfilerNode* GetParent() const { return m_parent; }

	void SetParent(ProfilerNode* node) { m_parent = node; }
	void SetEnd(const uint64_t& end) { m_end = end; }
};
