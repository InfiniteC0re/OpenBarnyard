#pragma once
#include "TMutex.h"

#define TMUTEX_LOCK_SCOPE( mutex ) TANONYMOUSVAR( Toshi::TMutexLock )( mutex )

TOSHI_NAMESPACE_START

class TMutexLock
{
public:
	TMutexLock()
	{
		m_Mutex = TNULL;
	}

	TMutexLock( TMutex& mutex )
	{
		Create( &mutex );
	}

	TMutexLock( TMutex* mutex )
	{
		Create( mutex );
	}

	TMutexLock( const TMutexLock& other )
	{
		m_Mutex = other.m_Mutex;
	}

	~TMutexLock()
	{
		Destroy();
	}

	// Locks the passed mutex
	void Create( TMutex* mutex )
	{
		m_Mutex = mutex;
		m_Mutex->Lock();
	}

	// Destroys the TMutexLock and TMutex
	void Destroy()
	{
		if ( m_Mutex )
		{
			m_Mutex->Unlock();
			m_Mutex = TNULL;
		}
	}

public:
	TMutexLock& operator=( const TMutexLock& mutexLock )
	{
		m_Mutex = mutexLock.m_Mutex;
		return *this;
	}

private:
	TMutex* m_Mutex;
};

TOSHI_NAMESPACE_END
