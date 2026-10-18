#pragma once

#ifdef BARNYARD_COMMUNITY_PATCH

//-----------------------------------------------------------------------------
// A new version of the modloader that will replace legacy BYModCore in favor
// of working directly with OpenBarnyard rather than the retail version of the
// game.
//
// Look into BYModCore if you want to make a mod for the retail build.
//-----------------------------------------------------------------------------

// Define one of these per project in premake:
//   MODAPI_HOST - OpenBarnyard.exe and the Toshi it links (exports symbols)
//   MODAPI_MOD  - mod DLLs (imports symbols from the exe)
#  if defined( MODAPI_HOST ) && defined( MODAPI_MOD )
#	error "MODAPI_HOST and MODAPI_MOD are mutually exclusive"
#  elif defined( MODAPI_HOST )
#	define MODAPI __declspec( dllexport )
#  elif defined( MODAPI_MOD )
#	define MODAPI __declspec( dllimport )
#  else
#	define MODAPI
#  endif

#else // BARNYARD_COMMUNITY_PATCH

#  define MODAPI

#endif // !BARNYARD_COMMUNITY_PATCH
