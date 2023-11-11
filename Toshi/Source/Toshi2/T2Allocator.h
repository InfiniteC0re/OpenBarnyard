#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi
{
	class T2Allocator
	{
	public:
		virtual void* Malloc(size_t size, size_t alignment) = 0;
		virtual void* Malloc(size_t size) = 0;
		virtual void Free(void* ptr) = 0;
		virtual TBOOL CanAllocate(size_t size) = 0;
		virtual void* TryMalloc(size_t size, size_t alignment) = 0;
		virtual void* TryMalloc(size_t size) = 0;
		virtual size_t GetUsedSize() = 0;
		virtual size_t GetCapacity() = 0;

		template<class T, class... Args>
		T* New(Args&& ...args)
		{
			void* mem = Malloc(sizeof(T));
			T* object = new (mem) T(std::forward<Args>(args)...);
			return object;
		}

		template<class T>
		void Delete(T* ptr)
		{
			ptr->~T();
			Free(ptr);
		}

	public:
		static class T2GlobalAllocator s_GlobalAllocator;
	};

	class T2GlobalAllocator : public T2Allocator
	{
	public:
		virtual void* Malloc(size_t size, size_t alignment) override;
		virtual void* Malloc(size_t size) override;
		virtual void Free(void* ptr) override;
		virtual TBOOL CanAllocate(size_t size) override;
		virtual void* TryMalloc(size_t size, size_t alignment) override;
		virtual void* TryMalloc(size_t size) override;
		virtual size_t GetUsedSize() override;
		virtual size_t GetCapacity() override;
	};
}