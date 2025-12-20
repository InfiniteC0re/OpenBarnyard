#pragma once

#if defined( TOSHI_SKU_WINDOWS ) && !defined( BARNYARD_COMMUNITY_PATCH )
#  define USE_WIN32_MUTEX
#endif // TOSHI_SKU_WINDOWS && !BARNYARD_COMMUNITY_PATCH

#ifndef USE_WIN32_MUTEX
#  include "Thread/T2AtomicMutex.h"
#endif // !USE_WIN32_MUTEX

TOSHI_NAMESPACE_START

class TMutex
{
public:
	enum FLAG
	{
		FLAG_NONE    = 0,
		FLAG_NO_WAIT = BITFLAG( 0 ),
	};

public:
	TMutex();
	~TMutex();

	// Returns TTRUE if success
	TBOOL Create();

	// Returns TTRUE if success
	TBOOL Destroy();

	// Returns TTRUE if the state is signaled
	TBOOL Lock( FLAG a_eFlags = FLAG_NONE );

	// Returns TTRUE if success
	TBOOL Unlock();

public:
	TMutex& operator=( const TMutex& mutex );
	TMutex& operator=( void* handle );

private:
#ifdef USE_WIN32_MUTEX
	void* m_Handle;
#else  // !USE_WIN32_MUTEX
	T2AtomicMutex m_Atomic;
#endif // USE_WIN32_MUTEX
};

TOSHI_NAMESPACE_END
