#pragma once

#ifdef TOSHI_PLATFORM_WINDOWS
#define TOSHI_MULTIPLATFORM(FILENAME) STRINGIFY(STRCAT_3(Platform/Windows/, FILENAME, _Win.h))
#define TOSHI_MULTIRENDER(FILENAME) STRINGIFY(STRCAT_3(Platform/Windows/DX11/, FILENAME, _DX11.h))
#else
#error Unsupported platform
#endif