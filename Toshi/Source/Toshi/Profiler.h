#pragma once

#if defined( TOSHI_PROFILER ) || defined( TOSHI_PROFILER_MEMORY )

#  if defined( TRACY_PROFILER_EXPORTS )
#	define TRACY_EXPORTS
#  else
#	define TRACY_IMPORTS
#  endif // TRACY_PROFILER_EXPORTS

#  include "Profiler/tracy/Tracy.hpp"

#endif // defined(TOSHI_PROFILER) || defined(TOSHI_PROFILER_MEMORY)

#if defined( TOSHI_PROFILER )

#  define TPROFILER_SCOPE()             ZoneScoped
#  define TPROFILER_NAMED( NAME )       ZoneScopedN( NAME )
#  define TPROFILER_FRAME_START( NAME ) FrameMarkStart( NAME )
#  define TPROFILER_FRAME_END( NAME )   FrameMarkEnd( NAME )

#else // defined(TOSHI_PROFILER)

#  define TPROFILER_SCOPE()
#  define TPROFILER_SCOPE_NAMED( NAME )
#  define TPROFILER_FRAME_START( NAME )
#  define TPROFILER_FRAME_END( NAME )

#endif // defined(TOSHI_PROFILER)
