#pragma once
#include "T2Mutex.h"

#define T2MUTEX_LOCK_SCOPE( mutex ) TANONYMOUSVAR( Toshi::T2MutexLock )( mutex )

TOSHI_NAMESPACE_START

class T2MutexLock
{
public:
	T2MutexLock( T2Mutex& mutex )
	    : m_Mutex( mutex ) { m_Mutex.Lock(); }
	~T2MutexLock() { m_Mutex.Unlock(); }

private:
	T2Mutex& m_Mutex;
};

TOSHI_NAMESPACE_END
