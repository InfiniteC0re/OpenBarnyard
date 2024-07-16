#include "ToshiPCH.h"
#include "TMemory.h"
#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

#ifdef TOSHI_PROFILER_MEMORY
//#include "Profiler/tracy/Tracy.hpp"
#endif // TOSHI_PROFILER_MEMORY

#define MEM_TO_HOLE(PTR) (((Hole*)(((TUINT)PTR) + sizeof(void*))) - 1)

#include "TMemoryDebugOff.h"

void* __CRTDECL operator new( size_t size )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void* __CRTDECL operator new( size_t size, ::std::nothrow_t const& ) noexcept
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void* __CRTDECL operator new[]( size_t size )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void* __CRTDECL operator new[]( size_t size, ::std::nothrow_t const& ) noexcept
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void __CRTDECL operator delete( void* ptr ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete( void* ptr, ::std::nothrow_t const& ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete[]( void* ptr ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete[]( void* ptr, ::std::nothrow_t const& ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete[]( void* ptr, size_t _Size ) noexcept
{
	TFree( ptr );
}

namespace Toshi {

	TMemory* g_pMemory = TNULL;

	TMemory::TMemory()
	{
		TASSERT( g_pMemory == TNULL );
		g_pMemory = this;

		m_TotalAllocatedSize = 0;

		for ( TINT i = 0; i < TMEMORY_NUM_BLOCK_SLOTS; i++ )
		{
			m_FreeBlocks.InsertTail( m_aBlockSlots[ i ] );
		}

		m_bFlag1 = TFALSE;
		m_bFlag2 = TTRUE;
	}

	TMemory::~TMemory()
	{
		g_pMemory = TNULL;
	}

	// General utility macro
#define PP_CAT( A, B ) A ## B
#define PP_EXPAND(...) __VA_ARGS__

// Macro overloading feature support
#define PP_VA_ARG_SIZE(...) PP_EXPAND(PP_APPLY_ARG_N((PP_ZERO_ARGS_DETECT(__VA_ARGS__), PP_RSEQ_N)))

#define PP_ZERO_ARGS_DETECT(...) PP_EXPAND(PP_ZERO_ARGS_DETECT_PREFIX_ ## __VA_ARGS__ ## _ZERO_ARGS_DETECT_SUFFIX)
#define PP_ZERO_ARGS_DETECT_PREFIX__ZERO_ARGS_DETECT_SUFFIX ,,,,,,,,,,,0

#define PP_APPLY_ARG_N(ARGS) PP_EXPAND(PP_ARG_N ARGS)
#define PP_ARG_N(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, N,...) N
#define PP_RSEQ_N 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define PP_OVERLOAD_SELECT(NAME, NUM) PP_CAT( NAME ## _, NUM)
#define PP_MACRO_OVERLOAD(NAME, ...) PP_OVERLOAD_SELECT(NAME, PP_VA_ARG_SIZE(__VA_ARGS__))(__VA_ARGS__)

#define CALL_THIS_4(ADDR, TYPE, RET_TYPE, THIS) ((RET_TYPE(__thiscall*)(TYPE pThis))(ADDR))(THIS)
#define CALL_THIS_6(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1) ((RET_TYPE(__thiscall*)(TYPE, TYPE1))(ADDR))(THIS, VALUE1)
#define CALL_THIS_8(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2))(ADDR))(THIS, VALUE1, VALUE2)
#define CALL_THIS_10(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2, TYPE3))(ADDR))(THIS, VALUE1, VALUE2, VALUE3)
#define CALL_THIS_12(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2, TYPE3, TYPE4))(ADDR))(THIS, VALUE1, VALUE2, VALUE3, VALUE4)
#define CALL_THIS_14(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4, TYPE5, VALUE5) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5))(ADDR))(THIS, VALUE1, VALUE2, VALUE3, VALUE4, VALUE5)
#define CALL_THIS(...) PP_MACRO_OVERLOAD(CALL_THIS, __VA_ARGS__)

#define CALL_2(ADDR, RET_TYPE) ((RET_TYPE(__stdcall*)())(ADDR))()
#define CALL_4(ADDR, RET_TYPE, TYPE1, VALUE1) ((RET_TYPE(__stdcall*)(TYPE1))(ADDR))(VALUE1)
#define CALL_6(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2))(ADDR))(VALUE1, VALUE2)
#define CALL_8(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2, TYPE3))(ADDR))(VALUE1, VALUE2, VALUE3)
#define CALL_10(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2, TYPE3, TYPE4))(ADDR))(VALUE1, VALUE2, VALUE3, VALUE4)
#define CALL_12(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4, TYPE5, VALUE5) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2, TYPE3, TYPE4, TYPE5))(ADDR))(VALUE1, VALUE2, VALUE3, VALUE4, VALUE5)
#define CALL(...) PP_MACRO_OVERLOAD(CALL, __VA_ARGS__)

	void* TMemory::Alloc( TUINT a_uiSize, TINT a_uiAlignment, MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum )
	{
		TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

		TUINT uiAllocationSize = a_uiSize;
		TUINT uiAlignment = a_uiAlignment;

		if ( uiAllocationSize < 4 )
			uiAllocationSize = 4;

		if ( uiAlignment < 16 )
		{
			uiAlignment = 16;
		}
		else if ( uiAlignment < TMEMORY_ROUNDUP )
		{
			TDebug_FinalPrintf( "MEMORY ERROR: CANT ALLOC Alignment(%d)<TMEMORY_ROUNDUP\n", uiAlignment );
			DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
			TASSERT( TFALSE );
			return TNULL;
		}

		// Use global block if it's not specified
		if ( a_pMemBlock == TNULL )
			a_pMemBlock = m_pGlobalBlock;

		if ( uiAllocationSize == 0 )
		{
			DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
			TASSERT( TFALSE );
			return TNULL;
		}

		uiAllocationSize = TAlignNumUp( uiAllocationSize );

		void* pAllocatedMemory = TNULL;
		Hole* pFreeList = TNULL;

#ifdef TOSHI_PROFILER_MEMORY
#include "TMemoryDebugOff.h"

		auto SaveDebugInfo = [ a_szFileName, a_iLineNum, uiAllocationSize ]( void* pMem ) {
			if ( pMem )
			{
				TracyAlloc( pMem, uiAllocationSize );
			}
		};
#else  // TOSHI_PROFILER_MEMORY
#define SaveDebugInfo(...)
#endif // !TOSHI_PROFILER_MEMORY

		TUINT uiId = MapSizeToFreeList( uiAllocationSize );

		if ( uiId >= TMEMORY_NUM_FREELISTS )
			goto OUT_OF_MEMORY;

		for ( ; uiId < TMEMORY_NUM_FREELISTS; ++uiId )
		{
			pFreeList = a_pMemBlock->m_pHoles[ uiId ];

			// Find a hole that can allocate the required number of bytes
			TUINTPTR iDataRegionStart;
			TUINTPTR iDataRegionEnd;
			TUINTPTR iDataRegionSize;

			while ( pFreeList != TNULL )
			{
				pAllocatedMemory = TAlignPointerUp( pFreeList->GetDataRegionStart(), uiAlignment );

				iDataRegionStart = TREINTERPRETCAST( TUINTPTR, pAllocatedMemory );
				iDataRegionEnd = TREINTERPRETCAST( TUINTPTR, pFreeList->GetDataRegionEnd() );
				iDataRegionSize = iDataRegionEnd - iDataRegionStart;

				// Check if the allocation fits this hole
				if ( iDataRegionEnd > iDataRegionStart && iDataRegionSize >= uiAllocationSize )
					break;

				// Go for a next hole split from this one
				pFreeList = pFreeList->m_pNextHole;
			}

			// This freelist can't be used, let's check for the next
			if ( pFreeList == TNULL )
				continue;

			Hole* pAllocationHole = MEM_TO_HOLE( iDataRegionStart );

			if ( Hole* pOwnerHole = pFreeList->m_pOwnerHole )
			{
				// This line probably has a bug which presents in the original code but idk
				pOwnerHole->m_uiSize = ( iDataRegionStart - (TUINTPTR)pFreeList->m_pOwnerHole - 24 )
					| ( pOwnerHole->m_uiSize & TMEMORY_FLAGS_MASK );
			}
			else if ( pAllocationHole != pFreeList )
			{
				// Seems that due to alignment we have a gap between start of the
				// data region and the actual address we gonna return so let's
				// make sure we don't lost this pointer
				a_pMemBlock->m_pFirstHole = pAllocationHole;
			}

			{
				// Unlink the hole from the linked list
				if ( Hole* pPrevHole = pFreeList->m_pPrevHole )
					pPrevHole->m_pNextHole = pFreeList->m_pNextHole;        // Remove reference to this hole from the previous one
				else
					a_pMemBlock->m_pHoles[ uiId ] = pFreeList->m_pNextHole; // Since this is the first hole, it's stored in the memblock's list

				// Remove reference to this hole from the next one
				if ( Hole* pNextHole = pFreeList->m_pNextHole )
					pNextHole->m_pPrevHole = pFreeList->m_pPrevHole;
			}

			// I have no clue what it does but I won't lose sleep over this
			if ( pFreeList != pAllocationHole )
			{
				Hole** ppHole = (Hole**)pFreeList->GetDataRegionEnd();
				pAllocationHole->m_pOwnerHole = pFreeList->m_pOwnerHole;
				*ppHole = pAllocationHole;
			}

			// Make sure the hole has correct info about it's size and memblock
			pAllocationHole->m_uiSize = iDataRegionSize | g_pMemory->m_uiGlobalHoleFlags | TMEMORY_FLAGS_HOLE_USED;
			pAllocationHole->m_pMemBlock = a_pMemBlock;

			// Check if we can split the hole in two
			if ( iDataRegionSize > uiAllocationSize + TMEMORY_ALLOC_HOLE_SIZE )
			{
				// We can split it!

				TUINT uiOldHoleSize = TAlignNumDown( pAllocationHole->m_uiSize );

				// Create a new hole right after the allocated data
				Hole* pNewHole = (Hole*)( iDataRegionStart + uiAllocationSize );

				// Set size of the new hole
				pNewHole->m_uiSize = uiOldHoleSize - uiAllocationSize - TMEMORY_ALLOC_RESERVED_SIZE;
				pNewHole->m_pOwnerHole = pAllocationHole;

				// Update size of the old hole and mark it as used
				pAllocationHole->m_pMemBlock = a_pMemBlock;
				pAllocationHole->m_uiSize = uiAllocationSize | g_pMemory->m_uiGlobalHoleFlags | TMEMORY_FLAGS_HOLE_USED;

				// Place the new hole in the memblock's list
				TUINT uiNewHoleId = MapSizeToFreeList( pNewHole->m_uiSize );
				Hole* pOldHole = a_pMemBlock->m_pHoles[ uiNewHoleId ];

				pNewHole->m_pPrevHole = TNULL;
				pNewHole->m_pNextHole = pOldHole;

				if ( pOldHole )
					pOldHole->m_pPrevHole = pNewHole;

				a_pMemBlock->m_pHoles[ uiNewHoleId ] = pNewHole;

				// Save pointer to the hole right at the end of the data region (probably for some validation)
				*(Hole**)( pNewHole->GetDataRegionEnd() ) = pNewHole;

				SaveDebugInfo( pAllocatedMemory );
				return pAllocatedMemory;
			}
			else
			{
				// Damn, we can't split this one but it surely can fit the allocation

				SaveDebugInfo( pAllocatedMemory );
				return pAllocatedMemory;
			}
		}

		if ( pFreeList )
		{
			SaveDebugInfo( pAllocatedMemory );
			return pAllocatedMemory;
		}

OUT_OF_MEMORY:
		DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
		DebugPrintHALMemInfo( "Requested memory block size: %d\n", uiAllocationSize );
		DumpMemInfo();
		pAllocatedMemory = TNULL;

		TASSERT( TFALSE );
		return pAllocatedMemory;
	}

	TBOOL TMemory::Free( const void* a_pAllocated )
	{
		return CALL_THIS(0x006b4a20, TMemory*, TBOOL, this, const void*, a_pAllocated );

		TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

		TUINTPTR uiAllocatedAddr = TREINTERPRETCAST( TUINTPTR, a_pAllocated );

		if ( a_pAllocated && TIsPointerAligned( a_pAllocated ) )
		{
			Hole* pThisHole = MEM_TO_HOLE( a_pAllocated );
			MemBlock* pThisMemBlock = pThisHole->m_pMemBlock;

			TUINT uiAllocatedSize = TAlignNumDown( pThisHole->m_uiSize );
			Hole* pNextMemHole = (Hole*)( uiAllocatedAddr + uiAllocatedSize );

			Hole* pOwnerHole = pThisHole->m_pOwnerHole;
			Hole** ppNextHole = &pThisHole->m_pNextHole;

			pThisHole->m_uiSize = uiAllocatedSize;

			// If has owner and it's not used, merge with it
			if ( pOwnerHole && !HASANYFLAG( pOwnerHole->m_uiSize, TMEMORY_FLAGS_HOLE_USED ) )
			{
				TUINT uiOwnerHoleSize = pOwnerHole->m_uiSize;
				pOwnerHole->m_uiSize = uiAllocatedSize + TMEMORY_ALLOC_RESERVED_SIZE + TAlignNumDown( uiOwnerHoleSize ) | uiOwnerHoleSize & TMEMORY_FLAGS_MASK;

				pNextMemHole->m_pOwnerHole = pOwnerHole;
				ppNextHole = &pOwnerHole->m_pNextHole;

				if ( pOwnerHole->m_pPrevHole == TNULL )
				{
					TUINT uiMappedHoleId = MapSizeToFreeList( TAlignNumDown( uiOwnerHoleSize ) );
					pThisMemBlock->m_pHoles[ uiMappedHoleId ] = *ppNextHole;
				}
				else
				{
					pOwnerHole->m_pPrevHole->m_pNextHole = *ppNextHole;
				}

				pThisHole = pOwnerHole;

				if ( *ppNextHole )
				{
					( *ppNextHole )->m_pPrevHole = pOwnerHole->m_pPrevHole;
				}
			}

			// If the next lying hole is not used, merge with it
			if ( pNextMemHole && !HASANYFLAG( pNextMemHole->m_uiSize, TMEMORY_FLAGS_HOLE_USED ) )
			{
				if ( pNextMemHole->m_pPrevHole == TNULL )
				{
					TUINT uiMappedHoleId = MapSizeToFreeList( TAlignNumDown( pNextMemHole->m_uiSize ) );
					pThisMemBlock->m_pHoles[ uiMappedHoleId ] = pNextMemHole->m_pNextHole;
				}
				else
				{
					pNextMemHole->m_pPrevHole->m_pNextHole = pNextMemHole->m_pNextHole;
				}

				if ( pNextMemHole->m_pNextHole != TNULL )
				{
					pNextMemHole->m_pNextHole->m_pPrevHole = pNextMemHole->m_pPrevHole;
				}

				TUINT uiSize = TAlignNumDown( pNextMemHole->m_uiSize );

				pThisHole->m_uiSize = uiSize + TMEMORY_ALLOC_RESERVED_SIZE + TAlignNumDown( pThisHole->m_uiSize ) | pThisHole->m_uiSize & TMEMORY_FLAGS_MASK;
				*(Hole**)( pNextMemHole->GetDataRegionEnd() ) = pThisHole;
			}

			TUINT uiMappedHoleId = MapSizeToFreeList( TAlignNumDown( pThisHole->m_uiSize ) );
			pThisHole->m_pPrevHole = TNULL;

			Hole* pOldHole = pThisMemBlock->m_pHoles[ uiMappedHoleId ];
			*ppNextHole = pOldHole;

			if ( pOldHole != TNULL )
			{
				pOldHole->m_pPrevHole = pThisHole;
			}

			pThisMemBlock->m_pHoles[ uiMappedHoleId ] = pThisHole;

#ifdef TOSHI_PROFILER_MEMORY

			TracyFree( a_pAllocated );

#endif // TOSHI_PROFILER_MEMORY

			return TTRUE;
		}

		return TFALSE;
	}

	TMemory::MemBlock* TMemory::CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock )
	{
		void* pMem = Alloc( a_uiSize, 16, a_pOwnerBlock, TNULL, -1 );
		return CreateMemBlockInPlace( pMem, a_uiSize, a_szName );
	}

	TMemory::MemBlock* TMemory::CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName )
	{
		TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

		if ( a_pMem && a_uiSize != 0 && !m_FreeBlocks.IsEmpty() )
		{
			auto pBlock = TREINTERPRETCAST( MemBlock*, TAlignNumDown( TREINTERPRETCAST( TUINT32, a_pMem ) ) );
			auto uiBlockTotalSize = ( TREINTERPRETCAST( TUINT32, a_pMem ) + TAlignNumDown( a_uiSize ) ) - TREINTERPRETCAST( TUINT32, pBlock );

			if ( pBlock )
			{
				auto pSlot = m_FreeBlocks.RemoveHead();
				m_UsedBlocks.InsertHead( *pSlot );

				pBlock->m_pSlot = pSlot;
				pBlock->m_pSlot->m_pPtr = pBlock;
			}

			if ( uiBlockTotalSize != 0 )
			{
				constexpr TUINT CHUNK_RESERVED_SIZE = ( sizeof( MemBlock ) + ( sizeof( Hole ) - sizeof( void* ) ) );

				pBlock->m_uiTotalSize1 = uiBlockTotalSize;
				TUtil::MemClear( pBlock->m_pHoles, sizeof( pBlock->m_pHoles ) );
				pBlock->m_pFirstHole = &pBlock->m_RootHole;

				auto pHole = &pBlock->m_RootHole;
				pHole->m_uiSize = uiBlockTotalSize - CHUNK_RESERVED_SIZE;
				pHole->m_pNextHole = TNULL;
				pHole->m_pPrevHole = TNULL;
				pHole->m_pOwnerHole = TNULL;

				auto uiFreeListId = MapSizeToFreeList( uiBlockTotalSize - CHUNK_RESERVED_SIZE );
				pBlock->m_pHoles[ uiFreeListId ] = pHole;

				auto pBlockFooter = TREINTERPRETCAST( MemBlockFooter*, TREINTERPRETCAST( TUINT32, pBlock ) + uiBlockTotalSize ) - 1;
				pBlockFooter->m_pBlockOwner = TNULL;
				pBlockFooter->m_Unk4 = 0;
				pBlockFooter->m_Unk1 = 0;
				pBlockFooter->m_Unk2 = g_pMemory->m_uiGlobalHoleFlags | TMEMORY_FLAGS_HOLE_USED;
				pBlockFooter->m_pBlockOwner = pBlock;

				pBlock->m_pNextBlock = pBlock;
				pBlock->m_uiTotalSize2 = uiBlockTotalSize;
				TStringManager::String8Copy( pBlock->m_szSignature, "#MB_uID" );
				TStringManager::String8Copy( pBlock->m_szName, a_szName );

				return pBlock;
			}
		}

		return TNULL;
	}

	void TMemory::DestroyMemBlock( MemBlock* a_pMemBlock )
	{
		FreeMemBlock( a_pMemBlock );
		Free( a_pMemBlock );
	}

	Toshi::TMemory::MemBlock* TMemory::GetGlobalBlock() const
	{
		return m_pGlobalBlock;
	}

	TBOOL TMemory::FreeMemBlock( MemBlock* a_pMemBlock )
	{
		TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

		SetMemBlockUnused( a_pMemBlock );
		a_pMemBlock->m_pSlot->Remove();
		m_FreeBlocks.InsertHead( *a_pMemBlock->m_pSlot );

		return TTRUE;
	}

	void TMemory::SetMemBlockUnused( MemBlock* a_pMemBlock )
	{
		a_pMemBlock->m_pSlot->Remove();
		m_FreeBlocks.InsertHead( *a_pMemBlock->m_pSlot );

		CreateMemBlockInPlace( a_pMemBlock, a_pMemBlock->m_uiTotalSize1, "_unused" );
		TStringManager::String8Copy( a_pMemBlock->m_szSignature, "xxxxxxx" );
	}

	TBOOL TMemory::Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize )
	{
		auto tmemory = TSTATICCAST( TMemory, calloc( sizeof( TMemory ), 1 ) );
		new ( tmemory ) TMemory();

		tmemory->m_pMemory = TNULL;
		tmemory->m_pGlobalBlock = TNULL;
		tmemory->m_uiGlobalHoleFlags = 0;
		tmemory->m_Unknown2 = 0;

		ms_pGlobalMutex = TSTATICCAST( TMutex, malloc( sizeof( TMutex ) ) );
		new ( ms_pGlobalMutex ) TMutex();

		tmemory->m_ReservedSize = a_uiReservedSize;
		tmemory->m_TotalAllocatedSize = ( a_uiHeapSize == 0 ) ? 128 * 1024 * 1024 : a_uiHeapSize;
		tmemory->m_MainBlockSize = tmemory->m_TotalAllocatedSize - a_uiReservedSize;
		tmemory->m_pMemory = malloc( tmemory->m_TotalAllocatedSize );
		tmemory->m_pGlobalBlock = tmemory->CreateMemBlockInPlace(
			tmemory->m_pMemory,
			tmemory->m_TotalAllocatedSize,
			"Toshi"
		);

		return TTRUE;
	}

	void TMemory::Deinitialise()
	{
		TASSERT( g_pMemory != TNULL );
		auto pMainBlockMemory = g_pMemory->m_pMemory;

		g_pMemory->m_UsedBlocks.RemoveAll();
		g_pMemory->m_FreeBlocks.RemoveAll();
		g_pMemory->~TMemory();

		free( g_pMemory );
		free( pMainBlockMemory );
	}

	TUINT TMemory::MapSizeToFreeList( TUINT a_uiSize )
	{
		TFLOAT fSize = TFLOAT( TAlignNumUp( a_uiSize ) - 1 );
		TUINT uiExponentSign = ( *(TINT*)&fSize ) >> 23;
		TUINT uiResult = uiExponentSign - 127;

		if ( uiResult & 1 )
			uiResult = uiExponentSign - 126;

		uiResult = uiResult >> 1;

		if ( uiResult != 0 )
			uiResult = uiResult - 1;

		if ( uiResult >= TMEMORY_NUM_FREELISTS )
			uiResult = TMEMORY_NUM_FREELISTS - 1;

		return uiResult;
	}

	TMemory::MemBlock* TMemory::SetGlobalBlock( MemBlock* a_pMemBlock )
	{
		MemBlock* pOldMemBlock = m_pGlobalBlock;
		m_pGlobalBlock = a_pMemBlock;
		return pOldMemBlock;
	}

	void TMemory::DumpMemInfo()
	{
		MemInfo memInfo;

		for ( auto it = m_UsedBlocks.Tail(); it != m_UsedBlocks.End(); --it )
		{
			GetMemInfo( memInfo, it->m_pPtr );

			DebugPrintHALMemInfo( "Pool: \'%s\'\n", it->m_pPtr->m_szName );
			DebugPrintHALMemInfo( "\tLargest Hole    : %d\n", memInfo.m_uiLargestHole );
			DebugPrintHALMemInfo( "\tSmallest Hole   : %d\n", memInfo.m_uiSmallestHole );
			DebugPrintHALMemInfo( "\tLargest Process : %d\n", memInfo.m_uiLargestProcess );
			DebugPrintHALMemInfo( "\tSmallest Process: %d\n", memInfo.m_uiSmallestProcess );
			DebugPrintHALMemInfo( "\tTotal Free      : %d\n", memInfo.m_uiTotalFree );
			DebugPrintHALMemInfo( "\tTotal Used      : %d\n", memInfo.m_uiTotalUsed );
			DebugPrintHALMemInfo( "\tTotal Size      : %d\n", memInfo.m_uiTotalSize );
			DebugPrintHALMemInfo( "\tLogic Total Free: %d\n", memInfo.m_uiLogicTotalFree );
			DebugPrintHALMemInfo( "\tLogic Total Used: %d\n", memInfo.m_uiLogicTotalUsed );
			DebugPrintHALMemInfo( "\tLogic Total Size: %d\n", memInfo.m_uiLogicTotalSize );

			TFLOAT fLogicTotalUsed = TMath::Abs( TFLOAT( memInfo.m_uiLogicTotalUsed ) );
			TFLOAT fLogicTotalSize = TMath::Abs( TFLOAT( memInfo.m_uiLogicTotalSize ) );
			DebugPrintHALMemInfo( "\t%%Logical Used   : %f\n", ( fLogicTotalUsed / fLogicTotalSize ) * 100.0 );

			TFLOAT fTotalUsed = TMath::Abs( TFLOAT( memInfo.m_uiTotalUsed ) );
			TFLOAT fTotalSize = TMath::Abs( TFLOAT( memInfo.m_uiTotalSize ) );
			DebugPrintHALMemInfo( "\t%%Used\t          : %f\n", ( fTotalUsed / fTotalSize ) * 100.0 );
			DebugPrintHALMemInfo( "------\n\n" );
		}
	}

	void TMemory::DebugPrintHALMemInfo( const TCHAR* a_szFormat, ... )
	{
		va_list args;
		va_start( args, a_szFormat );
		TDebug_FinalVPrintf( a_szFormat, args );
		va_end( args );
	}

	void TMemory::GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock )
	{
		TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

		if ( !a_pMemBlock )
		{
			a_pMemBlock = g_pMemory->m_pGlobalBlock;
		}

		a_rMemInfo.m_uiUnk3 = 28;
		a_rMemInfo.m_uiUnk4 = 28;
		a_rMemInfo.m_uiTotalSize = 0;
		a_rMemInfo.m_uiLogicTotalSize = 0;
		a_rMemInfo.m_uiLargestHole = 0;
		a_rMemInfo.m_uiLargestProcess = 0;
		a_rMemInfo.m_uiSmallestHole = 0;
		a_rMemInfo.m_uiSmallestProcess = 0;
		a_rMemInfo.m_uiTotalFree = 0;
		a_rMemInfo.m_uiLogicTotalFree = 0;
		a_rMemInfo.m_uiTotalUsed = 0;
		a_rMemInfo.m_uiLogicTotalUsed = 0;
		a_rMemInfo.m_iNumHoles = 0;
		a_rMemInfo.m_iNumProcesses = 0;
		a_rMemInfo.m_uiTotalSize = a_pMemBlock->m_uiTotalSize1;
		a_rMemInfo.m_uiLogicTotalSize = a_pMemBlock->m_uiTotalSize1;
		a_rMemInfo.m_uiSmallestProcess = -1;
		a_rMemInfo.m_uiSmallestHole = -1;
		a_rMemInfo.m_uiLargestProcess = 0;
		a_rMemInfo.m_uiLargestHole = 0;

		auto uiUnk = (TUINT)a_pMemBlock->m_pFirstHole + ( -88 - (TUINT)a_pMemBlock );
		a_rMemInfo.m_uiUnk3 = uiUnk;
		a_rMemInfo.m_uiUnk4 = uiUnk;

		auto pHole = a_pMemBlock->m_pFirstHole;
		TUINT uiHoleSize = pHole->m_uiSize;

		while ( TAlignNumDown( uiHoleSize ) != 0 )
		{
			a_rMemInfo.m_uiUnk3 += sizeof( Hole ) - sizeof( void* );
			uiHoleSize = TAlignNumDown( pHole->m_uiSize );

			if ( ( pHole->m_uiSize & 1 ) == 0 )
			{
				a_rMemInfo.m_iNumHoles += 1;
				a_rMemInfo.m_uiTotalFree += uiHoleSize;

				if ( a_rMemInfo.m_uiLargestHole <= uiHoleSize && uiHoleSize != a_rMemInfo.m_uiLargestHole )
				{
					a_rMemInfo.m_uiLargestHole = uiHoleSize;
				}

				if ( uiHoleSize < a_rMemInfo.m_uiSmallestHole )
				{
					a_rMemInfo.m_uiSmallestHole = uiHoleSize;
				}
			}
			else
			{
				a_rMemInfo.m_iNumProcesses += 1;
				a_rMemInfo.m_uiTotalUsed += uiHoleSize;

				if ( a_rMemInfo.m_uiLargestProcess <= uiHoleSize && uiHoleSize != a_rMemInfo.m_uiLargestProcess )
				{
					a_rMemInfo.m_uiLargestProcess = uiHoleSize;
				}

				if ( uiHoleSize < a_rMemInfo.m_uiSmallestProcess )
				{
					a_rMemInfo.m_uiSmallestProcess = uiHoleSize;
				}
			}

			auto pOldHole = pHole;
			pHole = (Hole*)( (TUINT)&pHole->m_pPrevHole + uiHoleSize );
			uiHoleSize = pHole->m_uiSize;
		}

		a_rMemInfo.m_uiLogicTotalFree = a_rMemInfo.m_uiTotalFree;
		a_rMemInfo.m_uiLogicTotalUsed = a_rMemInfo.m_uiTotalUsed + uiUnk;

		if ( a_rMemInfo.m_uiSmallestHole == -1 )
		{
			a_rMemInfo.m_uiSmallestHole = 0;
		}
	}

	void TMemory::GetHALMemInfo( HALMemInfo& a_rHALMemInfo )
	{

		TUtil::MemClear( &a_rHALMemInfo, sizeof( a_rHALMemInfo ) );
	}

	TMemory::HALMemInfo::HALMemInfo()
	{
		TUtil::MemClear( this, sizeof( *this ) );
	}
}

void* TMalloc( TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum )
{
	if ( !a_pMemBlock )
	{
		a_pMemBlock = Toshi::g_pMemory->GetGlobalBlock();
	}

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, 16, a_pMemBlock, a_szFileName, a_iLineNum );

	if ( !pMem )
	{
		Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
	}

	return pMem;
}

void* TMalloc( TUINT a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum )
{
	auto pMemBlock = Toshi::g_pMemory->GetGlobalBlock();

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, 16, pMemBlock, a_szFileName, a_iLineNum );

	if ( !pMem )
	{
		Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
	}

	return pMem;
}

void* TMalloc( TUINT a_uiSize )
{
	auto pMemBlock = Toshi::g_pMemory->GetGlobalBlock();

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, 16, pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
	}

	return pMem;
}

void* TMemalign( TINT a_iAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock )
{
	if ( !a_pMemBlock )
	{
		a_pMemBlock = Toshi::g_pMemory->GetGlobalBlock();
	}

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, a_iAlignment, a_pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
	}

	return pMem;
}

void* TMemalign( TUINT a_uiSize, TINT a_iAlignment )
{
	auto pMemBlock = Toshi::g_pMemory->GetGlobalBlock();

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, a_iAlignment, pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
	}

	return pMem;
}

void TFree( void* a_pMem )
{
	Toshi::g_pMemory->Free( a_pMem );
}
