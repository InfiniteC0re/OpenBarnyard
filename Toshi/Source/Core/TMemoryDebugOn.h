//-----------------------------------------------------------------------------
// Include this file to enable memory debugging if TMEMORY_DEBUG is defined.
// 
// Note: This file should be the last include in order to make sure it works.
//       Include TMemoryDebugOff.h to disable debugging mode.
//-----------------------------------------------------------------------------

#if defined(TMEMORY_DEBUG) && !defined(TMEMORY_DEBUG_ENABLED)

#define new new(__FILE__, __LINE__)
#define TMEMORY_DEBUG_ENABLED

#endif
