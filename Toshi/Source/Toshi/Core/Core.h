#pragma once

#define TMAKEVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16) | VER_MINOR)
#define BITFIELD(x) (1 << x)

#if defined(TOSHI_DEBUG)
	#if defined(TOSHI_PLATFORM_WINDOWS)
		#define TBREAK() __debugbreak()
		#define TOSHI_ENABLE_ASSERTS
	#endif
#endif

#ifndef TBREAK
	#define TBREAK() 
#endif

#ifndef TOSHI_PLATFORM_WINDOWS
	#error Unsupported platform
#endif

#if defined(TOSHI_ENABLE_ASSERTS)
	#define TTODO(DESC) { TOSHI_ERROR("TODO: {0} ({1}, at line {2})", DESC, __FUNCTION__, __LINE__); }
	#define TIMPLEMENT() { TOSHI_ERROR("{0} is not implemented", __FUNCTION__); }
	#define TASSERT(x, ...) { if (!(x)) { TOSHI_CORE_ERROR(__VA_ARGS__); TBREAK(); } }
#else
	#define TTODO(DESC)
	#define TIMPLEMENT()
	#define TASSERT(x, ...)
#endif