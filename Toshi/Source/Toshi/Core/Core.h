#pragma once

constexpr uint32_t TMAKEFOUR(const char str[4])
{
	return ((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
}

#define TMAKEVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16) | VER_MINOR)
#define BITFIELD(x) (1 << x)

#define _TS16(x) L#x

__forceinline uint32_t BIG_ENDIAN_TO_LITTLE(uint32_t val)
{
	return (val >> 8 & 0xff | (uint32_t)(unsigned short)((short)val << 8)) << 0x10 | val >> 0x18 | (uint32_t)(unsigned short)((short)(val >> 0x10) << 8);
}

__forceinline uint16_t PARSEDWORD(const uint8_t bytes[2])
{
	return (bytes[1] << 0) | (bytes[0] << 8);
}

__forceinline uint32_t PARSEWORD(const uint8_t bytes[4])
{
	return (bytes[3] << 0) | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 24);
}

__forceinline uint16_t PARSEDWORD(uint16_t val)
{
	return ((((val) >> 8) & 0xff) | (((val) & 0xff) << 8));
}

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
	#define TIMPLEMENT_D(DESC) { TOSHI_ERROR("{0} is not implemented: {1}", __FUNCTION__, DESC); }
	#define TASSERT(x, ...) { if (!(x)) { TOSHI_CORE_ERROR(__VA_ARGS__); TBREAK(); } }
#else
	#define TTODO(DESC)
	#define TIMPLEMENT()
	#define TIMPLEMENT_D(DESC)
	#define TASSERT(x, ...)
#endif