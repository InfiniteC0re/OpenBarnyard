#include "ToshiPCH.h"
#include "T2Allocator.h"

namespace Toshi
{
	T2GlobalAllocator T2Allocator::s_GlobalAllocator;

	void* T2GlobalAllocator::Malloc(size_t size)
	{
		return TMalloc(size);
	}

	void* T2GlobalAllocator::Malloc(size_t size, size_t alignment)
	{
		return TMemalign(alignment, size);
	}

	void T2GlobalAllocator::Free(void* ptr)
	{
		TFree(ptr);
	}

	bool T2GlobalAllocator::CanAllocate(int unk)
	{
		return true;
	}

	void* T2GlobalAllocator::TryMalloc(size_t size)
	{
		return TMalloc(size);
	}

	void* T2GlobalAllocator::TryMalloc(size_t size, size_t alignment)
	{
		return TMemalign(alignment, size);
	}

	size_t T2GlobalAllocator::Size()
	{
		return 0;
	}

	size_t T2GlobalAllocator::Capacity()
	{
		return 0;
	}
}