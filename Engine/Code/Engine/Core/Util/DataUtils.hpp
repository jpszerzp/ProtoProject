#pragma once

#include <vector>

//struct Node 
//{
//	int				   m_index;
//	Node*			   m_parent;
//	std::vector<Node*> m_childNodes;
//
//	Node(int idx, Node* parent)
//		: m_index(idx), m_parent(parent){}
//};

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

// delete duplicates with the help of set
template<typename T>
void DeleteDuplicatesVector(std::vector<T>& v)
{
	std::set<T> s;
	for (std::vector<T>::size_type idx = 0; idx < v.size(); ++idx)
	{
		T element = v[idx];
		s.insert(element);
	}
	v.assign(s.begin(), s.end());
}

template<typename T>
std::set<T> ConvertToSetFromVector(const std::vector<T>& v)
{
	std::set<T> s;
	for (std::vector<T>::size_type idx = 0; idx < v.size(); ++idx)
	{
		T element = v[idx];
		s.insert(element);
	}
	return s;
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