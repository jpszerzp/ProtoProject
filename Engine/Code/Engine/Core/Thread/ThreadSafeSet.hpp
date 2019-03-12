#pragma once

#include "Engine/Core/Thread/NaiveLock.hpp"

#include <set>

template<typename T>
class ThreadSafeSet
{
public:
	std::set<T> m_data;
	NaiveLock m_lock;

public:
	void Add(const T& data);
	bool Remove(T outData);
	bool RemoveAll();
	bool Find(T data);
};

template<typename T>
void ThreadSafeSet<T>::Add(const T& data)
{
	m_lock.Enter();
	m_data.insert(data);
	m_lock.Leave();
}

template<typename T>
bool ThreadSafeSet<T>::Remove(T outData)
{
	m_lock.Enter();
	
	bool hasItem = !m_data.empty();
	if (hasItem)
	{
		std::set<T>::iterator it = m_data.find(outData);

		if (it != m_data.end())
		{
			// found the data, erase it
			m_data.erase(it);
			return true;
		}
		else
		{
			// data not found; no data to erase
			return false;
		}
	}

	m_lock.Leave();
	return false;		// no item in set; no data to erase
}

template<typename T>
bool ThreadSafeSet<T>::RemoveAll()
{
	m_lock.Enter();

	m_data.clear();

	m_lock.Leave();
	return true;
}

template<typename T>
bool ThreadSafeSet<T>::Find(T data)
{
	std::set<T>::iterator it = m_data.find(data);
	if (it != m_data.end())
	{
		return true;
	}

	return false;
}