#pragma once

#ifdef TOSHI_PLATFORM_WINDOWS
#include "Platform/Windows/TSemaphore_Win.h"
#else
#error "Unsupported platform"
#endif