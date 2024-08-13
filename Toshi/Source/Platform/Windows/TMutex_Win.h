#pragma once

TOSHI_NAMESPACE_START

enum TMutexLockFlag
{
	TMutexLockFlag_DoNotWait = 1,
};

class TMutex
{
public:
	TMutex() = default;

	~TMutex()
	{
		Destroy();
	}

	// Returns TTRUE if success
	TBOOL Create()
	{
		m_Handle = CreateMutexA( NULL, FALSE, NULL );
		return TTRUE;
	}

	// Returns TTRUE if success
	TBOOL Destroy()
	{
		BOOL result = CloseHandle( m_Handle );
		m_Handle	= NULL;

		return result;
	}

	// Returns TTRUE if the state is signaled
	TBOOL Lock( TUINT32 flags = 0 )
	{
		DWORD waitForMs = ( flags & TMutexLockFlag_DoNotWait ) ? 0 : INFINITE;
		DWORD result	= WaitForSingleObject( m_Handle, waitForMs );

		return result == WAIT_OBJECT_0;
	}

	// Returns TTRUE if success
	TBOOL Unlock()
	{
		return ReleaseMutex( m_Handle );
	}

public:
	TMutex& operator=( const TMutex& mutex )
	{
		m_Handle = mutex.m_Handle;
		return *this;
	}

private:
	HANDLE m_Handle;
};

TOSHI_NAMESPACE_END
