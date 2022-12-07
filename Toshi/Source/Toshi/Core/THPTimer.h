#pragma once

#ifdef TOSHI_PLATFORM_WINDOWS
#include "Platform/Windows/THPTimer_Win.h"
#else
#error "Unsupported platform"
#endif