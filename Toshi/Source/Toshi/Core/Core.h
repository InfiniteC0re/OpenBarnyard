#pragma once
#include "Platform.h"
#include "Endianness.h"

constexpr uint32_t TMAKEFOUR(const char str[4])
{
	return ((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
}

#define TCOLOR(R, G, B)         ((((R & 0xff) << 8 | G & 0xff) << 8 | B & 0xff) | 0xFF000000)
#define TCOLOR_GET_RED(COLOR)   ((COLOR >> 16) & 0xff)
#define TCOLOR_GET_GREEN(COLOR) ((COLOR >> 8) & 0xff)
#define TCOLOR_GET_BLUE(COLOR)  (COLOR & 0xff)
#define ISZERO(X) ((X) == 0)
#define HASFLAG(X) (!ISZERO(X))
#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X

#define STRCAT(X, Y) STRCAT2(X, Y)
#define STRCAT2(X, Y) X##Y
#define STRCAT_2 CAT
#define STRCAT_3(X, Y, Z) STRCAT(X, STRCAT(Y, Z))
#define STRCAT_4(A, X, Y, Z) STRCAT(A, STRCAT_3(X, Y, Z))

#define TMAKEVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16) | VER_MINOR)
#define BITFIELD(x) (1U << x)

#define _TS16(x) L##x

#define GLUE(x, y) x y

#define RETURN_ARG_COUNT(_1_, _2_, _3_, _4_, _5_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX5(...) EXPAND_ARGS((__VA_ARGS__, 5, 4, 3, 2, 1, 0))

#define OVERLOAD_MACRO2(name, count) name##count
#define OVERLOAD_MACRO1(name, count) OVERLOAD_MACRO2(name, count)
#define OVERLOAD_MACRO(name, count) OVERLOAD_MACRO1(name, count)

#define CALL_OVERLOAD(name, ...) GLUE(OVERLOAD_MACRO(name, COUNT_ARGS_MAX5(__VA_ARGS__)), (__VA_ARGS__))

#ifdef TOSHI_DEBUG
#ifdef TOSHI_PLATFORM_WINDOWS
#define TBREAK() __debugbreak()
#define TOSHI_ENABLE_ASSERTS
#endif // TOSHI_PLATFORM_WINDOWS
#endif // TOSHI_DEBUG

#ifndef TBREAK
#define TBREAK() 
#endif // TBREAK

#define TREINTERPRETCAST(TYPE, VALUE) (reinterpret_cast<TYPE>(VALUE))
#define TSTATICCAST(TYPE, VALUE) (static_cast<TYPE>(VALUE))
#define TARRAYSIZE(ARRAY) (sizeof(ARRAY) / sizeof(*ARRAY))

#ifdef TOSHI_ENABLE_ASSERTS

typedef enum {
	Ignore,
	Break
} AssertionAction;

AssertionAction AssertionCallback(const char* file, int line, const char* expression);

#define TFIREFLAG static TBOOL FIREFLAG = TFALSE; if (!FIREFLAG)
#define TWIP() { TFIREFLAG { TOSHI_ERROR("Work in progress: {0}, at line {1}", __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
#define TWIP_D(DESC) { TFIREFLAG { TOSHI_ERROR("Work in progress: {0} ({1}, at line {2})", DESC, __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
#define TTODO(DESC) { TFIREFLAG { TOSHI_ERROR("TODO: {0} ({1}, at line {2})", DESC, __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
#define TFIXME(DESC) { TFIREFLAG { TOSHI_WARN("FIXME: {0} ({1}, at line {2})", DESC, __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
#define TIMPLEMENT() { TFIREFLAG { TOSHI_ERROR("{0} is not implemented", __FUNCTION__); FIREFLAG = TTRUE; } }
#define TIMPLEMENT_D(DESC) { TFIREFLAG { TOSHI_ERROR("{0} is not implemented: {1}", __FUNCTION__, DESC); FIREFLAG = TTRUE; } }
#define TASSERT_IMPL(X, ...) { TFIREFLAG if (!(X)) { TOSHI_CORE_ERROR(__VA_ARGS__); if (::AssertionCallback(__FILE__, __LINE__, #X) == AssertionAction::Break) TBREAK(); FIREFLAG = TTRUE; } }
#define TASSERT1(X) TASSERT_IMPL(X, "TASSERT: {0} ({1}, at line {2})", #X, __FILE__, __LINE__)
#define TASSERT2(X, TEXT) TASSERT_IMPL(X, TEXT)
#define TASSERT3(X, TEXT, P1) TASSERT_IMPL(X, TEXT, P1)
#define TASSERT(...) CALL_OVERLOAD(TASSERT, __VA_ARGS__)
#define TVALIDPTR(PTR) TASSERT(PTR)
#else // TOSHI_ENABLE_ASSERTS
#define TWIP()
#define TWIP_D(DESC)
#define TTODO(DESC)
#define TFIXME(DESC)
#define TIMPLEMENT()
#define TIMPLEMENT_D(DESC)
#define TASSERT(x, ...)
#define TVALIDPTR(PTR)
#endif // TOSHI_ENABLE_ASSERTS