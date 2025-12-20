#pragma once

#if defined( TOSHI_SKU_WINDOWS ) && !defined( BARNYARD_COMMUNITY_PATCH )
#  define USE_WIN32_SEMAPHORE
#endif // TOSHI_SKU_WINDOWS && !BARNYARD_COMMUNITY_PATCH

#if !defined( USE_WIN32_SEMAPHORE )
#  include "T2AtomicMutex.h"
#endif // !USE_WIN32_SEMAPHORE

TOSHI_NAMESPACE_START

class TSemaphore
{
public:
	TSemaphore() = default;
	~TSemaphore();

public:
	TBOOL Create( TINT a_iInitCount, TINT a_iMaxCount );
	TBOOL Destroy();
	TBOOL Signal();
	TBOOL Wait();
	TBOOL Poll();

private:
#if defined( USE_WIN32_SEMAPHORE )
	void* m_hSemaHnd;
#else  // TOSHI_SKU_WINDOWS && !BARNYARD_COMMUNITY_PATCH
	T2Atomic32 m_iCounter;
#endif // !TOSHI_SKU_WINDOWS || BARNYARD_COMMUNITY_PATCH
};

TOSHI_NAMESPACE_END
