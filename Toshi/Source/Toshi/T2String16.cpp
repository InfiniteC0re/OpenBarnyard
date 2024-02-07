#include "ToshiPCH.h"
#include "T2String16.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
	wchar_t T2String16::ms_aScratchMem[512];
}