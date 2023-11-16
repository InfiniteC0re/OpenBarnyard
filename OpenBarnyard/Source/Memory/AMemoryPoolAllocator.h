#pragma once
#include <Toshi/Memory/TMemory.h>

class AMemoryPoolAllocator : public Toshi::T2Allocator
{
public:
	virtual void* Malloc(size_t size, size_t alignment) override;
	virtual void* Malloc(size_t size) override;
	virtual void Free(void* ptr) override;
	virtual TBOOL CanAllocate(size_t size) override;
	virtual void* TryMalloc(size_t size, size_t alignment) override;
	virtual void* TryMalloc(size_t size) override;
	virtual size_t GetUsedSize() override;
	virtual size_t GetCapacity() override;
	virtual void Unknown();

	void SetHeap(Toshi::TMemoryHeap* a_pHeap)
	{
		m_pMemoryHeap = a_pHeap;
	}

private:
	Toshi::TMemoryHeap* m_pMemoryHeap;
};
