#include "ToshiPCH.h"
#include "Toshi/Memory/TMemory.h"

#ifdef CreateMutex
#undef CreateMutex
#endif

#ifdef FillMemory
#undef FillMemory
#endif

namespace Toshi
{
	void TMemory::Shutdown()
	{
		DestroyHeap(s_GlobalHeap);
		s_GlobalHeap = nullptr;

		if (TMemory::s_Context.s_Heap)
		{
			HeapFree(TMemory::s_Context.s_Sysheap, NULL, TMemory::s_Context.s_Heap);
		}

		TUtil::MemSet(&TMemory::s_Context, 0, sizeof(TMemory::s_Context));
	}

	TMemory::Error TMemory::Init()
	{
		// 0x006fb9d0
		TASSERT(TMemory::s_Context.s_Sysheap == NULL, "TMemory is already initialized");
		
		TMemory::s_Context.s_Sysheap = GetProcessHeap();
		TMemory::s_GlobalMutex.Create();

		if (TMemory::s_Context.s_Sysheap == NULL)
		{
			return Error_Heap;
		}

		// Check if we should use default memory management methods
		if (m_Flags & Flags_NativeMethods)
		{
			TMemory::s_Context.s_cbMalloc = [](size_t size) -> void*
			{
				return malloc(size);
			};

			TMemory::s_Context.s_cbCalloc = [](size_t nitems, size_t size) -> void*
			{
				return calloc(nitems, size);
			};

			TMemory::s_Context.s_cbRealloc = [](void* ptr, size_t size) -> void*
			{
				return realloc(ptr, size);
			};

			TMemory::s_Context.s_cbMemalign = [](size_t alignment, size_t size) -> void*
			{
				return malloc(size);
			};

			TMemory::s_Context.s_cbFree = [](void* ptr) -> void
			{
				free(ptr);
			};

			TMemory::s_Context.s_cbIdk = [](void* ptr, size_t size) -> void
			{
				
			};

			return Error_Ok;
		}

		// Allocate memory for the heap
		TMemory::s_Context.s_Heap = HeapAlloc(TMemory::s_Context.s_Sysheap, NULL, m_GlobalSize);

		// Save pointers to our own functions
		if (TMemory::s_Context.s_Heap == NULL)
		{
			return Error_Heap;
		}

		TMemory::s_Context.s_cbMalloc = [](size_t size) -> void*
		{
			return TMemory::dlheapmalloc(TMemory::s_GlobalHeap, size);
		};

		TMemory::s_Context.s_cbCalloc = [](size_t nitems, size_t size) -> void*
		{
			return TMemory::dlheapcalloc(TMemory::s_GlobalHeap, nitems, size);
		};

		TMemory::s_Context.s_cbRealloc = [](void* ptr, size_t size) -> void*
		{
			return nullptr;
		};

		TMemory::s_Context.s_cbMemalign = [](size_t alignment, size_t size) -> void*
		{
			return TMemory::dlheapmemalign(TMemory::s_GlobalHeap, alignment, size);
		};

		TMemory::s_Context.s_cbFree = [](void* ptr) -> void
		{
			TMemory::dlheapfree(TMemory::s_GlobalHeap, ptr);
		};

		TMemory::s_Context.s_cbIdk = [](void* ptr, size_t size) -> void
		{

		};

		s_GlobalHeap = CreateHeapInPlace(TMemory::s_Context.s_Heap, m_GlobalSize, TMemoryHeapFlags_UseMutex, "global");
		return Error_Ok;
	}
}