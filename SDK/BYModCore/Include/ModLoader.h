#pragma once
#include "AModInstance.h"

#include <Windows.h>

#define MODLOADER_EXPORT __declspec(dllexport)
#define MODLOADER_IMPORT __declspec(dllimport)

#if defined(TOSHI_MODLOADER)
#define MODCORE_API MODLOADER_EXPORT
#elif defined(TOSHI_MODLOADER_CLIENT)
#define MODCORE_API MODLOADER_IMPORT
#else
#define MODCORE_API
#endif

using t_CreateModInstance = AModInstance*(*)();
