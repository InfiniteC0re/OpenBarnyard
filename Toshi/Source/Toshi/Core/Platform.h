#pragma once

#ifdef TOSHI_SKU_WINDOWS

#define TOSHI_MULTIPLATFORM(FILENAME) STRINGIFY(STRCAT_3(Platform/Windows/, FILENAME, _Win.h))

#ifdef TOSHI_RENDERER_DX11

#define TOSHI_MULTIRENDER(FILENAME) STRINGIFY(STRCAT_3(Platform/DX11/, FILENAME, _DX11.h))

#endif // TOSHI_RENDERER_DX11

#ifdef TOSHI_RENDERER_OPENGL

#define TOSHI_MULTIRENDER(FILENAME) STRINGIFY(STRCAT_3(Platform/SDL/, FILENAME, _SDL.h))

#endif // TOSHI_RENDERER_OPENGL

#ifdef TOSHI_SKU_REV_TO_Win

#define TOSHI_MULTIPLATFORM(FILENAME) STRINGIFY(STRCAT_3(Platform/Revolution/, FILENAME, _Rev.h))

#endif // TOSHI_SKU_REV_TO_Win

#else TOSHI_SKU_REVOLUTION

#error Unsupported platform

#endif // TOSHI_SKU_REVOLUTION

namespace Toshi {

	enum class Platform
	{
		Wii,
		Unk2,
		PC,
		Unk3
	};

}
