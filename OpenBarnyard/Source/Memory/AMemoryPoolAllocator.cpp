#include "pch.h"
#include "AMemoryPoolAllocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void* AMemoryPoolAllocator::Malloc(size_t size, size_t alignment)
{
	return TMemalign(alignment, size, m_pMemBlock);
}

void* AMemoryPoolAllocator::Malloc(size_t size)
{
	return TMalloc(size, m_pMemBlock);
}

void AMemoryPoolAllocator::Free(void* ptr)
{
	TFree(ptr);
}

TBOOL AMemoryPoolAllocator::CanAllocate(size_t size) const
{
	return TTRUE;
}

void* AMemoryPoolAllocator::TryMalloc(size_t size, size_t alignment)
{
	return TMemalign(alignment, size, m_pMemBlock);
}

void* AMemoryPoolAllocator::TryMalloc(size_t size)
{
	return TMalloc(size, m_pMemBlock);
}

size_t AMemoryPoolAllocator::GetUsedSize() const
{
	return 0;
}

size_t AMemoryPoolAllocator::GetCapacity() const
{
	return 0;
}

void AMemoryPoolAllocator::Unknown()
{
	TASSERT(TFALSE);
}
