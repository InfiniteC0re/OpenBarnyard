#pragma once
#include "TMutex.h"

namespace Toshi
{
	class TMutexLock
	{
	public:
		TMutexLock()
		{
			m_Mutex = nullptr;
		}

		TMutexLock(TMutex& mutex)
		{
			Create(mutex);
		}

		TMutexLock(const TMutexLock& other)
		{
			m_Mutex = other.m_Mutex;
		}

		~TMutexLock()
		{
			Destroy();
		}

		// Locks the passed mutex
		void Create(TMutex& mutex)
		{
			m_Mutex = &mutex;
			m_Mutex->Lock();
		}

		// Destroys the TMutexLock and TMutex
		void Destroy()
		{
			if (m_Mutex)
			{
				m_Mutex->Unlock();
				m_Mutex = nullptr;
			}
		}

	public:
		TMutexLock& operator = (const TMutexLock& mutexLock)
		{
			m_Mutex = mutexLock.m_Mutex;
			return *this;
		}

	private:
		TMutex* m_Mutex;
	};
}