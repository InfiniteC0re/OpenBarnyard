#pragma once
#include "Toshi/Memory/TMemory.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>

TOSHI_NAMESPACE_START

namespace STL
{
template <class Type>
class TAllocator
{
public:
	using value_type = Type;

	using pointer		= Type*;
	using const_pointer = const Type*;

	using void_pointer		 = void*;
	using const_void_pointer = const void*;

	using size_type		  = size_t;
	using difference_type = std::ptrdiff_t;

	TAllocator()  = default;
	~TAllocator() = default;

	template <class U>
	TAllocator( const TAllocator<U>& other ) {}

	inline pointer allocate( size_type count )
	{
		return static_cast<pointer>( TMalloc( sizeof( value_type ) * count ) );
	}

	inline pointer allocate( size_type count, const_void_pointer hint )
	{
		return allocate( count );
	}

	inline void deallocate( pointer ptr, size_type count )
	{
		TFree( ptr );
	}

	inline size_type max_size() const
	{
		return std::numeric_limits<size_type>::max();
	}

	template <class U, class... Args>
	inline void construct( U* ptr, Args&&... args )
	{
		new ( ptr ) U( std::forward<Args>( args )... );
	}

	template <class U>
	inline void destroy( U* ptr )
	{
		ptr->~U();
	}
};

template <class Type>
class MemoryDeleter
{
public:
	inline void operator()( Type* ptr ) { delete ptr; }
};

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using Scope = std::unique_ptr<T, MemoryDeleter<T>>;

template <typename T>
using Vector = std::vector<T, Toshi::STL::TAllocator<T>>;

template <typename Key, typename T>
using Map = std::map<Key, T, std::less<Key>, Toshi::STL::TAllocator<std::pair<const Key, T>>>;

template <typename Key, typename T>
using UnorderedMap = std::unordered_map<Key, T, std::hash<Key>, std::equal_to<Key>, Toshi::STL::TAllocator<std::pair<const Key, T>>>;

template <typename T, typename... Args>
constexpr Ref<T> CreateRef( Args&&... args )
{
	return std::allocate_shared<T>( Toshi::STL::TAllocator<T>(), std::forward<Args>( args )... );
}

template <typename T, typename... Args>
constexpr Scope<T> CreateScope( Args&&... args )
{
	return Scope<T>( new T( std::forward<Args>( args )... ) );
}

template <typename T, typename Result, typename... Args>
constexpr Scope<Result> CreateScope( Args&&... args )
{
	return Scope<Result>( new T( std::forward<Args>( args )... ) );
}
} // namespace STL

TOSHI_NAMESPACE_END
