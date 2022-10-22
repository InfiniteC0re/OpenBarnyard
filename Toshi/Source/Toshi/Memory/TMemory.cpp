#include "ToshiPCH.h"
#include "dlmalloc.h"
#include "TMemory.h"

#ifdef CreateMutex
#undef CreateMutex
#endif

#ifdef FillMemory
#undef FillMemory
#endif

namespace Toshi
{
	TMemoryContext TMemory::s_Context;
	TMemoryHeap* TMemory::s_GlobalHeap         = nullptr;
	T2Mutex TMemory::s_GlobalMutex             = T2Mutex();

	void TMemory::FillMemory(void* ptr, size_t value, size_t size)
	{
		if (size >= sizeof(void*))
		{
			size_t* pos = static_cast<size_t*>(ptr);
			size_t  stepCount = size / sizeof(void*);
			size -= stepCount * sizeof(void*);

			for (; stepCount != 0; stepCount--) *(pos++) = value;

			ptr = pos;
		}

		while (size > 0)
		{
			uint8_t stepSize = size & 0b11;
			if (stepSize == 0) stepSize = 4;

			if (size == 1)
			{
				*(uint8_t*)ptr = (uint8_t)value;
			}
			else
			{
				if (size == 2)
				{
					*(uint16_t*)ptr = (uint16_t)value;
				}
				else if (size == 3)
				{
					*(uint16_t*)ptr = (uint16_t)value;
					*(uint8_t*)((uintptr_t)ptr + sizeof(uint16_t)) = (uint8_t)value;
				}
				else if (size == 4)
				{
					*(uint32_t*)ptr = (uint32_t)value;
				}
			}

			ptr = reinterpret_cast<void*>((uintptr_t)ptr + stepSize);
			size -= stepSize;
		}
	}

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
		if (heap->m_Flags & TMemoryHeap::Flags_AllocAsPile)
		{
			TTODO("Toshi::TMemoryHeap::AllocAsPile(heap, size, 4) - 0x006fc8b0");
			return heap;
		}

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_malloc(heap->m_MSpace, size);
		if (chunk == nullptr) TMemory::OutOfMem(heap, size);

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void* TMemory::dlheapcalloc(TMemoryHeap* heap, size_t nitems, size_t size)
	{
		if (heap->m_Flags & TMemoryHeap::Flags_AllocAsPile)
		{
			TTODO("Toshi::TMemoryHeap::AllocAsPile(heap, size, 4) - 0x006fc8b0");
			return heap;
		}

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_calloc(heap->m_MSpace, nitems, size);
		if (chunk == nullptr) TMemory::OutOfMem(heap, size);

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void* TMemory::dlheapmemalign(TMemoryHeap* heap, size_t alignment, size_t size)
	{
		if (heap->m_Flags & TMemoryHeap::Flags_AllocAsPile)
		{
			TTODO("Toshi::TMemoryHeap::AllocAsPile(heap, size, alignment) - 0x006fc8b0");
			return heap;
		}

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_memalign(heap->m_MSpace, alignment, size);
		if (chunk == nullptr) TMemory::OutOfMem(heap, size);

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void* TMemory::dlheaprealloc(TMemoryHeap* heap, void* mem, size_t newsize)
	{
		if (heap->m_Flags & TMemoryHeap::Flags_AllocAsPile)
		{
			TTODO("Toshi::TMemoryHeap::AllocAsPile(heap, size, 4) - 0x006fc8b0");
			return heap;
		}

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::AcquireMutex();

		void* chunk = mspace_realloc(heap->m_MSpace, mem, newsize);
		if (chunk == nullptr) TMemory::OutOfMem(heap, newsize);

		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::ReleaseMutex();
		return chunk;
	}

	void TMemory::dlheapfree(TMemoryHeap* heap, void* mem)
	{
		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::AcquireMutex();
		
		mspace_free(heap->m_MSpace, mem);
		
		if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) TMemory::ReleaseMutex();
	}

	void TMemory::dlheapdestroy(TMemoryHeap* heap)
	{
		if (heap != nullptr)
		{
			if (heap->m_Flags & TMemoryHeap::Flags_UseMutex) heap->DestroyMutex();
			destroy_mspace(heap->m_MSpace);
			heap->m_MSpace = nullptr;

			if (heap->m_SubHeapBuffer != nullptr)
			{
				TFree(heap->m_SubHeapBuffer);
				heap->m_SubHeapBuffer = nullptr;
			}
		}
	}

	TMemoryHeap* TMemory::dlheapcreatesubheap(TMemoryHeap* heap, size_t size, Flags flags, const char name[15])
	{
		TMemoryHeap* subHeap = nullptr;

		size_t subHeapSize = size + sizeof(TMemoryHeap);
		void* mem = heap->Malloc(subHeapSize);

		if (mem != nullptr)
		{
			subHeap = TMemory::dlheapcreateinplace(mem, subHeapSize, flags, name);

			if (subHeap == nullptr)
			{
				TFree(mem);
			}
			else
			{
				subHeap->m_SubHeapBuffer = mem;
			}
		}
		
		return subHeap;
	}

	TMemoryHeap* TMemory::dlheapcreateinplace(void* ptr, size_t heapSize, TMemoryHeap::Flags flags, const char name[15])
	{
		TASSERT(heapSize > 0, "Allocation size is zero");
		TASSERT((heapSize & 3) == 0, "Allocation size is not aligned to 4");
		
		TMemoryHeap* heap = static_cast<TMemoryHeap*>(ptr);
		
		size_t capacity = heapSize - sizeof(TMemoryHeap);
		heap->m_MSpace = create_mspace_with_base(heap + 1, capacity, 0);

		if (heap->m_MSpace != nullptr)
		{
			heap->m_Flags = flags;
			strncpy_s(heap->m_Name, name, sizeof(heap->m_Name));

			if (flags & TMemoryHeap::Flags_UseMutex)
			{
				heap->CreateMutex();
			}
			else
			{
				FillMemory(&heap->m_Mutex, 0, sizeof(heap->m_Mutex));
			}

			return heap;
		}
		else
		{
			return nullptr;
		}
	}
}