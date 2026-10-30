#pragma once

//-----------------------------------------------------------------------------
// PrimeDebug: the debug tooling of Prime (Blue Tongue's engine after TOSHI,
// de Blob 2), ported from the OpenPrime decompilation without Prime's kernel:
// only the layout and the logic, on TOSHI's types, strings and logging.
// Functions ported from OpenPrime keep their `$deBlob2` markers (de Blob 2
// retail addresses, provenance only).
//
// PRIMEDEBUG     the console (variables, commands, command lines): every
//                BARNYARD_COMMUNITY_PATCH build
// PRIMEDEBUG_DEV the development tools (key bindings, the debug menu, debug
//                text and windows, primitives, profile counters, frame
//                stats): not in Final builds
//
// Prime's colours are R8G8B8A8 (0xAABBGGRR); here every colour is TOSHI's
// 0xAARRGGBB, the colour constants of the ported code are converted.
// Drawing goes through PDebugRenderer, implemented by the game.
//-----------------------------------------------------------------------------

#ifdef BARNYARD_COMMUNITY_PATCH
#  define PRIMEDEBUG
#  ifndef TOSHI_FINAL
#	define PRIMEDEBUG_DEV
#  endif
#endif

#include <Toshi/Toshi.h>
