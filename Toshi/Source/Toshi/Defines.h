#pragma once

#define ISZERO(X) ((X) == 0)
#define HASFLAG(X) (!ISZERO(X))
#define STRINGIFY8(X) #X
#define STRINGIFY16(x) L##x
#define STRINGIFY(X) STRINGIFY8(X)
#define BITFLAG(x) (1U << (x))

#define TVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16) | VER_MINOR)

#define RETURN_ARG_COUNT(_1_, _2_, _3_, _4_, _5_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX5(...) EXPAND_ARGS((__VA_ARGS__, 5, 4, 3, 2, 1, 0))

#define OVERLOAD_MACRO2(name, count) name##count
#define OVERLOAD_MACRO1(name, count) OVERLOAD_MACRO2(name, count)
#define OVERLOAD_MACRO(name, count) OVERLOAD_MACRO1(name, count)

#define GLUE(x, y) x y
#define CALL_OVERLOAD(name, ...) GLUE(OVERLOAD_MACRO(name, COUNT_ARGS_MAX5(__VA_ARGS__)), (__VA_ARGS__))

#define TOSHI_NAMESPACE Toshi
#define TOSHI_NAMESPACE_USING using namespace TOSHI_NAMESPACE;
#define TOSHI_NAMESPACE_START namespace TOSHI_NAMESPACE {
#define TOSHI_NAMESPACE_END }

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
#define TSTATICCAST(POINTERTYPE, VALUE) (static_cast<POINTERTYPE*>(VALUE))
#define TARRAYSIZE(ARRAY) (sizeof(ARRAY) / sizeof(*ARRAY))
#define TALIGNAS(VALUE) alignas(VALUE)
#define TINLINE inline
#define TFORCEINLINE __forceinline

#ifdef TOSHI_ENABLE_ASSERTS
	#define TFIREFLAG static TBOOL FIREFLAG = TFALSE; if (!FIREFLAG)
	#define TWIP() { TFIREFLAG { TERROR("Work in progress: '%s', at line %d\n", __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
	#define TTODO(DESC) { TFIREFLAG { TERROR("TODO: %s ('%s', at line %d)\n", DESC, __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
	#define TFIXME(DESC) { TFIREFLAG { TWARN("FIXME: %s ('%s', at line %d)\n", DESC, __FUNCTION__, __LINE__); FIREFLAG = TTRUE; } }
	#define TIMPLEMENT() { TFIREFLAG { TERROR("%s is not implemented\n", __FUNCTION__); FIREFLAG = TTRUE; } }
	#define TIMPLEMENT_D(DESC) { TFIREFLAG { TERROR("%s is not implemented: %s\n", __FUNCTION__, DESC); FIREFLAG = TTRUE; } }
	#define TASSERT(X, ...) { TFIREFLAG if (!(X)) { if (1 == TDebug_AssertHandler(#X, __FILE__, __LINE__, __VA_ARGS__)) TBREAK(); FIREFLAG = TTRUE; } }
	#define TASSERT_NO_CHECK(X, ...) TDebug_AssertHandler(#X, __FILE__, __LINE__, __VA_ARGS__))
	#define TVALIDPTR(PTR) TASSERT(TDebug_IsValidAddress(PTR))
#else // TOSHI_ENABLE_ASSERTS
	#define TWIP()
	#define TTODO(DESC)
	#define TFIXME(DESC)
	#define TIMPLEMENT()
	#define TIMPLEMENT_D(DESC)
	#define TASSERT(X, ...)
	#define TVALIDPTR(PTR)
#endif // TOSHI_ENABLE_ASSERTS

#define TUNREACHABLE() { TASSERT( TFALSE, "Unreachable code is reached!" ); __assume(0); }

#ifndef TOSHI_NO_LOGS
	#define TINFO(...) Toshi::TUtil::Log(Toshi::TUtil::LogType_Info, __VA_ARGS__)
	#define TWARN(...) Toshi::TUtil::Log(Toshi::TUtil::LogType_Warning, __VA_ARGS__)
	#define TERROR(...) Toshi::TUtil::Log(Toshi::TUtil::LogType_Error, __VA_ARGS__)
	#define TTRACE(...) TDebug_FinalPrintf(__VA_ARGS__)
#else // TOSHI_NO_LOGS
	#define TINFO(...)
	#define TWARN(...)
	#define TERROR(...)
	#define TDEBUG(...)
#endif // TOSHI_NO_LOGS
