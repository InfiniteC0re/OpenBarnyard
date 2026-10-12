#include "pch.h"
#include "AMemoryPoolAllocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// $Barnyard: FUNCTION 004028f0
void* AMemoryPoolAllocator::Malloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( alignment, size, m_pMemBlock );
}

// $Barnyard: FUNCTION 004028d0
void* AMemoryPoolAllocator::Malloc( TSIZE size )
{
	return TMalloc( size, m_pMemBlock );
}

// $Barnyard: FUNCTION 004028c0
void AMemoryPoolAllocator::Free( void* ptr )
{
	TFree( ptr );
}

void* AMemoryPoolAllocator::TryMalloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( alignment, size, m_pMemBlock );
}

void* AMemoryPoolAllocator::TryMalloc( TSIZE size )
{
	return TMalloc( size, m_pMemBlock );
}

// $Barnyard: FUNCTION 004028a0
void* AMemoryPoolAllocator::Realloc( void* a_pMem, TSIZE a_uiSize )
{
	// NOTE: TRealloc (006b4e00) is inlined. It ignores both arguments, reports the block as out of memory and returns TNULL
	Toshi::TMemory::MemBlock* pMemBlock = m_pMemBlock ? m_pMemBlock : Toshi::g_pMemory->GetGlobalBlock();
	Toshi::TMemory::DebugPrintHALMemInfo( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );

	return TNULL;
}
