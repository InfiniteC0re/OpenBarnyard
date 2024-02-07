#include "pch.h"
#include "AMemory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void AMemory::CreatePool(POOL a_ePool)
{
	TASSERT(a_ePool >= POOL_None && a_ePool < POOL_NUMOF);

	auto& rPool = s_aPools[a_ePool];
	TMemory::MemBlock* pAllocOnHeap = (rPool.GetHeapIndex() == a_ePool) ?
		TNULL : s_aMemBlocks[rPool.GetHeapIndex()];

	s_aMemBlocks[a_ePool] = TMemory::GetSingleton()->CreateMemBlock(
		rPool.GetSize(),
		rPool.GetName(),
		pAllocOnHeap
	);

	s_AllocatorList.aAllocators[a_ePool].SetMemBlock(s_aMemBlocks[a_ePool]);
}

T2Allocator* AMemory::GetAllocator(POOL a_ePool)
{
	TASSERT(a_ePool >= POOL_None && a_ePool < POOL_NUMOF);

	return &s_AllocatorList.aAllocators[a_ePool];
}

TMemory::MemBlock* AMemory::GetMemBlock(POOL a_ePool)
{
	TASSERT(a_ePool >= POOL_None && a_ePool < POOL_NUMOF);

	return s_aMemBlocks[a_ePool];
}
