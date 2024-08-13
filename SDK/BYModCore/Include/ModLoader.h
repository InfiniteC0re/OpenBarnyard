#pragma once
#include "AModInstance.h"

#include <ToshiTools/T2CommandLine.h>

#include <Windows.h>

#define MODLOADER_EXPORT __declspec( dllexport )
#define MODLOADER_IMPORT __declspec( dllimport )

#if defined( TOSHI_MODLOADER )
#	define MODCORE_API MODLOADER_EXPORT
#elif defined( TOSHI_MODLOADER_CLIENT )
#	define MODCORE_API MODLOADER_IMPORT
#else
#	define MODCORE_API
#endif

MODCORE_API const char* GetModsDirectory();

using t_CreateModInstance = AModInstance* (*)( const Toshi::T2CommandLine* a_pCommandLine );
