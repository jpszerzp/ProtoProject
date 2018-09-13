#pragma once

#include <mutex>

class SpinLock
{
public:
	std::mutex	m_lock;

public:
	void Enter() { m_lock.lock(); }
	void Leave() { m_lock.unlock(); }
	bool TryLeave() { return m_lock.try_lock(); }
};