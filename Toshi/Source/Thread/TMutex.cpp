#include "ToshiPCH.h"
#include "TMutex.h"

#ifdef USE_WIN32_MUTEX
#  include <windows.h>
#endif // USE_WIN32_MUTEX

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TMutex::TMutex()
{
}

TMutex::~TMutex()
{
	Destroy();
}

// $Barnyard: FUNCTION 006bb660
TBOOL TMutex::Create()
{
#ifdef USE_WIN32_MUTEX
	m_Handle = CreateMutexA( NULL, FALSE, NULL );
#endif // !USE_WIN32_MUTEX
	return TTRUE;
}

// $Barnyard: FUNCTION 006bb680
TBOOL TMutex::Destroy()
{
#ifdef USE_WIN32_MUTEX
	BOOL result = CloseHandle( m_Handle );
	m_Handle    = NULL;
	return result;
#else // USE_WIN32_MUTEX
	return TTRUE;
#endif // !USE_WIN32_MUTEX
}

TMutex& TMutex::operator=( const TMutex& mutex )
{
#ifdef USE_WIN32_MUTEX
	m_Handle = mutex.m_Handle;
#endif // !USE_WIN32_MUTEX
	return *this;
}

TMutex& TMutex::operator=( void* handle )
{
#ifdef USE_WIN32_MUTEX
	m_Handle = handle;
#endif // !USE_WIN32_MUTEX
	return *this;
}

// $Barnyard: FUNCTION 006bb6e0
TBOOL TMutex::Unlock()
{
#ifdef USE_WIN32_MUTEX
	return ReleaseMutex( m_Handle );
#else  // !USE_WIN32_MUTEX
	m_Atomic.Unlock();
	return TTRUE;
#endif // USE_WIN32_MUTEX
}

// $Barnyard: FUNCTION 006bb6a0
TBOOL TMutex::Lock( FLAG a_eFlags /*= FLAG_NONE */ )
{
#ifdef USE_WIN32_MUTEX
	DWORD waitForMs = ( a_eFlags & FLAG_NO_WAIT ) ? 0 : INFINITE;
	DWORD result    = WaitForSingleObject( m_Handle, waitForMs );

	return result == WAIT_OBJECT_0;
#else  // !USE_WIN32_MUTEX
	// TODO: take flags into account?
	m_Atomic.Lock();
	return TTRUE;
#endif // USE_WIN32_MUTEX
}

TOSHI_NAMESPACE_END
