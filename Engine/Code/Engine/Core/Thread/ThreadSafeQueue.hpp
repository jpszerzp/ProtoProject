#pragma once

#include "Engine/Core/Thread/NaiveLock.hpp"

#include <deque>
#include <mutex>

template<typename T>
class ThreadSafeQueue
{
public:
	std::deque<T> m_data;
	NaiveLock	  m_lock;

public:
	void Enqueue(const T& data);
	bool Dequeue(T* outData);
};

template<typename T>
void ThreadSafeQueue<T>::Enqueue(const T& data)
{
	m_lock.Enter();
	m_data.push_back(data);
	m_lock.Leave();
}


template<typename T>
bool ThreadSafeQueue<T>::Dequeue(T* outData)
{
	m_lock.Enter();

	bool hasItem = !m_data.empty();
	if (hasItem)
	{
		*outData = m_data.front();
		m_data.pop_front();
	}

	m_lock.Leave();
	return hasItem;
}
