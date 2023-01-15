#include "ToshiPCH.h"
#include "dlmalloc.h"
#include "TMemory.h"

#ifdef CreateMutex
#undef CreateMutex
#endif

#ifdef FillMemory
#undef FillMemory
#endif

void* __CRTDECL operator new(size_t size)
{
	return TMalloc(size);
}

void* __CRTDECL operator new(size_t size, Toshi::TMemoryHeap* heap) noexcept
{
	return Toshi::TMemoryHeap::Malloc(heap, size);
}

void* __CRTDECL operator new[](size_t size)
{
	return TMalloc(size);
}

void* __CRTDECL operator new[](size_t size, ::std::nothrow_t const&) noexcept
{
	return TMalloc(size);
}

void* __CRTDECL operator new[](size_t size, Toshi::TMemoryHeap* heap) noexcept
{
	return Toshi::TMemoryHeap::Malloc(heap, size);
}

void __CRTDECL operator delete(void* ptr, ::std::nothrow_t const&) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete(void* ptr) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete[](void* ptr) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete[](void* ptr, ::std::nothrow_t const&) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete[](void* ptr, size_t _Size) noexcept
{
	TFree(ptr);
}

namespace Toshi
{
	TMemoryContext TMemory::s_Context;
	TMemoryHeap* TMemory::s_GlobalHeap         = TNULL;
	T2Mutex TMemory::s_GlobalMutex             = T2Mutex();
	TMemory* TMemory::s_Instance               = TNULL;

	void TMemory::OutOfMem(TMemoryHeap* heap, size_t size)
	{
		// 006fc7c0
		TOSHI_CORE_CRITICAL("=========================================================================");
		TOSHI_CORE_CRITICAL("MEMORY ALLOCATION FAILED");
		TOSHI_CORE_CRITICAL("------------------------");
		TOSHI_CORE_CRITICAL("HEAP: {0}", heap->m_Name);
		TOSHI_CORE_CRITICAL("------------------------");
		TOSHI_CORE_CRITICAL("failed to allocate {0} bytes (~{1}KB, ~{2}MB)", size, size >> 10, size >> 20);
		// FUN_006fbec0(local_e8);
		TOSHI_CORE_CRITICAL("=========================================================================");
		// ...
	}

	void* TMemory::dlheapmalloc(TMemoryHeap* heap, size_t size)
	{
		// 006fc520
		if (heap->m_Flags & TMemoryHeapFlags_AllocAsPile)
		{

			return TMemoryHeap::AllocAsPile(heap, size, 4);
		}

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_malloc(heap->m_MSpace, size);
		if (chunk == nullptr) TMemory::OutOfMem(heap, size);

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void* TMemory::dlheapcalloc(TMemoryHeap* heap, size_t nitems, size_t size)
	{
		// 006fc580
		if (heap->m_Flags & TMemoryHeapFlags_AllocAsPile)
		{
			return TMemoryHeap::AllocAsPile(heap, nitems * size, 4);
		}

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_calloc(heap->m_MSpace, nitems, size);
		if (chunk == nullptr) TMemory::OutOfMem(heap, size);

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void* TMemory::dlheapmemalign(TMemoryHeap* heap, size_t alignment, size_t size)
	{
		// 006fc6f0
		if (heap->m_Flags & TMemoryHeapFlags_AllocAsPile)
		{
			return TMemoryHeap::AllocAsPile(heap, size, 4);
		}

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_memalign(heap->m_MSpace, alignment, size);
		if (chunk == nullptr) TMemory::OutOfMem(heap, size);

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void* TMemory::dlheaprealloc(TMemoryHeap* heap, void* mem, size_t newsize)
	{
		// 006fc5f0
		TASSERT((heap->m_Flags & TMemoryHeapFlags_AllocAsPile) == 0, "Cannot realloc on pile");

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_realloc(heap->m_MSpace, mem, newsize);
		if (chunk == nullptr) TMemory::OutOfMem(heap, newsize);

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void TMemory::dlheapfree(TMemoryHeap* heap, void* mem)
	{
		TASSERT((heap->m_Flags & TMemoryHeapFlags_AllocAsPile) == 0, "Cannot free pile memory");

		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::AcquireMutex();
		
		mspace_free(heap->m_MSpace, mem);
		
		if (heap->m_Flags & TMemoryHeapFlags_UseMutex) TMemory::ReleaseMutex();
	}

	void TMemory::dlheapdestroy(TMemoryHeap* heap)
	{
		if (heap != nullptr)
		{
			if (heap->m_Flags & TMemoryHeapFlags_UseMutex) heap->DestroyMutex();
			destroy_mspace(heap->m_MSpace);
			heap->m_MSpace = nullptr;

			if (heap->m_SubHeapBuffer != nullptr)
			{
				TFree(heap->m_SubHeapBuffer);
				heap->m_SubHeapBuffer = nullptr;
			}
		}
	}

	TMemoryHeap* TMemory::dlheapcreatesubheap(TMemoryHeap* heap, size_t size, TMemoryHeapFlags flags, const char name[HEAP_MAXNAME])
	{
		// 006fc320
		TMemoryHeap* subHeap = nullptr;
		size_t subHeapSize = size + sizeof(TMemoryHeap);
		void* mem = heap->Malloc(subHeapSize);

		if (mem != nullptr)
		{	
			if (flags & TMemoryHeapFlags_AllocAsPile)
			{
				subHeap = static_cast<TMemoryHeap*>(mem);

				subHeap->m_Flags = flags;
				subHeap->m_MSpace = TNULL;
				subHeap->m_PileData = reinterpret_cast<char*>(subHeap + 1);
				subHeap->m_PileSize = subHeapSize;
				subHeap->SetName(name);

				if (flags & TMemoryHeapFlags_UseMutex)
				{
					subHeap->CreateMutex();
				}
				else
				{
					TUtil::MemSet(&subHeap->m_Mutex, 0, sizeof(subHeap->m_Mutex));
				}
			}
			else
			{
				subHeap = TMemory::CreateHeapInPlace(mem, subHeapSize, flags, name);
			}
			

			if (subHeap != TNULL)
			{
				subHeap->m_SubHeapBuffer = static_cast<char*>(mem);
			}
			else
			{
				TFree(mem);
			}
		}
		
		return subHeap;
	}

	TMemoryHeap* TMemory::dlheapcreateinplace(void* ptr, size_t heapSize, TMemoryHeapFlags flags, const char name[HEAP_MAXNAME])
	{
		TASSERT(heapSize > 0, "Allocation size is zero");
		TASSERT((heapSize & 3) == 0, "Allocation size is not aligned to 4");
		
		TMemoryHeap* heap = static_cast<TMemoryHeap*>(ptr);
		
		size_t capacity = heapSize - sizeof(TMemoryHeap);
		heap->m_MSpace = create_mspace_with_base(heap + 1, capacity, 1);

		if (heap->m_MSpace != nullptr)
		{
			heap->m_Flags = flags;
			heap->SetName(name);

			if (flags & TMemoryHeapFlags_UseMutex)
			{
				heap->CreateMutex();
			}
			else
			{
				TUtil::MemSet(&heap->m_Mutex, 0, sizeof(heap->m_Mutex));
			}

			return heap;
		}
		else
		{
			return nullptr;
		}
	}

	void* TMemoryHeap::AllocAsPile(TMemoryHeap* heap, size_t size, size_t alignment)
	{
		TASSERT(heap->m_Flags & TMemoryHeapFlags_AllocAsPile, "Can't allocate as pile on a non-pile heap");

		// Align current pointer
		heap->m_PileData = reinterpret_cast<char*>((((size_t)heap->m_PileData + (alignment - 1)) / alignment) * alignment);
		size_t usedMemorySize = heap->m_PileData - heap->m_SubHeapBuffer;

		if (usedMemorySize + size > heap->m_PileSize)
		{
			TOSHI_CORE_CRITICAL("Out of memory in pile '{0}' when trying to alloc {1} bytes", heap->m_Name, size);
			TBREAK();
		}

		void* allocated = heap->m_PileData;
		heap->m_PileData += size;

		return allocated;
	}
}