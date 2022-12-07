#pragma once

#ifdef TOSHI_PLATFORM_WINDOWS
#include "Platform/Windows/TThread_Win.h"
#else
#error "Unsupported platform"
#endif