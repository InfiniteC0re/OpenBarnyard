#include "ToshiPCH.h"
#include "T2Mutex_Win.h"

#include <Windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL T2Mutex::Create()
{
	Destroy();
	m_Mutex = CreateMutexA( NULL, FALSE, NULL );
	TASSERT( m_Mutex != NULL, "Failed to create mutex" );
	return TTRUE;
}

TBOOL T2Mutex::Lock( Flags flags )
{
	TASSERT( m_Mutex != NULL, "Mutex is NULL" );

	DWORD waitForMs = ( flags & Flags_DoNotWait ) ? 0 : INFINITE;
	DWORD result    = WaitForSingleObject( m_Mutex, waitForMs );

	return result == WAIT_OBJECT_0;
}

TBOOL T2Mutex::Unlock()
{
	TASSERT( m_Mutex != NULL, "Mutex is NULL" );
	return ReleaseMutex( m_Mutex );
}

void T2Mutex::Destroy()
{
	CloseHandle( m_Mutex );
	m_Mutex = NULL;
}

TOSHI_NAMESPACE_END
