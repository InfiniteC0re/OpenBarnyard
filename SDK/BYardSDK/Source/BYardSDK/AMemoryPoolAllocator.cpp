#include "pch.h"
#include "AMemoryPoolAllocator.h"

void* AMemoryPoolAllocator::Malloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( alignment, size, m_pMemBlock );
}

void* AMemoryPoolAllocator::Malloc( TSIZE size )
{
	return TMalloc( size, m_pMemBlock );
}

void AMemoryPoolAllocator::Free( void* ptr )
{
	TFree( ptr );
}

TBOOL AMemoryPoolAllocator::CanAllocate( TSIZE size ) const
{
	return TTRUE;
}

void* AMemoryPoolAllocator::TryMalloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( alignment, size, m_pMemBlock );
}

void* AMemoryPoolAllocator::TryMalloc( TSIZE size )
{
	return TMalloc( size, m_pMemBlock );
}

TSIZE AMemoryPoolAllocator::GetUsedSize() const
{
	return 0;
}

TSIZE AMemoryPoolAllocator::GetCapacity() const
{
	return 0;
}

void AMemoryPoolAllocator::Unknown()
{
	TASSERT( TFALSE );
}
