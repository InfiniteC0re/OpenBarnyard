#pragma once
#include "TMutex.h"

namespace Toshi
{
	class TMutexLock
	{
	public:
		TMutexLock();
		TMutexLock(TMutex& mutex);
		TMutexLock(const TMutexLock&);
		~TMutexLock();

		// Locks the passed mutex
		void Create(TMutex& mutex);

		// Destroys the TMutexLock and TMutex
		void Destroy();

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