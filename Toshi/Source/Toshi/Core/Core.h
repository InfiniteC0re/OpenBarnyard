#pragma once
#include "Platform.h"

constexpr uint32_t TMAKEFOUR(const char str[4])
{
	return ((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
}

#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X

#define STRCAT(X, Y) STRCAT2(X, Y)
#define STRCAT2(X, Y) X##Y
#define STRCAT_2 CAT
#define STRCAT_3(X, Y, Z) STRCAT(X, STRCAT(Y, Z))
#define STRCAT_4(A, X, Y, Z) STRCAT(A, STRCAT_3(X, Y, Z))

#define TMAKEVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16) | VER_MINOR)
#define BITFIELD(x) (1 << x)

#define _TS16(x) L##x

#define GLUE(x, y) x y

#define RETURN_ARG_COUNT(_1_, _2_, _3_, _4_, _5_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX5(...) EXPAND_ARGS((__VA_ARGS__, 5, 4, 3, 2, 1, 0))

#define OVERLOAD_MACRO2(name, count) name##count
#define OVERLOAD_MACRO1(name, count) OVERLOAD_MACRO2(name, count)
#define OVERLOAD_MACRO(name, count) OVERLOAD_MACRO1(name, count)

#define CALL_OVERLOAD(name, ...) GLUE(OVERLOAD_MACRO(name, COUNT_ARGS_MAX5(__VA_ARGS__)), (__VA_ARGS__))

__forceinline uint16_t PARSEWORD(const uint8_t bytes[2])
{
	return (bytes[0] << 0) | (bytes[1] << 8);
}

__forceinline uint16_t PARSEWORD_BIG(const uint8_t bytes[2])
{
	return (bytes[1] << 0) | (bytes[0] << 8);
}

__forceinline uint32_t PARSEDWORD(const uint8_t bytes[4])
{
	return (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

__forceinline uint32_t PARSEDWORD_BIG(const uint8_t bytes[4])
{
	return (bytes[3] << 0) | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 24);
}

__forceinline float PARSEFLOAT_BIG(const uint8_t bytes[4])
{
	uint32_t res = (bytes[3] << 0) | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 24);
	return *((float*)&res);
}

__forceinline uint32_t PARSEDWORD_BIG(uint32_t number)
{
	return PARSEDWORD_BIG(reinterpret_cast<uint8_t*>(&number));
}

__forceinline uint16_t PARSEWORD(uint16_t val)
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

#if defined(TOSHI_ENABLE_ASSERTS)
	#define TFIREFLAG static bool FIREFLAG = false; if (!FIREFLAG)
	#define TWIP() { TFIREFLAG { TOSHI_ERROR("Work in progress: {0}, at line {1}", __FUNCTION__, __LINE__); FIREFLAG = true; } }
	#define TWIP_D(DESC) { TFIREFLAG { TOSHI_ERROR("Work in progress: {0} ({1}, at line {2})", DESC, __FUNCTION__, __LINE__); FIREFLAG = true; } }
	#define TTODO(DESC) { TFIREFLAG { TOSHI_ERROR("TODO: {0} ({1}, at line {2})", DESC, __FUNCTION__, __LINE__); FIREFLAG = true; } }
	#define TIMPLEMENT() { TFIREFLAG { TOSHI_ERROR("{0} is not implemented", __FUNCTION__); FIREFLAG = true; } }
	#define TIMPLEMENT_D(DESC) { TFIREFLAG { TOSHI_ERROR("{0} is not implemented: {1}", __FUNCTION__, DESC); FIREFLAG = true; } }
	#define TASSERT_IMPL(X, ...) { TFIREFLAG if (!(X)) { TOSHI_CORE_ERROR(__VA_ARGS__); TBREAK(); FIREFLAG = true; } }
	#define TASSERT1(X) TASSERT_IMPL(X, "TASSERT: {0} ({1}, at line {2})", #X, __FILE__, __LINE__)
	#define TASSERT2(X, TEXT) TASSERT_IMPL(X, TEXT)
	#define TASSERT3(X, TEXT, P1) TASSERT_IMPL(X, TEXT, P1)
	#define TASSERT(...) CALL_OVERLOAD(TASSERT, __VA_ARGS__)
#else
	#define TWIP()
	#define TWIP_D(DESC)
	#define TTODO(DESC)
	#define TIMPLEMENT()
	#define TIMPLEMENT_D(DESC)
	#define TASSERT(x, ...)
#endif