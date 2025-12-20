#include "ToshiPCH.h"
#include "T2AtomicMutex_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#define FAST_PATH_ATTEMPTS                   200
#define GET_STATEFLAG_NUM_WAITS( NUM_WAITS ) T2AtomicMutex::STATEFLAG_NUM_WAITS << ( NUM_WAITS - 1 )

//-----------------------------------------------------------------------------
// Note: T2AtomicMutex is a custom (isnt't in the original engine) class which
// implementation is heavily inspired on the way Source 2 engine implements it.
//-----------------------------------------------------------------------------

void T2AtomicMutex::Lock()
{
	TUINT32 uiActiveThread = GetCurrentThreadId();

	if ( m_uiLockedThread == uiActiveThread )
	{
		// Don't lock in case some other lock is already made by this thread to avoid deadlocks
		m_uiThreadDepth += 1;
		return;
	}

	TINT iAttempts = 0;
	TINT iState    = m_iState.Add( GET_STATEFLAG_NUM_WAITS( 1 ) ) + GET_STATEFLAG_NUM_WAITS( 1 );

TryLocking:
	// Try to set STATEFLAG_LOCKED
	TINT32 iStateBeforeTry = m_iState.Or( STATEFLAG_LOCKED );
	if ( ( iStateBeforeTry & STATEFLAG_LOCKED ) == 0 )
	{
		// Locked! Can safely leave this function
		m_uiLockedThread = uiActiveThread;
		m_uiThreadDepth  = 1;
		return;
	}

	while ( iState & STATEFLAG_LOCKED )
	{
		if ( iAttempts < FAST_PATH_ATTEMPTS )
		{
			// Fast path, wastes the CPU a lot, but it's fast
			unsigned long      uiWaitTime  = 1 << ( ( iAttempts < 8 ) ? iAttempts : 8 );
			unsigned long long uiEndRDTime = __rdtsc() + uiWaitTime;
		
			do _mm_pause();
			while (__rdtsc() < uiEndRDTime);
		}
		else
		{
			// Slow path, doesn't waste much of the CPU
			// Wait until state flags change
			m_iState.WaitForChange( iState );
		}

		// Update state
		iState = m_iState.Load();
		iAttempts += 1;
	}

	// Try locking again
	goto TryLocking;
}

void T2AtomicMutex::Unlock()
{
	TASSERT( GetCurrentThreadId() == m_uiLockedThread );

	if ( --m_uiThreadDepth == 0 )
	{
		constexpr TINT ONE_LOCK_FLAGS = STATEFLAG_LOCKED | GET_STATEFLAG_NUM_WAITS( 1 );

		// Mark as unused and remove one wait from the total count
		TINT iOldState = m_iState.Add( -ONE_LOCK_FLAGS );

		// Signal only when there's some thread waiting for this mutex to be unlocked to avoid unnecessary kernel calls
		if ( iOldState != ONE_LOCK_FLAGS ) m_iState.Signal();
	}
}

TOSHI_NAMESPACE_END
