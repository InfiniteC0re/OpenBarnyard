#include "ToshiPCH.h"
#include "TMemory.h"
#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

#ifdef TOSHI_PROFILER_MEMORY
//#include "Profiler/tracy/Tracy.hpp"
#endif // TOSHI_PROFILER_MEMORY

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

#ifndef TMEMORY_USE_DLMALLOC

#define MEM_TO_NODE(PTR) (((MemNode*)(((TUINT)PTR) + sizeof(void*))) - 1)

TOSHI_NAMESPACE_START

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
	//return CALL_THIS( 0x006b5230, TMemory*, void*, this, TUINT, a_uiSize, TINT, a_uiAlignment, MemBlock*, a_pMemBlock, const TCHAR*, a_szFileName, TINT, a_iLineNum );

	TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

	volatile TUINT uiSize = a_uiSize;
	volatile TUINT uiAlignment = a_uiAlignment;
	MemBlock* pMemBlock = a_pMemBlock;

	if ( uiSize < 4 )
		uiSize = 4;

	if ( uiAlignment < 16 )
		uiAlignment = 16;

	if ( uiAlignment < TMEMORY_ROUNDUP )
	{
		TDebug_FinalPrintf( "MEMORY ERROR: CANT ALLOC Alignment(%d)<TMEMORY_ROUNDUP\n", uiAlignment );
		DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
		TASSERT( TFALSE );
		return TNULL;
	}

	if ( !pMemBlock )
	{
		pMemBlock = m_pGlobalBlock;
	}

	volatile TUINT uiAlignedSize = TAlignNumUp( uiSize );
	volatile TUINT uiRequiredSize = uiAlignedSize + sizeof( MemNode );

	MemNode* pMemNode = TNULL;

	volatile void* pAllocatedAddress;
	volatile TUINTPTR uiDataRegionStart;
	volatile TUINTPTR uiDataRegionEnd;
	volatile TUINTPTR uiDataRegionSize;

	volatile TUINT uiNodeId = MapSizeToFreeList( uiAlignedSize );

	// Find a hole that can allocate the required number of bytes
	for ( ; uiNodeId < TMEMORY_NUM_FREELISTS; uiNodeId++ )
	{
		pMemNode = pMemBlock->m_apHoles[ uiNodeId ];

		while ( pMemNode != TNULL )
		{
			pAllocatedAddress = TAlignPointerUp( pMemNode->GetDataRegionStart(), uiAlignment );
			uiDataRegionStart = TREINTERPRETCAST( TUINTPTR, pAllocatedAddress );
			uiDataRegionEnd = TREINTERPRETCAST( TUINTPTR, pMemNode->GetDataRegionEnd() );
			uiDataRegionSize = uiDataRegionEnd - uiDataRegionStart;

			if ( uiDataRegionEnd > uiDataRegionStart && uiDataRegionSize >= uiAlignedSize )
				break;

			// This freelist can't be used, let's check for the next
			pMemNode = pMemNode->pNextHole;
		}

		if ( pMemNode )
			break;
	}

	if ( !pMemNode )
	{
		DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
		DebugPrintHALMemInfo( "Requested memory block size: %d\n", uiAlignedSize );
		DumpMemInfo();

		TASSERT( TFALSE );
		return TNULL;
	}

	MemNode* pAllocNode = MEM_TO_NODE( pAllocatedAddress );

	if ( volatile MemNode* pOwner = pMemNode->pOwner )
	{
		pOwner->uiSize = ( uiDataRegionStart - (TUINTPTR)pOwner - 24 ) | pOwner->uiSize & TMEMORY_FLAGS_MASK;
	}
	else if ( pAllocNode != pMemNode )
	{
		// Seems that due to alignment we have a gap between start of the
		// data region and the actual address we gonna return so let's
		// make sure we don't lost this pointer
		pMemBlock->m_pFirstHole = pAllocNode;
	}

	// Check if we can split the hole in two
	if ( uiDataRegionSize > uiRequiredSize )
	{
		// We can split it!

		// Unlink the hole from the linked list
		if ( volatile MemNode* pPrev = pMemNode->pPrevHole )
			pPrev->pNextHole = pMemNode->pNextHole;
		else
			pMemBlock->m_apHoles[ uiNodeId ] = pMemNode->pNextHole;

		// Remove reference to this hole from the next one
		if ( volatile MemNode* pNext = pMemNode->pNextHole )
			pNext->pPrevHole = pMemNode->pPrevHole;

		if ( pMemNode != pAllocNode )
		{
			TUINT uiNodeSize = GetNodeSize( pMemNode );
			pAllocNode->pOwner = pMemNode->pOwner;
			*(MemNode**)( (TUINTPTR)pMemNode->GetDataRegionStart() + uiNodeSize ) = pAllocNode;
		}

		// Create a new hole right after the allocated data
		MemNode* pNewNode = TREINTERPRETCAST( MemNode*, uiDataRegionStart + uiAlignedSize );

		SetProcess( pMemBlock, pAllocNode, uiDataRegionSize );
		SetHoleSize( pNewNode, (TUINTPTR)pAllocNode + GetNodeSize( pAllocNode ) - uiDataRegionStart - uiAlignedSize );
		SetProcess( pMemBlock, pAllocNode, uiAlignedSize + uiDataRegionStart - (TUINTPTR)pAllocNode - TMEMORY_ALLOC_RESERVED_SIZE );
		pNewNode->pOwner = pAllocNode;

		// Place the new hole in the memblock's list

		TUINT uiNewHoleId = MapSizeToFreeList( GetNodeSize( pNewNode ) );

		MemNode* pOldNode = pMemBlock->m_apHoles[ uiNewHoleId ];
		pNewNode->pNextHole = pOldNode;

		if ( pOldNode )
			pOldNode->pPrevHole = pNewNode;

		pNewNode->pPrevHole = TNULL;
		pMemBlock->m_apHoles[ uiNewHoleId ] = pNewNode;

		// Save pointer to the hole right at the end of the data region (probably for some validation)
		*(MemNode* volatile*)pNewNode->GetDataRegionEnd() = pNewNode;

		return (void*)pAllocatedAddress;
	}
	else
	{
		// Damn, we can't split this one but it surely can fit the allocation

		// Unlink the hole from the linked list
		if ( volatile MemNode* pPrev = pMemNode->pPrevHole )
			pPrev->pNextHole = pMemNode->pNextHole;
		else
			pMemBlock->m_apHoles[ uiNodeId ] = pMemNode->pNextHole;

		// Remove reference to this hole from the next one
		if ( volatile MemNode* pNext = pMemNode->pNextHole )
			pNext->pPrevHole = pMemNode->pPrevHole;

		if ( pMemNode != pAllocNode )
		{
			TUINT uiNodeSize = GetNodeSize( pMemNode );
			pAllocNode->pOwner = pMemNode->pOwner;
			*(MemNode* volatile*)( (TUINTPTR)pMemNode->GetDataRegionStart() + uiNodeSize ) = pAllocNode;
		}

		SetProcess( pMemBlock, pAllocNode, uiDataRegionSize );

		return (void*)pAllocatedAddress;
	}
}

TBOOL TMemory::Free( const void* a_pAllocated )
{
	//return CALL_THIS( 0x006b4a20, TMemory*, TBOOL, this, const void*, a_pAllocated );

	TMUTEX_LOCK_SCOPE( ms_pGlobalMutex );

	if ( !a_pAllocated || !TIsPointerAligned( a_pAllocated ) )
	{
		// Can't free TNULL or unaligned pointer
		return TFALSE;
	}

	MemNode* pAllocationNode = MEM_TO_NODE( a_pAllocated );

	TUINT uiAllocationSize = GetNodeSize( pAllocationNode );
	MemBlock* pMemBlock = GetProcessMemBlock( pAllocationNode );

	SetHoleSize( pAllocationNode, uiAllocationSize );

	MemNode* pNextLyingNode = (MemNode*)pAllocationNode->GetDataRegionEnd();
	MemNode* pOwner = pAllocationNode->pOwner;

	MemNode** ppNextNode = &pAllocationNode->pNextHole;

	if ( pOwner && !IsProcess( pOwner ) )
	{
		pAllocationNode = pOwner;
		TUINT uiOwnerSize = GetNodeSize( pOwner );

		ExtendNodeSize( pOwner, uiAllocationSize + uiOwnerSize + TMEMORY_ALLOC_RESERVED_SIZE );
		pNextLyingNode->pOwner = pOwner;
		ppNextNode = &pOwner->pNextHole;

		if ( MemNode* pPrev = pOwner->pPrevHole )
			pPrev->pNextHole = pOwner->pNextHole;
		else
			pMemBlock->m_apHoles[ MapSizeToFreeList( uiOwnerSize ) ] = pOwner->pNextHole;

		if ( MemNode* pNext = pOwner->pNextHole )
			pNext->pPrevHole = pOwner->pPrevHole;
	}

	if ( !IsProcess( pNextLyingNode ) )
	{
		TUINT uiNextNodeSize = GetNodeSize( pNextLyingNode );

		if ( MemNode* pPrev = pNextLyingNode->pPrevHole )
			pPrev->pNextHole = pNextLyingNode->pNextHole;
		else
			pMemBlock->m_apHoles[ MapSizeToFreeList( uiNextNodeSize ) ] = pNextLyingNode->pNextHole;

		if ( MemNode* pNext = pNextLyingNode->pNextHole )
			pNext->pPrevHole = pNextLyingNode->pPrevHole;

		ExtendNodeSize( pAllocationNode, GetNodeSize( pAllocationNode ) + GetNodeSize( pNextLyingNode ) + TMEMORY_ALLOC_RESERVED_SIZE );
		*(MemNode**)pNextLyingNode->GetDataRegionEnd() = pAllocationNode;
	}

	TUINT uiId = MapSizeToFreeList( GetNodeSize( pAllocationNode ) );
	pAllocationNode->pPrevHole = TNULL;
	*ppNextNode = pMemBlock->m_apHoles[ uiId ];

	if ( *ppNextNode )
		( *ppNextNode )->pPrevHole = pAllocationNode;

	pMemBlock->m_apHoles[ uiId ] = pAllocationNode;

	return TTRUE;
}

TMemory::MemBlock* TMemory::CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock, TINT a_iUnused )
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
			constexpr TUINT CHUNK_RESERVED_SIZE = ( sizeof( MemBlock ) + ( sizeof( MemNode ) - sizeof( void* ) ) );

			pBlock->m_uiTotalSize1 = uiBlockTotalSize;
			TUtil::MemClear( pBlock->m_apHoles, sizeof( pBlock->m_apHoles ) );
			pBlock->m_pFirstHole = &pBlock->m_RootHole;

			auto pHole = &pBlock->m_RootHole;
			pHole->uiSize = uiBlockTotalSize - CHUNK_RESERVED_SIZE;
			pHole->pNextHole = TNULL;
			pHole->pPrevHole = TNULL;
			pHole->pOwner = TNULL;

			auto uiFreeListId = MapSizeToFreeList( uiBlockTotalSize - CHUNK_RESERVED_SIZE );
			pBlock->m_apHoles[ uiFreeListId ] = pHole;

			auto pBlockFooter = TREINTERPRETCAST( MemBlockFooter*, TREINTERPRETCAST( TUINT32, pBlock ) + uiBlockTotalSize ) - 1;
			pBlockFooter->m_pBlockOwner = TNULL;
			pBlockFooter->m_Unk4 = 0;
			pBlockFooter->m_Unk1 = 0;
			pBlockFooter->m_Unk2 = g_pMemory->m_uiGlobalFlags | TMEMORY_FLAGS_HOLE_PROCESS;
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

Toshi::TMemory::MemNode* TMemory::GetMemNodeFromAddress( void* a_pMem )
{
	if ( !a_pMem || HASANYFLAG( TREINTERPRETCAST( TUINTPTR, a_pMem ), TMEMORY_FLAGS_MASK ) )
		return TNULL;

	MemNode* pMemNode = MEM_TO_NODE( a_pMem );
	TVALIDPTR( pMemNode );

	return pMemNode;
}

int TMemory::TestMemIntegrity( MemBlock* a_pMemBlock )
{
	// TODO: this method is cut in the release build
	return 0;
}

int TMemory::DebugTestMemoryBlock( MemBlock* a_pMemBlock )
{
	TIMPLEMENT();
	DebugPrintHALMemInfo( "TMemory: DebugTestMemoryBlock:" );

	MemBlock* pMemBlock = a_pMemBlock;

	if ( !pMemBlock )
	{
		pMemBlock = g_pMemory->m_pGlobalBlock;
	}

	MemInfo memInfo;
	GetMemInfo( memInfo, pMemBlock );

	// ...

	return 0;
}

TBOOL TMemory::Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize, TUINT a_uiUnused )
{
	auto tmemory = TSTATICCAST( TMemory, calloc( sizeof( TMemory ), 1 ) );
	new ( tmemory ) TMemory();

	tmemory->m_pMemory = TNULL;
	tmemory->m_pGlobalBlock = TNULL;
	tmemory->m_uiGlobalFlags = 0;
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
	TUINT uiHoleSize = pHole->uiSize;

	while ( TAlignNumDown( uiHoleSize ) != 0 )
	{
		a_rMemInfo.m_uiUnk3 += sizeof( MemNode ) - sizeof( void* );
		uiHoleSize = TAlignNumDown( pHole->uiSize );

		if ( ( pHole->uiSize & 1 ) == 0 )
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
		pHole = (MemNode*)( (TUINT)&pHole->pPrevHole + uiHoleSize );
		uiHoleSize = pHole->uiSize;
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

TOSHI_NAMESPACE_END

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

void* TMemalign( TUINT a_uiAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock )
{
	if ( !a_pMemBlock )
	{
		a_pMemBlock = Toshi::g_pMemory->GetGlobalBlock();
	}

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, a_uiAlignment, a_pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
	}

	return pMem;
}

void* TMemalign( TUINT a_uiSize, TUINT a_uiAlignment )
{
	auto pMemBlock = Toshi::g_pMemory->GetGlobalBlock();

	auto pMem = Toshi::g_pMemory->Alloc( a_uiSize, a_uiAlignment, pMemBlock, TNULL, -1 );

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

#else  // !TMEMORY_USE_DLMALLOC

TOSHI_NAMESPACE_START

TMemory* g_pMemory = TNULL;

TMemory::TMemory()
{
	m_pMemModule = TNULL;
}

TBOOL TMemory::Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize, TUINT a_uiUnused/* = 0*/)
{
	TASSERT( g_pMemory == TNULL );

	// Allocate TMemory
	TMemory* pMemory = (TMemory*)GlobalAlloc( GMEM_ZEROINIT, sizeof( TMemory ) );

	new ( pMemory ) TMemory();
	g_pMemory = pMemory;

	// Allocate TMemory_dlmalloc
	TMemoryDL* pMemModule = (TMemoryDL*)GlobalAlloc( GMEM_ZEROINIT, sizeof( TMemoryDL ) );

	// Set minimum heap size
	if ( a_uiHeapSize == 0 )
		a_uiHeapSize = 128 * 1024 * 1024;

	new ( pMemModule ) TMemoryDL( TMemoryDL::Flags_Standard, a_uiHeapSize + a_uiReservedSize );
	g_pMemory->m_pMemModule = pMemModule;
	g_pMemoryDL = pMemModule;

	// Initialise the memory block
	TBOOL bInitialised = pMemModule->Init() == TMemoryDL::Error_Ok;

	g_pMemory->m_pGlobalBlock = g_pMemoryDL->GetHeap();

	return bInitialised;
}

void TMemory::Deinitialise()
{
	TASSERT( g_pMemory != TNULL );

	g_pMemory->m_pMemModule->Shutdown();

	GlobalFree( g_pMemory->m_pMemModule );
	GlobalFree( g_pMemory );

	g_pMemory = TNULL;
}

TMemory::MemBlock* TMemory::CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock, TINT a_iUnused )
{
	if ( !a_pOwnerBlock )
		a_pOwnerBlock = m_pMemModule->GetHeap();

	return g_pMemoryDL->dlheapcreatesubheap( a_pOwnerBlock, a_uiSize, TMemoryHeapFlags_UseMutex, a_szName );
}

TMemory::MemBlock* TMemory::CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName )
{
	TVALIDPTR( a_pMem );

	return g_pMemoryDL->dlheapcreateinplace( a_pMem, a_uiSize, TMemoryHeapFlags_UseMutex, a_szName );
}

void TMemory::DestroyMemBlock( MemBlock* a_pMemBlock )
{
	g_pMemoryDL->dlheapdestroy( a_pMemBlock );
}

TMemory::MemBlock* TMemory::GetGlobalBlock() const
{
	return g_pMemoryDL->GetHeap();
}

TMemory::MemBlock* TMemory::SetGlobalBlock( MemBlock* a_pMemBlock )
{
	MemBlock* pOldHeap = g_pMemoryDL->GetHeap();
	m_pGlobalBlock = a_pMemBlock;
	g_pMemoryDL->SetHeap( a_pMemBlock );
	return pOldHeap;
}

void TMemory::GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock )
{
	TUtil::MemClear( &a_rMemInfo, sizeof( a_rMemInfo ) );
}

void TMemory::GetHALMemInfo( HALMemInfo& a_rHALMemInfo )
{
	TUtil::MemClear( &a_rHALMemInfo, sizeof( a_rHALMemInfo ) );
}

TOSHI_NAMESPACE_END

void* TMalloc( TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum )
{
	if ( !a_pMemBlock )
	{
		a_pMemBlock = Toshi::g_pMemoryDL->GetHeap();
	}

	return a_pMemBlock->Malloc( a_uiSize );
}

void* TMalloc( TUINT a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum )
{
	return Toshi::g_pMemoryDL->GetContext().Malloc( a_uiSize );
}

void* TMalloc( TUINT a_uiSize )
{
	return Toshi::g_pMemoryDL->GetContext().Malloc( a_uiSize );
}

void* TMemalign( TUINT a_uiAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock )
{
	if ( !a_pMemBlock )
	{
		a_pMemBlock = Toshi::g_pMemoryDL->GetHeap();
	}

	return a_pMemBlock->Memalign( a_uiAlignment, a_uiSize );
}

void* TMemalign( TUINT a_uiSize, TUINT a_uiAlignment )
{
	return Toshi::g_pMemoryDL->GetContext().Memalign( a_uiAlignment, a_uiSize );
}

void TFree( void* a_pMem )
{
	Toshi::g_pMemoryDL->GetContext().Free( a_pMem );
}

#endif // TMEMORY_USE_DLMALLOC
