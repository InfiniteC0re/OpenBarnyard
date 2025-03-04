#include "ToshiPCH.h"

#ifndef TMEMORY_USE_DLMALLOC

#  include "Core/TMemory.h"
#  include "Thread/TMutex.h"
#  include "Thread/TMutexLock.h"

#  include <windows.h>

TOSHI_NAMESPACE_START

void* TMemory::SysAlloc( TSIZE a_uiSize )
{
	void* pMem = GlobalAlloc( 64, a_uiSize );
	TVALIDPTR( pMem );

	return pMem;
}

void TMemory::SysFree( void* a_pMem )
{
	TASSERT( TNULL != a_pMem );
	GlobalFree( a_pMem );
}

TOSHI_NAMESPACE_END

#endif // !TMEMORY_USE_DLMALLOC
