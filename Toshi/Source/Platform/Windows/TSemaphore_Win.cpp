#include "ToshiPCH.h"
#include "TSemaphore_Win.h"

#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TSemaphore::~TSemaphore()
{
	Destroy();
}

TBOOL TSemaphore::Poll()
{
	return WaitForSingleObject( m_hSemaHnd, 0 ) == WAIT_OBJECT_0;
}

TBOOL TSemaphore::Wait()
{
	return WaitForSingleObject( m_hSemaHnd, INFINITE ) == WAIT_OBJECT_0;
}

TBOOL TSemaphore::Signal()
{
	TASSERT( m_hSemaHnd != NULL, "Handle is NULL" );
	return ReleaseSemaphore( m_hSemaHnd, 1, NULL );
}

TBOOL TSemaphore::Destroy()
{
	return TFALSE;
}

TBOOL TSemaphore::Create( TINT a_iInitCount, TINT a_iMaxCount )
{
	TASSERT( a_iInitCount >= 0, "Init count should be greater than zero" );
	TASSERT( a_iMaxCount >= a_iInitCount, "Init count cannot be less than max count" );

	m_hSemaHnd = CreateSemaphoreA( NULL, a_iInitCount, a_iMaxCount, NULL );
	TASSERT( m_hSemaHnd != NULL, "Unable to create semaphore" );

	return TTRUE;
}

TOSHI_NAMESPACE_END
