#include "ToshiPCH.h"
#include "Core/TMemory_dlmalloc.h"
#include "Core/dlmalloc.h"


/* ------------------- TSIZE and alignment properties -------------------- */

/* The byte and bit size of a TSIZE */
#define SIZE_T_SIZE         (sizeof(TSIZE))
#define SIZE_T_BITSIZE      (sizeof(TSIZE) << 3)

/* Some constants coerced to TSIZE */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO         ((TSIZE)0)
#define SIZE_T_ONE          ((TSIZE)1)
#define SIZE_T_TWO          ((TSIZE)2)
#define SIZE_T_FOUR         ((TSIZE)4)
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define FOUR_SIZE_T_SIZES   (SIZE_T_SIZE<<2)
#define SIX_SIZE_T_SIZES    (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T     (MAX_SIZE_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A)       (((TSIZE)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)\
 ((((TSIZE)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((TSIZE)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

struct malloc_chunk {
    TSIZE               prev_foot;  /* Size of previous chunk (if free).  */
    TSIZE               head;       /* Size and inuse bits. */
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

namespace Toshi {

    void TMemoryDL::Shutdown()
    {
        dlheapdestroy( g_pMemoryDL->m_GlobalHeap );
        g_pMemoryDL->m_GlobalHeap = TNULL;

        if ( g_pMemoryDL->m_Context.s_Heap )
        {
            HeapFree( g_pMemoryDL->m_Context.s_Sysheap, NULL, g_pMemoryDL->m_Context.s_Heap );
        }

        if ( TMemoryDL::GetFlags() & Flags_Standard )
        {
            // Reset callbacks so no external libraries can malloc or free anything
            g_pMemoryDL->m_Context.s_cbMalloc = []( TMemoryDL* pMemModule, TSIZE size ) -> void* { return TNULL; };
            g_pMemoryDL->m_Context.s_cbCalloc = []( TMemoryDL* pMemModule, TSIZE nitems, TSIZE size ) -> void* { return TNULL; };
            g_pMemoryDL->m_Context.s_cbRealloc = []( TMemoryDL* pMemModule, void* ptr, TSIZE size ) -> void* { return TNULL; };
            g_pMemoryDL->m_Context.s_cbMemalign = []( TMemoryDL* pMemModule, TSIZE alignment, TSIZE size ) -> void* { return TNULL; };
            g_pMemoryDL->m_Context.s_cbFree = []( TMemoryDL* pMemModule, void* ptr ) -> void { };
            g_pMemoryDL->m_Context.s_cbIdk = []( TMemoryDL* pMemModule, void* ptr, TSIZE size ) -> void { };
        }

        TUtil::MemSet( &g_pMemoryDL->m_Context, 0, sizeof( m_Context ) );
    }

    TMemoryDL::Error TMemoryDL::Init()
    {
        // 0x006fb9d0
        TASSERT( m_Context.s_Sysheap == NULL, "TMemoryDL is already initialized" );

        m_Context.m_pMemModule = this;
        m_Context.s_Sysheap = GetProcessHeap();
        m_Mutex.Create();

        if ( m_Context.s_Sysheap == NULL )
        {
            return Error_Heap;
        }

        // Check if we should use default memory management methods
        if ( m_Flags & Flags_NativeMethods )
        {
            m_Context.s_cbMalloc = TMemoryDLContext::MallocNative;
            m_Context.s_cbCalloc = TMemoryDLContext::CallocNative;
            m_Context.s_cbRealloc = TMemoryDLContext::ReallocNative;
            m_Context.s_cbMemalign = TMemoryDLContext::MemalignNative;
            m_Context.s_cbFree = TMemoryDLContext::FreeNative;
            m_Context.s_cbIdk = TMemoryDLContext::IdkNative;

            return Error_Ok;
        }

        // Allocate memory for the heap
        m_Context.s_Heap = HeapAlloc( m_Context.s_Sysheap, HEAP_ZERO_MEMORY, m_GlobalSize );

        // Save pointers to our own functions
        if ( m_Context.s_Heap == NULL )
        {
            return Error_Heap;
        }

        m_Context.s_cbMalloc = []( TMemoryDL* pMemModule, TSIZE size ) -> void*
        {
            return pMemModule->dlheapmalloc( pMemModule->GetHeap(), size );
        };

        m_Context.s_cbCalloc = []( TMemoryDL* pMemModule, TSIZE nitems, TSIZE size ) -> void*
        {
            return pMemModule->dlheapcalloc( pMemModule->GetHeap(), nitems, size );
        };

        m_Context.s_cbRealloc = []( TMemoryDL* pMemModule, void* ptr, TSIZE size ) -> void*
        {

            return pMemModule->dlheaprealloc( pMemModule->GetHeap(), ptr, size );
        };

        m_Context.s_cbMemalign = []( TMemoryDL* pMemModule, TSIZE alignment, TSIZE size ) -> void*
        {
            return pMemModule->dlheapmemalign( pMemModule->GetHeap(), alignment, size );
        };

        m_Context.s_cbFree = []( TMemoryDL* pMemModule, void* ptr ) -> void
        {
            pMemModule->dlheapfree( pMemModule->GetHeap(), ptr );
        };

        m_Context.s_cbIdk = []( TMemoryDL* pMemModule, void* ptr, TSIZE size ) -> void
        {

        };

        m_GlobalHeap = CreateHeapInPlace( m_Context.s_Heap, m_GlobalSize, TMemoryHeapFlags_UseMutex, "global" );
        return Error_Ok;
    }
}