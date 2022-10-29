#pragma once

namespace Toshi
{
	class T2GlobalAllocator
	{
	public:
		virtual void* Malloc(size_t size);
		virtual void* Malloc(size_t size, size_t alignment);
		virtual void Free(void* ptr);
		virtual bool CanAllocate(int unk);
		virtual void* TryMalloc(size_t size);
		virtual void* TryMalloc(size_t size, size_t alignment);
		virtual size_t Size();
		virtual size_t Capacity();
	};

	class T2Allocator
	{
	public:
		static T2GlobalAllocator s_GlobalAllocator;
	};
}