#include "pch.h"
#include "AMemoryPoolAllocator.h"

void* AMemoryPoolAllocator::Malloc(size_t size, size_t alignment)
{
	return m_pMemoryHeap ? m_pMemoryHeap->Memalign(alignment, size) : TMemalign(alignment, size);
}

void* AMemoryPoolAllocator::Malloc(size_t size)
{
	return m_pMemoryHeap ? m_pMemoryHeap->Malloc(size) : TMalloc(size);
}

void AMemoryPoolAllocator::Free(void* ptr)
{
	return m_pMemoryHeap ? m_pMemoryHeap->Free(ptr) : TFree(ptr);
}

TBOOL AMemoryPoolAllocator::CanAllocate(size_t size)
{
	return TTRUE;
}

void* AMemoryPoolAllocator::TryMalloc(size_t size, size_t alignment)
{
	return m_pMemoryHeap ? m_pMemoryHeap->Memalign(alignment, size) : TMemalign(alignment, size);
}

void* AMemoryPoolAllocator::TryMalloc(size_t size)
{
	return m_pMemoryHeap ? m_pMemoryHeap->Malloc(size) : TMalloc(size);
}

size_t AMemoryPoolAllocator::GetUsedSize()
{
	return 0;
}

size_t AMemoryPoolAllocator::GetCapacity()
{
	return 0;
}

void AMemoryPoolAllocator::Unknown()
{
	TASSERT(TFALSE);
}
