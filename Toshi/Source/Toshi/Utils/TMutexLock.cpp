#include "pch.h"
#include "TMutexLock.h"

namespace Toshi
{
	TMutexLock::TMutexLock()
	{
		m_Mutex = nullptr;
	}

	TMutexLock::TMutexLock(TMutex& mutex)
	{
		Create(mutex);
	}

	TMutexLock::TMutexLock(const TMutexLock& mutexLock)
	{
		m_Mutex = mutexLock.m_Mutex;
	}

	TMutexLock::~TMutexLock()
	{
		Destroy();
	}

	void TMutexLock::Create(TMutex& mutex)
	{
		m_Mutex = &mutex;
		m_Mutex->Lock();
	}

	void TMutexLock::Destroy()
	{
		if (m_Mutex)
		{
			m_Mutex->Unlock();
			m_Mutex = nullptr;
		}
	}
}
