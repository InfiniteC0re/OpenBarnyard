//-----------------------------------------------------------------------------
// Include this file to disable memory debugging.
// 
// Note: This file should be the last include in order to make sure it works.
//       Include TMemoryDebugOn.h to enable debugging mode.
//-----------------------------------------------------------------------------

#if defined(TMEMORY_DEBUG_ENABLED)

#undef new
#undef TMEMORY_DEBUG_ENABLED

#endif
