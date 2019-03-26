#include "Engine/Core/Profiler/ProfilerNode.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Core/Util/StringUtils.hpp"

ProfilerNode::~ProfilerNode()
{
	DeleteChildren();

	m_parent = nullptr;
}

void ProfilerNode::AddChild(ProfilerNode* node)
{
	m_childNodes.push_back(node);
	
	//node->SetParent(this);
}

void ProfilerNode::DeleteChildren()
{
	for (int i = 0; i < m_childNodes.size(); ++i)
	{
		ProfilerNode* child_node = m_childNodes[i];

		delete child_node;
	}

	m_childNodes.clear();
}

void ProfilerNode::ProduceText(std::string& res, uint64_t total)
{
	// in format of name | inc percentage
	std::string text = m_tag + "\n";
	res += text;

	for (int i = 0; i < m_childNodes.size(); ++i)
	{
		ProfilerNode* child = m_childNodes[i];
		child->ProduceText(res, total);
	}
}
