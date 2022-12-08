#pragma once
#include "T2Mutex.h"

namespace Toshi
{
	class T2MutexLock
	{
	public:
		T2MutexLock(T2Mutex& mutex) : m_Mutex(mutex) { m_Mutex.Lock(); }
		~T2MutexLock() { m_Mutex.Unlock(); }

	private:
		T2Mutex& m_Mutex;
	};
}