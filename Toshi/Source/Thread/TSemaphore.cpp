#include "ToshiPCH.h"
#include "TSemaphore.h"

#if defined( USE_WIN32_SEMAPHORE )
#  include <windows.h>
#endif // USE_WIN32_SEMAPHORE

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
#if !defined( USE_WIN32_SEMAPHORE )
	if ( m_iCounter.Load() > 0 )
	{
		// Decrement the counter
		TINT32 iState = m_iCounter.Add( -1 ) - 1;

		// See if it actually had some non-zero value
		if ( iState >= 0 )
			return TTRUE;

		// It had zero value, increment it back and wait until it's free to use again
		iState = m_iCounter.Add( 1 ) + 1;
	}

	return TFALSE;
#else  // !USE_WIN32_SEMAPHORE
	return WaitForSingleObject( m_hSemaHnd, 0 ) == WAIT_OBJECT_0;
#endif // USE_WIN32_SEMAPHORE
}

TBOOL TSemaphore::Wait()
{
#if !defined( USE_WIN32_SEMAPHORE )
	while ( TTRUE )
	{
		// Wait until the counter is non-zero
		m_iCounter.WaitForChange( 0 );

		// Decrement the counter
		TINT32 iState = m_iCounter.Add( -1 ) - 1;

		// See if it actually had some non-zero value
		if ( iState >= 0 )
			return TTRUE;
		
		// It had zero value, increment it back and wait until it's free to use again
		iState = m_iCounter.Add( 1 ) + 1;
	}
	
#else  // !USE_WIN32_SEMAPHORE
	return WaitForSingleObject( m_hSemaHnd, INFINITE ) == WAIT_OBJECT_0;
#endif // USE_WIN32_SEMAPHORE
}

// $Barnyard: FUNCTION 006bb640
TBOOL TSemaphore::Signal()
{
#if !defined( USE_WIN32_SEMAPHORE )
	m_iCounter += 1;
	m_iCounter.Signal();
	return TTRUE;
#else  // !USE_WIN32_SEMAPHORE
	TASSERT( m_hSemaHnd != NULL, "Handle is NULL" );
	return ReleaseSemaphore( m_hSemaHnd, 1, NULL );
#endif // USE_WIN32_SEMAPHORE
}

TBOOL TSemaphore::Destroy()
{
	return TFALSE;
}

TBOOL TSemaphore::Create( TINT a_iInitCount, TINT a_iMaxCount )
{
	TASSERT( a_iInitCount >= 0, "Init count should be greater than zero" );
	TASSERT( a_iMaxCount >= a_iInitCount, "Init count cannot be less than max count" );

#if !defined( USE_WIN32_SEMAPHORE )
	m_iCounter = a_iInitCount;
#else  // !USE_WIN32_SEMAPHORE
	m_hSemaHnd = CreateSemaphoreA( NULL, a_iInitCount, a_iMaxCount, NULL );
	TASSERT( m_hSemaHnd != NULL, "Unable to create semaphore" );
#endif // USE_WIN32_SEMAPHORE

	return TTRUE;
}

TOSHI_NAMESPACE_END
