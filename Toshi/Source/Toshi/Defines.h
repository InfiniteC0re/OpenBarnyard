#pragma once
#include "Toshi/Typedefs.h"

constexpr TUINT32 TMAKEFOUR(const char str[4])
{
	return ((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
}

#define ISZERO(X) ((X) == 0)
#define HASFLAG(X) (!ISZERO(X))
#define STRINGIFY(X) STRINGIFY2(X)    
#define STRINGIFY2(X) #X

#define STRCAT(X, Y) STRCAT2(X, Y)
#define STRCAT2(X, Y) X##Y
#define STRCAT_2 CAT
#define STRCAT_3(X, Y, Z) STRCAT(X, STRCAT(Y, Z))
#define STRCAT_4(A, X, Y, Z) STRCAT(A, STRCAT_3(X, Y, Z))

#define TDEPRECATED(...) [[deprecated(__VA_ARGS__)]]
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

#define TOSHI_NAMESPACE_USING using namespace Toshi;

#ifdef TOSHI_DIST
#define TOSHI_NO_LOGS
#endif

#ifdef TOSHI_DEBUG
#ifdef TOSHI_SKU_WINDOWS
#define TBREAK() __debugbreak()
#define TOSHI_ENABLE_ASSERTS
#endif // TOSHI_SKU_WINDOWS
#endif // TOSHI_DEBUG

#ifndef TBREAK
#define TBREAK() 
#endif // TBREAK

#define TSTATICASSERT(...) static_assert(__VA_ARGS__, "Compile time assert failed: " #__VA_ARGS__)
#define TREINTERPRETCAST(TYPE, VALUE) (reinterpret_cast<TYPE>(VALUE))
#define TSTATICCAST(TYPE, VALUE) (static_cast<TYPE>(VALUE))
#define TARRAYSIZE(ARRAY) (sizeof(ARRAY) / sizeof(*ARRAY))
#define TINLINE inline
#define TFORCEINLINE __forceinline

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
#define TASSERT_IMPL(X, ...) { TFIREFLAG if (!(X)) { TOSHI_ERROR(__VA_ARGS__); if (::AssertionCallback(__FILE__, __LINE__, #X) == AssertionAction::Break) TBREAK(); FIREFLAG = TTRUE; } }
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
#define TASSERT1(x, ...)
#define TASSERT2(x, ...)
#define TASSERT3(x, ...)
#define TASSERT(x, ...)
#define TVALIDPTR(PTR)

#endif // TOSHI_ENABLE_ASSERTS


#ifndef TOSHI_NO_LOGS

#ifdef TOSHI_USER_ENGINE

// Engine log macros
#define TOSHI_INFO(...) Toshi::TLog::GetCoreLogger()->info(__VA_ARGS__)
#define TOSHI_WARN(...) Toshi::TLog::GetCoreLogger()->warn(__VA_ARGS__)
#define TOSHI_ERROR(...) Toshi::TLog::GetCoreLogger()->error(__VA_ARGS__)
#define TOSHI_TRACE(...) Toshi::TLog::GetCoreLogger()->trace(__VA_ARGS__)
#define TOSHI_CRITICAL(...) Toshi::TLog::GetCoreLogger()->critical(__VA_ARGS__)

#else

// App log macros
#define TOSHI_INFO(...) Toshi::TLog::GetAppLogger()->info(__VA_ARGS__)
#define TOSHI_WARN(...) Toshi::TLog::GetAppLogger()->warn(__VA_ARGS__)
#define TOSHI_ERROR(...) Toshi::TLog::GetAppLogger()->error(__VA_ARGS__)
#define TOSHI_TRACE(...) Toshi::TLog::GetAppLogger()->trace(__VA_ARGS__)
#define TOSHI_CRITICAL(...) Toshi::TLog::GetAppLogger()->critical(__VA_ARGS__)

#endif

#else

#define TOSHI_INFO(...)
#define TOSHI_WARN(...)
#define TOSHI_ERROR(...)
#define TOSHI_TRACE(...)
#define TOSHI_CRITICAL(...)

#endif