#pragma once
#include "T2Mutex.h"

#define T2MUTEX_LOCK_SCOPE_NAMED( mutex, name ) Toshi::T2MutexLock CONCATTOKEN(_mutex_lock_, name) ( mutex )
#define T2MUTEX_LOCK_SCOPE( mutex ) T2MUTEX_LOCK_SCOPE_NAMED( mutex, __COUNTER__ )

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