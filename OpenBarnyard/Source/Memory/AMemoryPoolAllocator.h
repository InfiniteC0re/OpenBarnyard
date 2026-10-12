#pragma once
#include <Core/TMemory.h>

class AMemoryPoolAllocator : public Toshi::T2Allocator
{
public:
	// NOTE: TryMalloc shares the bodies of Malloc in the original
	virtual void*  Malloc( size_t size, size_t alignment ) override;           // +0x00
	virtual void*  Malloc( size_t size ) override;                             // +0x04
	virtual void   Free( void* ptr ) override;                                 // +0x08
	virtual TBOOL  CanAllocate( size_t size ) const override { return TTRUE; } // +0x0C, RETURNS_TRUE 00413920
	virtual void*  TryMalloc( size_t size, size_t alignment ) override;        // +0x10
	virtual void*  TryMalloc( size_t size ) override;                          // +0x14
	virtual size_t GetUsedSize() const override { return 0; }                  // +0x18, RETURNS_ZERO 0054d8b0
	virtual size_t GetCapacity() const override { return 0; }                  // +0x1C, RETURNS_ZERO 0054d8b0
	virtual void*  Realloc( void* a_pMem, size_t a_uiSize );                   // +0x20

	void SetMemBlock( Toshi::TMemory::MemBlock* a_pMemBlock )
	{
		m_pMemBlock = a_pMemBlock;
	}

private:
	Toshi::TMemory::MemBlock* m_pMemBlock;
};
