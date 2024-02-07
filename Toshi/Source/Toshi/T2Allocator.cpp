#include "ToshiPCH.h"
#include "T2Allocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
	T2GlobalAllocator T2Allocator::s_GlobalAllocator;

	void* T2GlobalAllocator::Malloc(size_t size)
	{
		return TMalloc(size);
	}

	void* T2GlobalAllocator::Malloc(size_t size, size_t alignment)
	{
		return TMemalign(size, alignment);
	}

	void T2GlobalAllocator::Free(void* ptr)
	{
		TFree(ptr);
	}

	TBOOL T2GlobalAllocator::CanAllocate(size_t size)
	{
		return TTRUE;
	}

	void* T2GlobalAllocator::TryMalloc(size_t size)
	{
		return TMalloc(size);
	}

	void* T2GlobalAllocator::TryMalloc(size_t size, size_t alignment)
	{
		return TMemalign(size, alignment);
	}

	size_t T2GlobalAllocator::GetUsedSize()
	{
		return 0;
	}

	size_t T2GlobalAllocator::GetCapacity()
	{
		return 0;
	}
}