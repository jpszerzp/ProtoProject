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
std::vector<T> ConvertToVectorFromSet(const std::set<T>& s)
{
	std::vector<T> v;
	for (std::set<T>::iterator it = s.begin(); it != s.end(); ++it)
	{
		T element = *it;
		v.push_back(element);
	}
	return v;
}

// remove all elements in s except ele
template<typename T>
void RemoveExcept(std::set<T>& s, const T& ele)
{
	std::vector<T> v = ConvertToVectorFromSet(s);
	std::vector<T>::iterator it = std::find(v.begin(), v.end(), ele);
	if (it != v.end())
	{
		s.clear();
		s.insert(*it);
		v.clear();
		return;
	}

	s.clear();
	v.clear();
}

// remove all elements in s except elements in v
template<typename T>
void RemoveExcept(std::set<T>& s, const std::vector<T>& v)
{
	std::vector<T> result_v;
	std::vector<T> s_v = ConvertToVectorFromSet(s);
	for (std::vector<T>::size_type idx = 0; idx < v.size(); ++idx)
	{
		const T& ele = v[idx];
		std::vector<T>::iterator it = std::find(s_v.begin(), s_v.end(), ele);
		if (it != s_v.end())
			result_v.push_back(*it);
	}

	// assignment
	s.clear();
	s = ConvertToSetFromVector(result_v);

	// cleanups
	result_v.clear();
	s_v.clear();
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