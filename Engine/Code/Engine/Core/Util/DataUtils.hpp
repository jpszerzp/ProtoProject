#pragma once

#include <vector>

struct Node 
{
	int				   m_index;
	Node*			   m_parent;
	std::vector<Node*> m_childNodes;

	Node(int idx, Node* parent)
		: m_index(idx), m_parent(parent){}
};

template<typename T>
void DeleteVector(std::vector<T*>& vector)
{
	for each (T* element in vector)
	{
		delete element;
		element = nullptr;
	}

	vector.clear();
}

template<typename T>
int PickLargestElementIndexArray(T t_arg, size_t size)
{
	return 0;
}

template<typename T>
int PickSmallestElementIndexArray(T t_arg, size_t size)
{
	return 0;
}