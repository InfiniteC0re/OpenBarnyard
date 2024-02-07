//-----------------------------------------------------------------------------
// Include this file to disable memory debugging.
// 
// Note: This file should be the last include in order to make sure it works.
//       Include TMemoryDebugOn.h to enable debugging mode.
//-----------------------------------------------------------------------------

#ifdef TMEMORY_DEBUG_ENABLED

#undef new
#undef TMalloc1
#undef TMalloc2
#undef TMalloc
#undef TMEMORY_DEBUG_ENABLED

#endif // TMEMORY_DEBUG_ENABLED
