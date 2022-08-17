#pragma once
#include <memory>

namespace Toshi
{
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
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
	#define TIMPLEMENT(FUNC) { TOSHI_ERROR("{0} is not implemented", __FUNCTION__); }
	#define TASSERT(x, ...) { if (!(x)) { TOSHI_CORE_ERROR(__VA_ARGS__); TBREAK(); } }
#else
	#define TIMPLEMENT(FUNC)
	#define TASSERT(x, ...)
#endif