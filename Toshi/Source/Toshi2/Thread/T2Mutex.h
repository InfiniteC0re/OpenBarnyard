#pragma once

#ifdef TOSHI_PLATFORM_WINDOWS
#include "Platform/Windows/T2Mutex_Win.h"
#else
#error "Unsupported platform"
#endif