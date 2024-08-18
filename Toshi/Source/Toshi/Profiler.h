#pragma once

#if defined( TOSHI_PROFILER ) || defined( TOSHI_PROFILER_MEMORY )

#  include "Profiler/tracy/Tracy.hpp"

#endif // defined(TOSHI_PROFILER) || defined(TOSHI_PROFILER_MEMORY)

#if defined( TOSHI_PROFILER )

#  define TPROFILER_SCOPE()             ZoneScoped
#  define TPROFILER_FRAME_START( NAME ) FrameMarkStart( NAME )
#  define TPROFILER_FRAME_END( NAME )   FrameMarkEnd( NAME )

#else // defined(TOSHI_PROFILER)

#  define TPROFILER_SCOPE()
#  define TPROFILER_FRAME_START( NAME )
#  define TPROFILER_FRAME_END( NAME )

#endif // defined(TOSHI_PROFILER)
