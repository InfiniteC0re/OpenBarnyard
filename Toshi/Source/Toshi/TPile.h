#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a pile of fixed size memory blocks. Allocations are taken from the
// current block until it's full, a new block is allocated then. Single
// allocations can't be freed, only the whole pile (Reset keeps the blocks to
// reuse them, Free releases them).
//-----------------------------------------------------------------------------
class TPile
{
public:
	enum Flags_ : TUINT
	{
		Flags_None        = 0,
		Flags_AlignBlocks = BITFLAG( 0 ), // blocks are allocated aligned to 32 bytes
	};

public:
	// a_iBlockSize: size of a block, a_iMaxSize: maximum memory size of the pile (0 for no limit)
	TPile( TINT a_iBlockSize, TINT a_iMaxSize, TUINT a_uiFlags );
	~TPile();

	// Allocates a_iSize bytes aligned to a_iAlignment, returns TNULL if the pile reached its maximum size
	void* RawAlloc( TINT a_iSize, TINT a_iAlignment );

	// Checks if a_pMem belongs to one of the blocks of the pile
	TBOOL RawIsInside( void* a_pMem );

	// Frees all the blocks
	void Free();

	// Makes all the blocks available again (keeps them allocated)
	void Reset();

	// $JPOG: FUNCTION TKernelInterface.dll+00021880
	TINT GetNumAllocs() const { return m_iNumAllocs; }

	// $JPOG: FUNCTION TKernelInterface.dll+00021890
	TINT GetNumBlocks() const { return m_iNumBlocks; }

	// $JPOG: FUNCTION TKernelInterface.dll+00021870
	TINT GetWastedSpace() const { return m_iWastedSpace; }

private:
	// Header of a block, the memory of the block follows it
	struct Block
	{
		Block* pNext;
	};

private:
	TINT   m_iBlockSize;     // 0x00
	TINT   m_iMaxNumBlocks;  // 0x04
	TUINT  m_uiFlags;        // 0x08
	TINT   m_iNumBlocks;     // 0x0C
	TINT   m_iNumUsedBlocks; // 0x10
	TUINT8* m_pCurrent;      // 0x14
	TUINT8* m_pEnd;          // 0x18
	Block* m_pFirstBlock;    // 0x1C
	Block* m_pCurrentBlock;  // 0x20
	TINT   m_iNumAllocs;     // 0x24
	TINT   m_iWastedSpace;   // 0x28 never updated
};

TOSHI_NAMESPACE_END
