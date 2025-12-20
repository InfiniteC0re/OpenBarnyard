#pragma once
#include "T2Atomic_Win.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: uses atomic numbers to implement locking. Faster than TMutex, cause
// doesn't trigger context switches on fast operations.
//-----------------------------------------------------------------------------
class T2AtomicMutex
{
private:
	using StateFlag = TINT32;
	enum StateFlags : StateFlag
	{
		STATEFLAG_NONE      = 0,
		STATEFLAG_LOCKED    = BITFLAG( 0 ),
		STATEFLAG_NUM_WAITS = BITFLAG( 1 ), // STATEFLAG_NUM_WAITS << ( NUM_WAITS - 1 )
	};

public:
	constexpr T2AtomicMutex() = default;
	~T2AtomicMutex() = default;

	void Lock();
	void Unlock();

private:
	T2Atomic32       m_iState         = 0;
	volatile TUINT32 m_uiLockedThread = 0;
	volatile TUINT32 m_uiThreadDepth  = 0;
};

TOSHI_NAMESPACE_END
