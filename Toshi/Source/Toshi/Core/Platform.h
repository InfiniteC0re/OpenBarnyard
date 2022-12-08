#pragma once

#ifdef TOSHI_PLATFORM_WINDOWS
#define TOSHI_MULTIPLATFORM(FILENAME) STRINGIFY(STRCAT_3(Platform/Windows/, FILENAME, _Win.h))
#else
#error Unsupported platform
#endif