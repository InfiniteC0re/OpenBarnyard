//-----------------------------------------------------------------------------
// Include this file to enable memory debugging if TOSHI_PROFILER_MEMORY is defined.
// 
// Note: This file should be the last include in order to make sure it works.
//       Include TMemoryDebugOff.h to disable debugging mode.
//-----------------------------------------------------------------------------

#if defined(TOSHI_PROFILER_MEMORY) && !defined(TOSHI_PROFILER_MEMORY_ENABLED)

#include "TMemoryDebug.h"

#define new (TMemory__FILE__ = __FILE__, TMemory__LINE__ = __LINE__) && TFALSE ? TNULL : new
#define TMalloc(...) TMalloc(__VA_ARGS__, __FILE__, __LINE__)
#define TOSHI_PROFILER_MEMORY_ENABLED

#endif
