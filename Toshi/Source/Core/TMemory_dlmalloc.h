#pragma once
#include "Thread/T2Mutex.h"

#undef CreateMutex
#undef FillMemory

TOSHI_NAMESPACE_START

class TMemoryDLContext
{
private:
    inline static auto MallocNative = []( TSIZE size ) -> void* { return malloc( size ); };
    inline static auto CallocNative = []( TSIZE nitems, TSIZE size ) -> void* { return calloc( nitems, size ); };
    inline static auto ReallocNative = []( void* ptr, TSIZE size ) -> void* { return realloc( ptr, size ); };
    inline static auto MemalignNative = []( TSIZE alignment, TSIZE size ) -> void* { return malloc( size ); };
    inline static auto FreeNative = []( void* ptr ) -> void { free( ptr ); };
    inline static auto IdkNative = []( void* ptr, TSIZE size ) -> void { };

    friend class TMemoryDL;

public:
    typedef void* ( *t_Malloc )( TSIZE size );
    typedef void* ( *t_Calloc )( TSIZE nitems, TSIZE size );
    typedef void* ( *t_Realloc )( void* ptr, TSIZE size );
    typedef void  ( *t_Idk )( void* ptr, TSIZE size );
    typedef void* ( *t_Memalign )( TSIZE alignment, TSIZE size );
    typedef void  ( *t_Free )( void* ptr );

    t_Malloc s_cbMalloc = MallocNative;
    t_Calloc s_cbCalloc = CallocNative;
    t_Realloc s_cbRealloc = ReallocNative;
    t_Idk s_cbIdk = IdkNative;
    t_Memalign s_cbMemalign = MemalignNative;
    t_Free s_cbFree = FreeNative;

    void* s_Sysheap = TNULL;
    void* s_Heap = TNULL;
};

class TMemoryDLHeap;

typedef TUINT32 TMemoryDLHeapFlags;
enum TMemoryHeapFlags_ : TMemoryDLHeapFlags
{
    TMemoryHeapFlags_UseMutex = BITFLAG( 0 ),
    TMemoryHeapFlags_AllocAsPile = BITFLAG( 2 ),
};

class TMemoryDL
{
public:
    static constexpr int HEAP_MAXNAME = 15;

    typedef TUINT32 Flags;
    typedef TUINT32 Error;
    typedef TUINT32 BlockSize;

    enum Flags_ : Flags
    {
        Flags_Standard = 0,
        Flags_NativeMethods = BITFLAG( 1 )
    };

    enum Error_ : Error
    {
        Error_Ok = 0,
        Error_Heap = 1
    };

public:
    TMemoryDL( Flags flags = Flags_Standard, BlockSize blockSize = 640 * 1024 * 1024 ) :
        m_Flags( flags ), m_GlobalSize( blockSize ) {
        s_pSingleton = this;
    }

    static void               dlheapfree( TMemoryDLHeap* heap, void* mem );
    static void               dlheapdestroy( TMemoryDLHeap* heap );
    static void*              dlheapmalloc( TMemoryDLHeap* heap, TSIZE size );
    static void*              dlheapcalloc( TMemoryDLHeap* heap, TSIZE nitems, TSIZE size );
    static void*              dlheaprealloc( TMemoryDLHeap* heap, void* mem, TSIZE newsize );
    static void*              dlheapmemalign( TMemoryDLHeap* heap, TSIZE alignment, TSIZE size );
    static TMemoryDLHeap*     dlheapcreateinplace( void* ptr, TSIZE heapSize, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] );
    static TMemoryDLHeap*     dlheapcreatesubheap( TMemoryDLHeap* heap, TSIZE size, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] );
    static TMemoryDLHeap*     dlheapcreate( TMemoryDLHeap* heap, TSIZE size, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] ) { return TMemoryDL::dlheapcreatesubheap( heap, size, flags, name ); }

    static TMemoryDLHeap*     CreateHeapInPlace( void* ptr, TSIZE heapSize, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] ) { return TMemoryDL::dlheapcreateinplace( ptr, heapSize, flags, name ); }
    static TMemoryDLHeap*     CreateHeap( TSIZE size, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] ) { return TMemoryDL::dlheapcreate( s_GlobalHeap, size, flags, name ); }
    static void               DestroyHeap( TMemoryDLHeap* heap ) { TMemoryDL::dlheapdestroy( heap ); }

    static void               OutOfMem( TMemoryDLHeap* heap, TSIZE size );
    static void               Shutdown();

	static void               SetGlobalHeap( TMemoryDLHeap* a_pHeap ) { s_GlobalHeap = a_pHeap; }
	static TMemoryDLHeap*     GetGlobalHeap() { return s_GlobalHeap; }
    static void               AcquireMutex() { TMemoryDL::s_GlobalMutex.Lock(); }
    static void               ReleaseMutex() { TMemoryDL::s_GlobalMutex.Unlock(); }
    static unsigned long long GetNumOfAllocatedBytes() { return s_NumAllocatedBytes; }

private:
    static Flags              GetFlags() { return s_pSingleton->m_Flags; }

public:
    /*
    * Platform specific methods
    * Define them in TMemory_{Platform}.cpp
    */

    Error Init();

public:
    inline static TMemoryDLContext s_Context;

private:
    inline static TMemoryDLHeap* s_GlobalHeap;
    inline static T2Mutex s_GlobalMutex;
    inline static TMemoryDL* s_pSingleton;
    inline static unsigned long long s_NumAllocatedBytes;

private:
    Flags m_Flags;
    BlockSize m_GlobalSize;
};

class TMemoryDLHeap
{
public:
    friend class TMemoryDL;

public:
    void* Malloc( TSIZE size ) { return TMemoryDL::dlheapmalloc( this, size ); }
    static void* Malloc( TMemoryDLHeap* heap, TSIZE size ) { return TMemoryDL::dlheapmalloc( heap, size ); }
    void* Calloc( TSIZE nitems, TSIZE size ) { return TMemoryDL::dlheapcalloc( this, nitems, size ); }
    void* Realloc( void* mem, TSIZE newsize ) { return TMemoryDL::dlheaprealloc( this, mem, newsize ); }
    void* Memalign( TSIZE alignment, TSIZE size ) { return TMemoryDL::dlheapmemalign( this, alignment, size ); }
    void  Free( void* mem ) { TMemoryDL::dlheapfree( this, mem ); }

    void SetName( const TCHAR* name )
    {
        strncpy_s( m_Name, name, TMemoryDL::HEAP_MAXNAME );
        m_Name[ TMemoryDL::HEAP_MAXNAME ] = '\0';
    }

    void* GetMSpace() const { return m_MSpace; }

private:
    void  CreateMutex() { m_Mutex.Create(); }
    void  DestroyMutex() { m_Mutex.Destroy(); }

    static void* AllocAsPile( TMemoryDLHeap* heap, TSIZE size, TSIZE alignment = 4 );

private:
    TMemoryDLHeapFlags m_Flags;
    T2Mutex m_Mutex;
    TCHAR* m_SubHeapBuffer;
    void* m_MSpace;
    TCHAR* m_PileData;
    TUINT32 m_PileSize;
    TCHAR m_Name[ TMemoryDL::HEAP_MAXNAME + 1 ];
};

TOSHI_NAMESPACE_END

//inline static void* TMalloc(TSIZE size)
//{
//    return Toshi::TMemoryDL::s_Context.s_cbMalloc(size);
//}
//
//inline static void* TCalloc(TSIZE nitems, TSIZE size)
//{
//    return Toshi::TMemoryDL::s_Context.s_cbCalloc(nitems, size);
//}
//
//inline static void* TRealloc(void* mem, TSIZE newsize)
//{
//    return Toshi::TMemoryDL::s_Context.s_cbRealloc(mem, newsize);
//}
//
//inline static void* TMemalign(TSIZE alignment, TSIZE size)
//{
//    return Toshi::TMemoryDL::s_Context.s_cbMemalign(alignment, size);
//}
//
//inline static void TFree(void* mem)
//{
//    Toshi::TMemoryDL::s_Context.s_cbFree(mem);
//}
//
//inline void* __CRTDECL operator new(size_t size, Toshi::TMemoryDLHeap* heap)
//{
//    return Toshi::TMemoryDLHeap::Malloc(heap, size);
//}
//
//inline void* __CRTDECL operator new[]( size_t size, Toshi::TMemoryDLHeap* heap)
//{
//    return Toshi::TMemoryDLHeap::Malloc(heap, size);
//}
//
//inline void __CRTDECL operator delete(void* ptr, Toshi::TMemoryDLHeap* heap) noexcept
//{
//    heap->Free(ptr);
//}
//
//inline void __CRTDECL operator delete[](void* ptr, Toshi::TMemoryDLHeap* heap) noexcept
//{
//    heap->Free(ptr);
//}