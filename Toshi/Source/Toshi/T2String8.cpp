#include "ToshiPCH.h"
#include "T2String8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
	char T2String8::ms_aScratchMem[T2String8::SCRATCH_MEM_SIZE];
}