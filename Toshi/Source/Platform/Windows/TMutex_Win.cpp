#include "ToshiPCH.h"
#include "TMutex_Win.h"

#include <windows.h>

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
	m_Handle = CreateMutexA( NULL, FALSE, NULL );
	return TTRUE;
}

// $Barnyard: FUNCTION 006bb680
TBOOL TMutex::Destroy()
{
	BOOL result = CloseHandle( m_Handle );
	m_Handle    = NULL;

	return result;
}

TMutex& TMutex::operator=( const TMutex& mutex )
{
	m_Handle = mutex.m_Handle;
	return *this;
}

TMutex& TMutex::operator=( void* handle )
{
	m_Handle = handle;
	return *this;
}

// $Barnyard: FUNCTION 006bb6e0
TBOOL TMutex::Unlock()
{
	return ReleaseMutex( m_Handle );
}

// $Barnyard: FUNCTION 006bb6a0
TBOOL TMutex::Lock( FLAG a_eFlags /*= FLAG_NONE */ )
{
	DWORD waitForMs = ( a_eFlags & FLAG_NO_WAIT ) ? 0 : INFINITE;
	DWORD result    = WaitForSingleObject( m_Handle, waitForMs );

	return result == WAIT_OBJECT_0;
}

TOSHI_NAMESPACE_END
