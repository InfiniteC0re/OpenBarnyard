#pragma once

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/T2AtomicMutex_Win.h"
#endif // TOSHI_SKU_WINDOWS

#define T2ATOMICMUTEX_LOCK_SCOPE( mutex ) TANONYMOUSVAR( Toshi::T2AtomicMutexLock )( mutex )

TOSHI_NAMESPACE_START

class T2AtomicMutexLock
{
public:
	T2AtomicMutexLock( T2AtomicMutex& mutex )
	    : m_Mutex( mutex ) { m_Mutex.Lock(); }
	~T2AtomicMutexLock() { m_Mutex.Unlock(); }

private:
	T2AtomicMutex& m_Mutex;
};

TOSHI_NAMESPACE_END
