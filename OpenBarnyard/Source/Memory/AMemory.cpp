#include "pch.h"
#include "AMemory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TMemory::MemBlock* AMemory::s_aMemBlocks[ POOL_NUMOF ];
AMemory::AllocatorList    AMemory::s_AllocatorList;

// $Barnyard: FUNCTION 00402910
void AMemory::CreatePool( POOL a_ePool )
{
	TASSERT( a_ePool >= POOL_None && a_ePool < POOL_NUMOF );

	auto&              rPool        = s_aPools[ a_ePool ];
	TMemory::MemBlock* pAllocOnHeap = ( rPool.GetHeapIndex() == a_ePool ) ?
	    TNULL :
	    s_aMemBlocks[ rPool.GetHeapIndex() ];

	s_aMemBlocks[ a_ePool ] = g_pMemory->CreateMemBlock(
	    rPool.GetSize(),
	    rPool.GetName(),
	    pAllocOnHeap,
	    rPool.GetUnk2()
	);

	s_AllocatorList.aAllocators[ a_ePool ].SetMemBlock( s_aMemBlocks[ a_ePool ] );
}

// $Barnyard: FUNCTION 00402970
void AMemory::DestroyPool( POOL a_ePool )
{
	TASSERT( a_ePool >= POOL_None && a_ePool < POOL_NUMOF );

	g_pMemory->DestroyMemBlock( s_aMemBlocks[ a_ePool ] );

	// NOTE: retail stores the result of a stub that returns 1 here, which is never used as a memory block
	s_aMemBlocks[ a_ePool ] = TNULL;
	s_AllocatorList.aAllocators[ a_ePool ].SetMemBlock( TNULL );
}

T2Allocator* AMemory::GetAllocator( POOL a_ePool )
{
	TASSERT( a_ePool >= POOL_None && a_ePool < POOL_NUMOF );

	return &s_AllocatorList.aAllocators[ a_ePool ];
}

TMemory::MemBlock* AMemory::GetMemBlock( POOL a_ePool )
{
	TASSERT( a_ePool >= POOL_None && a_ePool < POOL_NUMOF );

	return s_aMemBlocks[ a_ePool ];
}
