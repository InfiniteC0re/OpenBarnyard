#include "ToshiPCH.h"
#include "TFifo.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006bb6f0
TBOOL TGenericFifo::Create( TCHAR* a_pBuffer, TINT a_iMaxItems, TINT a_iItemSize )
{
	TPROFILER_SCOPE();

	TASSERT( a_iMaxItems > 0, "Max items is less than zero" );
	TASSERT( a_iItemSize > 0, "Item size is less than zero" );

	m_iItemSize       = a_iItemSize;
	m_iMaxItems       = a_iMaxItems;
	m_pDataPopCursor  = a_pBuffer;
	m_pDataPushCursor = a_pBuffer;
	m_pDataBegin      = a_pBuffer;
	m_pDataEnd        = a_pBuffer + ( a_iMaxItems * a_iItemSize );

	TBOOL bResult;
	bResult = m_semNumFreeItems.Create( m_iMaxItems, m_iMaxItems );
	TASSERT( bResult != TFALSE, "Unable to create semaphore for TGenericFifo" );
	bResult = m_semNumUsedItems.Create( 0, m_iMaxItems );
	TASSERT( bResult != TFALSE, "Unable to create semaphore for TGenericFifo" );

#ifdef USE_WIN32_CRITICAL_SECTION
	InitializeCriticalSection( &m_CriticalSection );
#endif // USE_WIN32_CRITICAL_SECTION

	return TTRUE;
}

TBOOL TGenericFifo::Destroy()
{
	TPROFILER_SCOPE();

#ifdef USE_WIN32_CRITICAL_SECTION
	DeleteCriticalSection( &m_CriticalSection );
#endif // USE_WIN32_CRITICAL_SECTION

	m_semNumFreeItems.Destroy();
	m_semNumUsedItems.Destroy();
	return TTRUE;
}

// $Barnyard: FUNCTION 006bb780
TBOOL TGenericFifo::Push( void* a_pItem, Flags a_iFlags )
{
	TPROFILER_SCOPE();

	TBOOL bResult = ( a_iFlags & Flags_PollSemaphore ) ? m_semNumFreeItems.Poll() : m_semNumFreeItems.Wait();
	if ( !bResult ) return TFALSE;

#ifdef USE_WIN32_CRITICAL_SECTION
	EnterCriticalSection( &m_CriticalSection );
#else // USE_WIN32_CRITICAL_SECTION
	m_Mutex.Lock();
#endif // !USE_WIN32_CRITICAL_SECTION

	// Copy data to the buffer and move cursor
	Toshi::TUtil::MemCopy( m_pDataPushCursor, a_pItem, m_iItemSize );
	m_pDataPushCursor += m_iItemSize;

	if ( m_pDataPushCursor == m_pDataEnd )
	{
		// Set cursor to the beginning if reached the end
		m_pDataPushCursor = m_pDataBegin;
	}

#ifdef USE_WIN32_CRITICAL_SECTION
	LeaveCriticalSection( &m_CriticalSection );
#else // USE_WIN32_CRITICAL_SECTION
	m_Mutex.Unlock();
#endif // !USE_WIN32_CRITICAL_SECTION

	bResult = m_semNumUsedItems.Signal();
	TASSERT( bResult != TFALSE, "TSemaphore::Signal returned TFALSE" );

	return TTRUE;
}

// $Barnyard: FUNCTION 006bb810
TBOOL TGenericFifo::Pop( void* a_pOut, Flags a_iFlags )
{
	TPROFILER_SCOPE();

	TBOOL bResult = ( a_iFlags & Flags_PollSemaphore ) ? m_semNumUsedItems.Poll() : m_semNumUsedItems.Wait();
	if ( !bResult ) return TFALSE;

#ifdef USE_WIN32_CRITICAL_SECTION
	EnterCriticalSection( &m_CriticalSection );
#else  // USE_WIN32_CRITICAL_SECTION
	m_Mutex.Lock();
#endif // !USE_WIN32_CRITICAL_SECTION

	// Copy data to the output and move cursor
	TUtil::MemCopy( a_pOut, m_pDataPopCursor, m_iItemSize );
	m_pDataPopCursor += m_iItemSize;

	if ( m_pDataPopCursor == m_pDataEnd )
	{
		// Set cursor to the beginning if reached the end
		m_pDataPopCursor = m_pDataBegin;
	}

#ifdef USE_WIN32_CRITICAL_SECTION
	LeaveCriticalSection( &m_CriticalSection );
#else  // USE_WIN32_CRITICAL_SECTION
	m_Mutex.Unlock();
#endif // !USE_WIN32_CRITICAL_SECTION

	bResult = m_semNumFreeItems.Signal();
	TASSERT( bResult != TFALSE, "TSemaphore::Signal returned TFALSE" );

	return TTRUE;
}

TOSHI_NAMESPACE_END
