#pragma once
#include "Thread/T2Mutex.h"

#undef CreateMutex
#undef FillMemory

TOSHI_NAMESPACE_START

#ifdef TMEMORY_USE_DLMALLOC

class TMemoryDL;

class TMemoryDLContext
{
private:
	inline static auto MallocNative = []( TMemoryDL* pMemModule, TSIZE size ) -> void* {
		return malloc( size );
	};
	inline static auto CallocNative = []( TMemoryDL* pMemModule, TSIZE nitems, TSIZE size ) -> void* {
		return calloc( nitems, size );
	};
	inline static auto ReallocNative = []( TMemoryDL* pMemModule, void* ptr, TSIZE size ) -> void* {
		return realloc( ptr, size );
	};
	inline static auto MemalignNative = []( TMemoryDL* pMemModule, TSIZE alignment, TSIZE size ) -> void* {
		return malloc( size );
	};
	inline static auto FreeNative = []( TMemoryDL* pMemModule, void* ptr ) -> void {
		free( ptr );
	};
	inline static auto IdkNative = []( TMemoryDL* pMemModule, void* ptr, TSIZE size ) -> void {
	};

	friend TMemoryDL;

public:
	typedef void* ( *t_Malloc )( TMemoryDL* pMemModule, TSIZE size );
	typedef void* ( *t_Calloc )( TMemoryDL* pMemModule, TSIZE nitems, TSIZE size );
	typedef void* ( *t_Realloc )( TMemoryDL* pMemModule, void* ptr, TSIZE size );
	typedef void ( *t_Idk )( TMemoryDL* pMemModule, void* ptr, TSIZE size );
	typedef void* ( *t_Memalign )( TMemoryDL* pMemModule, TSIZE alignment, TSIZE size );
	typedef void ( *t_Free )( TMemoryDL* pMemModule, void* ptr );

public:
	void* Malloc( TSIZE size ) { return s_cbMalloc( m_pMemModule, size ); }
	void* Calloc( TSIZE nitems, TSIZE size ) { return s_cbCalloc( m_pMemModule, nitems, size ); }
	void* Realloc( void* ptr, TSIZE size ) { return s_cbRealloc( m_pMemModule, ptr, size ); }
	void* Memalign( TSIZE alignment, TSIZE size ) { return s_cbMemalign( m_pMemModule, alignment, size ); }
	void  Free( void* ptr ) { s_cbFree( m_pMemModule, ptr ); }

public:
	t_Malloc   s_cbMalloc   = MallocNative;
	t_Calloc   s_cbCalloc   = CallocNative;
	t_Realloc  s_cbRealloc  = ReallocNative;
	t_Idk      s_cbIdk      = IdkNative;
	t_Memalign s_cbMemalign = MemalignNative;
	t_Free     s_cbFree     = FreeNative;

	void*      s_Sysheap = TNULL;
	void*      s_Heap    = TNULL;
	TMemoryDL* m_pMemModule;
};

class TMemoryDLHeap;

typedef TUINT32 TMemoryDLHeapFlags;
enum TMemoryHeapFlags_ : TMemoryDLHeapFlags
{
	TMemoryHeapFlags_UseMutex    = BITFLAG( 0 ),
	TMemoryHeapFlags_AllocAsPile = BITFLAG( 2 ),
};

extern class TMemoryDL* g_pMemoryDL;

class TMemoryDL
{
public:
	static constexpr int HEAP_MAXNAME = 51;

	typedef TUINT32 Flags;
	typedef TUINT32 Error;
	typedef TUINT32 BlockSize;

	enum Flags_ : Flags
	{
		Flags_Standard      = 0,
		Flags_NativeMethods = BITFLAG( 1 )
	};

	enum Error_ : Error
	{
		Error_Ok   = 0,
		Error_Heap = 1
	};

public:
	TMemoryDL( Flags flags = Flags_Standard, BlockSize blockSize = 640 * 1024 * 1024 )
	    : m_Flags( flags ), m_GlobalSize( blockSize ), m_NumAllocatedBytes( 0 ) {}

	void              SetHeap( TMemoryDLHeap* a_pHeap ) { m_GlobalHeap = a_pHeap; }
	TMemoryDLHeap*    GetHeap() { return m_GlobalHeap; }
	TMemoryDLContext& GetContext() { return m_Context; }

public:
	void           dlheapfree( TMemoryDLHeap* heap, void* mem );
	void           dlheapdestroy( TMemoryDLHeap* heap );
	void*          dlheapmalloc( TMemoryDLHeap* heap, TSIZE size );
	void*          dlheapcalloc( TMemoryDLHeap* heap, TSIZE nitems, TSIZE size );
	void*          dlheaprealloc( TMemoryDLHeap* heap, void* mem, TSIZE newsize );
	void*          dlheapmemalign( TMemoryDLHeap* heap, TSIZE alignment, TSIZE size );
	TMemoryDLHeap* dlheapcreateinplace( void* ptr, TSIZE heapSize, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] );
	TMemoryDLHeap* dlheapcreatesubheap( TMemoryDLHeap* heap, TSIZE size, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] );
	TMemoryDLHeap* dlheapcreate( TMemoryDLHeap* heap, TSIZE size, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] ) { return TMemoryDL::dlheapcreatesubheap( heap, size, flags, name ); }

	TMemoryDLHeap* CreateHeapInPlace( void* ptr, TSIZE heapSize, TMemoryDLHeapFlags flags, const TCHAR name[ HEAP_MAXNAME ] ) { return TMemoryDL::dlheapcreateinplace( ptr, heapSize, flags, name ); }

	void               OutOfMem( TMemoryDLHeap* heap, TSIZE size );
	void               Shutdown();
	void               AcquireMutex() { m_Mutex.Lock(); }
	void               ReleaseMutex() { m_Mutex.Unlock(); }
	unsigned long long GetNumOfAllocatedBytes() { return m_NumAllocatedBytes; }

private:
	Flags GetFlags() { return m_Flags; }

public:
	/*
    * Platform specific methods
    * Define them in TMemory_{Platform}.cpp
    */

	Error Init();

public:
private:
private:
	Flags              m_Flags;
	BlockSize          m_GlobalSize;
	TMemoryDLHeap*     m_GlobalHeap;
	TMemoryDLContext   m_Context;
	T2Mutex            m_Mutex;
	unsigned long long m_NumAllocatedBytes;
};

class TMemoryDLHeap
{
public:
	friend TMemoryDL;

public:
	void* Malloc( TSIZE size ) { return m_pOwnerBlock->dlheapmalloc( this, size ); }
	void* Calloc( TSIZE nitems, TSIZE size ) { return m_pOwnerBlock->dlheapcalloc( this, nitems, size ); }
	void* Realloc( void* mem, TSIZE newsize ) { return m_pOwnerBlock->dlheaprealloc( this, mem, newsize ); }
	void* Memalign( TSIZE alignment, TSIZE size ) { return m_pOwnerBlock->dlheapmemalign( this, alignment, size ); }
	void  Free( void* mem ) { m_pOwnerBlock->dlheapfree( this, mem ); }

	void SetName( const TCHAR* name )
	{
		strncpy_s( m_Name, name, TMemoryDL::HEAP_MAXNAME );
		m_Name[ TMemoryDL::HEAP_MAXNAME ] = '\0';
	}

	void* GetMSpace() const { return m_MSpace; }

private:
	void CreateMutex() { m_Mutex.Create(); }
	void DestroyMutex() { m_Mutex.Destroy(); }

	static void* AllocAsPile( TMemoryDLHeap* heap, TSIZE size, TSIZE alignment = 4 );

private:
	TMemoryDL*         m_pOwnerBlock;
	TMemoryDLHeapFlags m_Flags;
	T2Mutex            m_Mutex;
	TCHAR*             m_SubHeapBuffer;
	void*              m_MSpace;
	TCHAR*             m_PileData;
	TUINT32            m_PileSize;
	TCHAR              m_Name[ TMemoryDL::HEAP_MAXNAME + 1 ];
};

#endif // TMEMORY_USE_DLMALLOC

TOSHI_NAMESPACE_END
