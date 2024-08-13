#pragma once
#include <Core/TMemory.h>

class AMemoryPoolAllocator : public Toshi::T2Allocator
{
public:
	virtual void*  Malloc( size_t size, size_t alignment ) override;
	virtual void*  Malloc( size_t size ) override;
	virtual void   Free( void* ptr ) override;
	virtual TBOOL  CanAllocate( size_t size ) const override;
	virtual void*  TryMalloc( size_t size, size_t alignment ) override;
	virtual void*  TryMalloc( size_t size ) override;
	virtual size_t GetUsedSize() const override;
	virtual size_t GetCapacity() const override;
	virtual void   Unknown();

	void SetMemBlock( Toshi::TMemory::MemBlock* a_pMemBlock )
	{
		m_pMemBlock = a_pMemBlock;
	}

private:
	Toshi::TMemory::MemBlock* m_pMemBlock;
};
