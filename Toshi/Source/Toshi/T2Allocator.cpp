#include "ToshiPCH.h"
#include "T2Allocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2GlobalAllocator T2Allocator::s_GlobalAllocator;

void* T2GlobalAllocator::Malloc( TSIZE size )
{
	return TMalloc( size );
}

void* T2GlobalAllocator::Malloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( size, alignment );
}

void T2GlobalAllocator::Free( void* ptr )
{
	TFree( ptr );
}

TBOOL T2GlobalAllocator::CanAllocate( TSIZE size ) const
{
	return TTRUE;
}

void* T2GlobalAllocator::TryMalloc( TSIZE size )
{
	return TMalloc( size );
}

void* T2GlobalAllocator::TryMalloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( size, alignment );
}

TSIZE T2GlobalAllocator::GetUsedSize() const
{
	return 0;
}

TSIZE T2GlobalAllocator::GetCapacity() const
{
	return 0;
}

TOSHI_NAMESPACE_END
