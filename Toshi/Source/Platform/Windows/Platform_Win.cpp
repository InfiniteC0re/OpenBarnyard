#include "ToshiPCH.h"
#include "Toshi/Platform.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

void ThreadSleep( TUINT dwMilliseconds )
{
	Sleep( 20 );
}

TOSHI_NAMESPACE_END