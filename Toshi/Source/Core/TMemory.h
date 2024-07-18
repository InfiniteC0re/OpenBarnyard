#pragma once
#include "Toshi/TNodeList.h"

#ifdef TOSHI_PROFILER_MEMORY
#include "TMemoryDebug.h"
#endif // TOSHI_PROFILER_MEMORY

#ifdef TMEMORY_USE_DLMALLOC
#include "TMemory_dlmalloc.h"
#endif // TMEMORY_USE_DLMALLOC

#ifndef TMEMORY_USE_DLMALLOC

#define TMEMORY_ROUNDUP 4 
#define TMEMORY_FLAGS_HOLE_PROCESS 1
#define TMEMORY_FLAGS_MASK ((1 << 2) - 1)
#define TMEMORY_NUM_FREELISTS 9
#define TMEMORY_NUM_BLOCK_SLOTS 128
#define TMEMORY_ALLOC_MEMNODE_SIZE sizeof( Toshi::TMemory::MemNode )
#define TMEMORY_ALLOC_RESERVED_SIZE ( TMEMORY_ALLOC_MEMNODE_SIZE - sizeof( void* ) )

TOSHI_NAMESPACE_START

extern class TMemory* g_pMemory;

//-----------------------------------------------------------------------------
// Purpose: Manages memory allocations and deallocations.
// NOTE: Alloc and Free methods are not valid since they can crash the game
// when the modloader forces the original game to use our custom methods.
//-----------------------------------------------------------------------------
class TMemory
{
public:
	struct MemBlock;
	struct MemBlockSlot;

	class MemNode
	{
	public:
		void* GetDataRegionStart() const { return (void*)( &pPrevHole ); }
		void* GetDataRegionEnd() const { return (void*)( ( (TUINTPTR)&pPrevHole + TAlignNumDown( uiSize ) ) ); }

	public:
		MemNode* pOwner;
		TUINT uiSize;
		union {
			MemNode* pNextHole;
			MemBlock* pMemBlock;
		};

		// Only used when the hole is split
		MemNode* pPrevHole;
	};

	struct MemBlock
	{
		MemBlockSlot* m_pSlot;
		TUINT m_uiTotalSize1;
		MemBlock* m_pNextBlock;
		TUINT m_uiTotalSize2;
		MemNode* m_apHoles[ TMEMORY_NUM_FREELISTS ];
		MemNode* m_pFirstHole;
		TCHAR m_szSignature[ 8 ];
		TCHAR m_szName[ 52 ];
		MemNode m_RootHole;
	};

	struct MemBlockFooter
	{
		TUINT m_Unk1;            // -0x10
		TUINT m_Unk2;            // -0x0C
		MemBlock* m_pBlockOwner; // -0x08
		TUINT m_Unk4;            // -0x04
	};

	struct MemBlockSlot :
		public TNodeList<MemBlockSlot>::TNode
	{
		MemBlock* m_pPtr = TNULL;
	};

	struct MemInfo
	{
		TUINT m_uiTotalSize;
		TUINT m_uiLogicTotalSize;
		TUINT m_uiTotalFree;
		TUINT m_uiLogicTotalFree;
		TUINT m_uiTotalUsed;
		TUINT m_uiLogicTotalUsed;
		TINT m_iNumHoles;
		TINT m_iNumProcesses;
		TUINT m_uiLargestHole;
		TUINT m_uiLargestProcess;
		TUINT m_uiSmallestHole;
		TUINT m_uiSmallestProcess;
		TUINT m_uiUnk3;
		TUINT m_uiUnk4;
	};

	struct HALMemInfo
	{
		HALMemInfo();

		TUINT m_Unknown1[ 10 ];
		TUINT m_uiMemUsage;
		TUINT m_Unknown2[ 15 ];
	};

public:
	TMemory();
	~TMemory();

	void* Alloc( TUINT a_uiSize, TINT a_uiAlignment, MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum );
	TBOOL Free( const void* a_pMem );

	void* SysAlloc( TSIZE a_uiSize );
	void SysFree( void* a_pMem );

	MemBlock* CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock );
	MemBlock* CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName );
	void DestroyMemBlock( MemBlock* a_pMemBlock );

	MemBlock* GetGlobalBlock() const;
	MemBlock* SetGlobalBlock( MemBlock* a_pMemBlock );

	TUINT GetGlobalFlags() const { return m_uiGlobalFlags; }

	void DumpMemInfo();

private:
	TBOOL FreeMemBlock( MemBlock* a_pMemBlock );
	void SetMemBlockUnused( MemBlock* a_pMemBlock );

	static MemNode* GetMemNodeFromAddress( void* a_pMem );
	static void ExtendNodeSize( MemNode* a_pNode, TUINT a_uiExtendSize ) { a_pNode->uiSize = a_uiExtendSize | ( a_pNode->uiSize & TMEMORY_FLAGS_MASK ); }
	static void SetHoleSize( MemNode* a_pNode, TUINT a_uiHoleSize ) { a_pNode->uiSize = a_uiHoleSize; }
	static TBOOL IsProcess( MemNode* a_pNode ) { return HASANYFLAG( a_pNode->uiSize, TMEMORY_FLAGS_HOLE_PROCESS ); }
	static void SetProcess( MemBlock* a_pMemBlock, MemNode* a_pNode, TUINT a_uiHoleSize ) { a_pNode->uiSize = a_uiHoleSize | g_pMemory->GetGlobalFlags() | TMEMORY_FLAGS_HOLE_PROCESS; a_pNode->pMemBlock = a_pMemBlock; }
	static TUINT GetNodeSize( MemNode* a_pNode ) { return TAlignNumDown( a_pNode->uiSize ); }
	static MemBlock* GetProcessMemBlock( MemNode* a_pNode ) { return a_pNode->pMemBlock; }
	static void ConvertProcessToHole( MemNode* a_pNode ) { a_pNode->uiSize &= ~TMEMORY_FLAGS_MASK; }
	static int TestMemIntegrity( MemBlock* a_pMemBlock );
	static int DebugTestMemoryBlock( MemBlock* a_pMemBlock );

public:
	static void GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock );
	static void GetHALMemInfo( HALMemInfo& a_rHALMemInfo );
	static TBOOL Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize );
	static void Deinitialise();
	static TUINT MapSizeToFreeList( TUINT a_uiSize );
	static void DebugPrintHALMemInfo( const TCHAR* a_szFormat, ... );

private:
	inline static class TMutex* ms_pGlobalMutex;

private:
	TUINT m_TotalAllocatedSize;
	TUINT m_ReservedSize;
	TUINT m_MainBlockSize;
	TNodeList<MemBlockSlot> m_UsedBlocks;
	TNodeList<MemBlockSlot> m_FreeBlocks;
	MemBlockSlot m_aBlockSlots[ TMEMORY_NUM_BLOCK_SLOTS ];
	void* m_pMemory;
	MemBlock* m_pGlobalBlock;
	TUINT m_uiGlobalFlags;
	TUINT m_Unknown2;
	TBOOL m_bFlag1;
	TBOOL m_bFlag2;
	HALMemInfo m_HALMemInfo;
};

TOSHI_NAMESPACE_END

#else  // !TMEMORY_USE_DLMALLOC

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: A wrapper around TMemoryDL that makes it's API compatible to the
// JPOG/Barnyard memory allocator.
//-----------------------------------------------------------------------------
class TMemory
{
public:
	using MemBlock = TMemoryDLHeap;

	struct MemInfo
	{
		TUINT m_uiTotalSize;
		TUINT m_uiLogicTotalSize;
		TUINT m_uiTotalFree;
		TUINT m_uiLogicTotalFree;
		TUINT m_uiTotalUsed;
		TUINT m_uiLogicTotalUsed;
		TINT m_iNumHoles;
		TINT m_iNumProcesses;
		TUINT m_uiLargestHole;
		TUINT m_uiLargestProcess;
		TUINT m_uiSmallestHole;
		TUINT m_uiSmallestProcess;
		TUINT m_uiUnk3;
		TUINT m_uiUnk4;
	};

	struct HALMemInfo
	{
		TUINT m_Unknown1[ 10 ];
		TUINT m_uiMemUsage;
		TUINT m_Unknown2[ 15 ];
	};

public:
	TMemory();

	MemBlock* CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock );
	MemBlock* CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName );
	void DestroyMemBlock( MemBlock* a_pMemBlock );

	MemBlock* GetGlobalBlock() const;
	MemBlock* SetGlobalBlock( MemBlock* a_pMemBlock );

public:
	static TBOOL Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize );
	static void Deinitialise();

	static void GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock );
	static void GetHALMemInfo( HALMemInfo& a_rHALMemInfo );

private:
	TMemoryDL* m_pMemModule;
};

extern TMemory* g_pMemory;

TOSHI_NAMESPACE_END

#endif // TMEMORY_USE_DLMALLOC

void* TMalloc( TUINT a_uiSize );
void* TMalloc( TUINT a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum );
void* TMalloc( TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName = TNULL, TINT a_iLineNum = -1 );
void* TMemalign( TUINT a_uiSize, TUINT a_uiAlignment );
void* TMemalign( TUINT a_uiAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock );

void TFree( void* a_pMem );

//-----------------------------------------------------------------------------
// Purpose: Calls constructor of type T on the specified pointer.
//-----------------------------------------------------------------------------
template<class T, class... Args>
TFORCEINLINE T* TConstruct( T* a_pMemory, Args&& ...args )
{
	return new ( a_pMemory ) T( std::forward<Args>( args )... );
}

//-----------------------------------------------------------------------------
// Purpose: Calls constructor of type T on the specified pointer N times.
//-----------------------------------------------------------------------------
template<class T, class... Args>
TFORCEINLINE T* TConstructArray( T* a_pMemory, TUINT a_uiNumTimes, Args&& ...args )
{
	for ( TUINT i = 0; i < a_uiNumTimes; i++ )
	{
		new ( a_pMemory + i ) T( std::forward<Args>( args )... );
	}

	return a_pMemory;
}

//-----------------------------------------------------------------------------
// Purpose: Calls destructor on the specified pointer.
//-----------------------------------------------------------------------------
template<class T>
TFORCEINLINE void TDestruct( T* a_pPtr )
{
	if ( a_pPtr )
	{
		a_pPtr->~T();
	}
}

TFORCEINLINE void* __CRTDECL operator new( size_t size, Toshi::TMemory::MemBlock* block )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, block, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, block, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

TFORCEINLINE void* __CRTDECL operator new[]( size_t size, Toshi::TMemory::MemBlock* block )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, block, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, block, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

TFORCEINLINE void __CRTDECL operator delete( void* ptr, Toshi::TMemory::MemBlock* block ) noexcept
{
	TFree( ptr );
}

TFORCEINLINE void __CRTDECL operator delete[]( void* ptr, Toshi::TMemory::MemBlock* block ) noexcept
{
	TFree( ptr );
}
