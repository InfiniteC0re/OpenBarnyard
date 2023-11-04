#include "pch.h"
#include "AMemory.h"

TOSHI_NAMESPACE_USING

void AMemory::CreatePool(POOL a_ePool)
{
	TASSERT(a_ePool >= POOL_None && a_ePool < POOL_NUMOF);

	auto& pool = s_aPools[a_ePool];
	Toshi::TMemoryHeap* pAllocOnHeap = (pool.GetHeapIndex() == a_ePool) ? TNULL : s_aHeaps[pool.GetHeapIndex()];

	s_aHeaps[a_ePool] = TMemory::CreateHeap(pool.GetSize(), TMemoryHeapFlags_UseMutex, pool.GetName(), pAllocOnHeap);
	s_aAllocators[a_ePool].pMemoryHeap = s_aHeaps[a_ePool];
}

Toshi::T2GlobalAllocator* AMemory::GetAllocator(POOL a_ePool)
{
	TASSERT(a_ePool >= POOL_None && a_ePool < POOL_NUMOF);

	return &s_aAllocators[a_ePool].Allocator;
}
