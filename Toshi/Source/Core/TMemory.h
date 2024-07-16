#pragma once
#include "Toshi/TNodeList.h"

#ifdef TOSHI_PROFILER_MEMORY
#include "TMemoryDebug.h"
#endif // TOSHI_PROFILER_MEMORY

#define TMEMORY_ROUNDUP 4 
#define TMEMORY_FLAGS_HOLE_USED 1
#define TMEMORY_FLAGS_MASK ((1 << 2) - 1)
#define TMEMORY_NUM_FREELISTS 9
#define TMEMORY_NUM_BLOCK_SLOTS 128
#define TMEMORY_ALLOC_HOLE_SIZE sizeof( Toshi::TMemory::Hole )
#define TMEMORY_ALLOC_RESERVED_SIZE ( TMEMORY_ALLOC_HOLE_SIZE - sizeof( void* ) )

namespace Toshi {

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

		struct Hole
		{
			Hole* m_pOwnerHole;
			TUINT m_uiSize;
			union {
				Hole* m_pNextHole;
				MemBlock* m_pMemBlock;
			};

			// Only used when the hole is split
			Hole* m_pPrevHole;

			void* GetDataRegionStart() const { return (void*)( &m_pPrevHole ); }
			void* GetDataRegionEnd() const { return (void*)( ( (TUINTPTR)&m_pPrevHole + TAlignNumDown( m_uiSize ) ) ); }
		};

		struct MemBlock
		{
			MemBlockSlot* m_pSlot;
			TUINT m_uiTotalSize1;
			MemBlock* m_pNextBlock;
			TUINT m_uiTotalSize2;
			Hole* m_pHoles[ TMEMORY_NUM_FREELISTS ];
			Hole* m_pFirstHole;
			TCHAR m_szSignature[ 8 ];
			TCHAR m_szName[ 52 ];
			Hole m_RootHole;
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

		MemBlock* CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock );
		MemBlock* CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName );
		void DestroyMemBlock( MemBlock* a_pMemBlock );

		MemBlock* GetGlobalBlock() const;
		MemBlock* SetGlobalBlock( MemBlock* a_pMemBlock );

		void DumpMemInfo();

	private:
		TBOOL FreeMemBlock( MemBlock* a_pMemBlock );
		void SetMemBlockUnused( MemBlock* a_pMemBlock );

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
		TUINT m_uiGlobalHoleFlags;
		TUINT m_Unknown2;
		TBOOL m_bFlag1;
		TBOOL m_bFlag2;
		HALMemInfo m_HALMemInfo;
	};

}

void* TMalloc( TUINT a_uiSize );
void* TMalloc( TUINT a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum );
void* TMalloc( TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName = TNULL, TINT a_iLineNum = -1 );
void* TMemalign( TUINT a_uiSize, TINT a_iAlignment );
void* TMemalign( TINT a_iAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock );

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

inline void* __CRTDECL operator new( size_t size, Toshi::TMemory::MemBlock* block )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, block, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, block, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

inline void* __CRTDECL operator new[]( size_t size, Toshi::TMemory::MemBlock* block )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, block, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, block, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

inline void __CRTDECL operator delete( void* ptr, Toshi::TMemory::MemBlock* block ) noexcept
{
	TFree( ptr );
}

inline void __CRTDECL operator delete[]( void* ptr, Toshi::TMemory::MemBlock* block ) noexcept
{
	TFree( ptr );
}