#include "ToshiPCH.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Memory/dlmalloc.h"

#ifdef CreateMutex
#undef CreateMutex
#endif

#ifdef FillMemory
#undef FillMemory
#endif

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#define SIZE_T_SIZE         (sizeof(size_t))
#define SIZE_T_BITSIZE      (sizeof(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO         ((size_t)0)
#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZE_T_FOUR         ((size_t)4)
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define FOUR_SIZE_T_SIZES   (SIZE_T_SIZE<<2)
#define SIX_SIZE_T_SIZES    (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T     (MAX_SIZE_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)\
 ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

struct malloc_chunk {
    size_t               prev_foot;  /* Size of previous chunk (if free).  */
    size_t               head;       /* Size and inuse bits. */
    struct malloc_chunk* fd;         /* double links -- used only if free. */
    struct malloc_chunk* bk;
};

typedef struct malloc_chunk  mchunk;
typedef struct malloc_chunk* mchunkptr;
typedef struct malloc_chunk* sbinptr;  /* The type of bins of chunks */
typedef unsigned int bindex_t;         /* Described below */
typedef unsigned int binmap_t;         /* Described below */
typedef unsigned int flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#define MCHUNK_SIZE         (sizeof(mchunk))

#if FOOTERS
#define CHUNK_OVERHEAD      (TWO_SIZE_T_SIZES)
#else /* FOOTERS */
#define CHUNK_OVERHEAD      (SIZE_T_SIZE)
#endif /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#define MMAP_CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#define MMAP_FOOT_PAD       (FOUR_SIZE_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#define MIN_CHUNK_SIZE\
  ((MCHUNK_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* conversion from malloc headers to user pointers, and back */
#define chunk2mem(p)        ((void*)((char*)(p)       + TWO_SIZE_T_SIZES))
#define mem2chunk(mem)      ((mchunkptr)((char*)(mem) - TWO_SIZE_T_SIZES))
/* chunk associated with aligned address A */
#define align_as_chunk(A)   (mchunkptr)((A) + align_offset(chunk2mem(A)))

/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#define PINUSE_BIT          (SIZE_T_ONE)
#define CINUSE_BIT          (SIZE_T_TWO)
#define FLAG4_BIT           (SIZE_T_FOUR)
#define INUSE_BITS          (PINUSE_BIT|CINUSE_BIT)
#define FLAG_BITS           (PINUSE_BIT|CINUSE_BIT|FLAG4_BIT)

#define chunksize(p)        ((p)->head & ~(FLAG_BITS))

namespace Toshi
{
	void TMemory::Shutdown()
	{
		DestroyHeap(s_GlobalHeap);
		s_GlobalHeap = TNULL;

		if (TMemory::s_Context.s_Heap)
		{
			HeapFree(TMemory::s_Context.s_Sysheap, NULL, TMemory::s_Context.s_Heap);
		}

		if (TMemory::GetFlags() & Flags_Standard)
		{
			// Reset callbacks so no external libraries can malloc or free anything
			TMemory::s_Context.s_cbMalloc = [](size_t size) -> void* { return TNULL; };
			TMemory::s_Context.s_cbCalloc = [](size_t nitems, size_t size) -> void* { return TNULL; };
			TMemory::s_Context.s_cbRealloc = [](void* ptr, size_t size) -> void* { return TNULL; };
			TMemory::s_Context.s_cbMemalign = [](size_t alignment, size_t size) -> void* { return TNULL; };
			TMemory::s_Context.s_cbFree = [](void* ptr) -> void { };
			TMemory::s_Context.s_cbIdk = [](void* ptr, size_t size) -> void { };
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
			void* ptr = TMemory::dlheapmalloc(TMemory::s_GlobalHeap, size);
            mchunkptr chunk = mem2chunk(ptr);
            s_NumAllocatedBytes += chunksize(chunk);
			return ptr;
		};

		TMemory::s_Context.s_cbCalloc = [](size_t nitems, size_t size) -> void*
		{
			void* ptr = TMemory::dlheapcalloc(TMemory::s_GlobalHeap, nitems, size);
            mchunkptr chunk = mem2chunk(ptr);
            s_NumAllocatedBytes += chunksize(chunk);
			return ptr;
		};

		TMemory::s_Context.s_cbRealloc = [](void* ptr, size_t size) -> void*
		{
			return TNULL;
		};

		TMemory::s_Context.s_cbMemalign = [](size_t alignment, size_t size) -> void*
		{
			void* ptr = TMemory::dlheapmemalign(TMemory::s_GlobalHeap, alignment, size);
            mchunkptr chunk = mem2chunk(ptr);
            s_NumAllocatedBytes += chunksize(chunk);
			return ptr;
		};

		TMemory::s_Context.s_cbFree = [](void* ptr) -> void
		{
			if (get_mspace_from_ptr(ptr) == TMemory::s_GlobalHeap->GetMSpace())
			{
                mchunkptr chunk = mem2chunk(ptr);
                s_NumAllocatedBytes -= chunksize(chunk);
			}

			TMemory::dlheapfree(TMemory::s_GlobalHeap, ptr);
		};

		TMemory::s_Context.s_cbIdk = [](void* ptr, size_t size) -> void
		{

		};

		s_GlobalHeap = CreateHeapInPlace(TMemory::s_Context.s_Heap, m_GlobalSize, TMemoryHeapFlags_UseMutex, "global");
		return Error_Ok;
	}
}