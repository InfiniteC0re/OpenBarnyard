#pragma once
#include "Toshi/Core/TMemory.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>

namespace Toshi
{
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T>
	using Scope = std::unique_ptr<T, Toshi::TMemoryDeleter>;

	template <typename T>
	using TVector = std::vector<T, Toshi::TAllocator<T>>;

	template <typename T>
	using TMap = std::map<T, Toshi::TAllocator<T>>;

	template <typename T>
	using TUnorderedMap = std::unordered_map<T, Toshi::TAllocator<T>>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::allocate_shared<T>(Toshi::TAllocator<T>(), std::forward<Args>(args)...);
	}

	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		Toshi::TAllocator<T> allocator;
		return Scope<T>(allocator.create(std::forward<Args>(args)...));
	}
}

#define MKVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16) | VER_MINOR)
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