#pragma once
#include "Toshi/Core/Core.h"
#include "Toshi2/T2Mutex.h"

namespace Toshi
{
	class TMemoryHeap;

	struct TMemoryContext
	{
		typedef void* (*t_Malloc)(size_t size);
		typedef void* (*t_Calloc)(size_t nitems, size_t size);
		typedef void* (*t_Realloc)(void* ptr, size_t size);
		typedef void  (*t_Idk)(void* ptr, size_t size);
		typedef void* (*t_Memalign)(size_t alignment, size_t size);
		typedef void  (*t_Free)(void* ptr);

		t_Malloc s_cbMalloc;
		t_Calloc s_cbCalloc;
		t_Realloc s_cbRealloc;
		t_Idk s_cbIdk;           // this one is unused and I don't know what it is
		t_Memalign s_cbMemalign;
		t_Free s_cbFree;

		void* s_Sysheap;
		void* s_Heap;
	};

	class TMemory
	{
	public:
		typedef uint32_t Flags;
		typedef uint32_t Error;
		typedef uint32_t BlockSize;

		enum Flags_ : Flags
		{
			Flags_Standard = 0,
			Flags_NativeMethods = BITFIELD(1)
		};

		enum Error_ : Error
		{
			Error_Ok = 0,
			Error_Heap = 1
		};

	public:
		TMemory(Flags flags = Flags_Standard, BlockSize blockSize = 640 * 1024 * 1024) :
			m_Flags(flags), m_GlobalSize(blockSize) {  }

		static void         dlheapfree(TMemoryHeap* heap, void* mem);
		static void         dlheapdestroy(TMemoryHeap* heap);
		static void*        dlheapmalloc(TMemoryHeap* heap, size_t size);
		static void*        dlheapcalloc(TMemoryHeap* heap, size_t nitems, size_t size);
		static void*        dlheaprealloc(TMemoryHeap* heap, void* mem, size_t newsize);
		static void*        dlheapmemalign(TMemoryHeap* heap, size_t alignment, size_t size);
		static TMemoryHeap* dlheapcreateinplace(void* ptr, size_t heapSize, Flags flags, const char name[15]);
		static TMemoryHeap* dlheapcreatesubheap(TMemoryHeap* heap, size_t size, Flags flags, const char name[15]);
		static TMemoryHeap* dlheapcreate(TMemoryHeap* heap, size_t size, Flags flags, const char name[15]) { return TMemory::dlheapcreatesubheap(heap, size, flags, name); }

		static TMemoryHeap* CreateHeapInPlace(void* ptr, size_t heapSize, Flags flags, const char name[15]) { return TMemory::dlheapcreateinplace(ptr, heapSize, flags, name); }
		static void         CreateHeap(size_t size, Flags flags, const char name[15]) { TMemory::dlheapcreate(s_GlobalHeap, size, flags, name); }
		static void         DestroyHeap(TMemoryHeap* heap) { TMemory::dlheapdestroy(heap); }

		static void         OutOfMem(TMemoryHeap* heap, size_t size);
		static void         Shutdown();

		static TMemoryHeap* GetGlobalHeap() { return s_GlobalHeap; }
		static void         AcquireMutex()          { TMemory::s_GlobalMutex.Lock(); }
		static void         ReleaseMutex()          { TMemory::s_GlobalMutex.Unlock(); }

	public:
		/*
		* Platform specific methods
		* Define them in TMemory_{Platform}.cpp
		*/

		Error Init();

	public:
		static TMemoryContext s_Context;

	private:
		static TMemoryHeap* s_GlobalHeap;
		static T2Mutex s_GlobalMutex;

	private:
		Flags m_Flags;
		BlockSize m_GlobalSize;
	};

	class TMemoryHeap
	{
	public:
		typedef uint32_t Flags;
		enum Flags_ : Flags
		{
			Flags_UseMutex = BITFIELD(0),
			Flags_AllocAsPile = BITFIELD(2),
		};

		friend class TMemory;

	public:
		void  CreateMutex() { m_Mutex.Create(); }
		void  DestroyMutex() { m_Mutex.Destroy(); }

		void* Malloc(size_t size) { return TMemory::dlheapmalloc(this, size); }
		static void* Malloc(TMemoryHeap* heap, size_t size) { return TMemory::dlheapmalloc(heap, size); }
		void* Calloc(size_t nitems, size_t size) { return TMemory::dlheapcalloc(this, nitems, size); }
		void* Realloc(void* mem, size_t newsize) { return TMemory::dlheaprealloc(this, mem, newsize); }
		void* Memalign(size_t alignment, size_t size) { return TMemory::dlheapmemalign(this, alignment, size); }
		void  Free(void* mem) { TMemory::dlheapfree(this, mem); }

	private:
		Flags m_Flags;
		T2Mutex m_Mutex;
		void* m_SubHeapBuffer;
		void* m_MSpace;
		uint32_t m_Unk4;
		uint32_t m_Unk5;
		char m_Name[15];
	};
}

inline static void* TMalloc(size_t size)
{
	return Toshi::TMemory::s_Context.s_cbMalloc(size);
}

inline static void* TCalloc(size_t nitems, size_t size)
{
	return Toshi::TMemory::s_Context.s_cbCalloc(nitems, size);
}

inline static void* TRealloc(void* mem, size_t newsize)
{
	return Toshi::TMemory::s_Context.s_cbRealloc(mem, newsize);
}

inline static void* TMemalign(size_t alignment, size_t size)
{
	return Toshi::TMemory::s_Context.s_cbMemalign(alignment, size);
}

inline static void TFree(void* mem)
{
	Toshi::TMemory::s_Context.s_cbFree(mem);
}

inline void* operator new(size_t size)
{
	return TMalloc(size);
}

inline void* operator new[](size_t size)
{
	return TMalloc(size);
}

inline void* operator new[](size_t size, Toshi::TMemoryHeap* heap)
{
	return Toshi::TMemoryHeap::Malloc(heap, size);
}

inline void operator delete(void* ptr)
{
	return TFree(ptr);
}