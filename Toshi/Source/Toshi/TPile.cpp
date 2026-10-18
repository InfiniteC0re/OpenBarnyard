#include "ToshiPCH.h"
#include "TPile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $JPOG: FUNCTION TKernelInterface.dll+00021680
TPile::TPile( TINT a_iBlockSize, TINT a_iMaxSize, TUINT a_uiFlags )
    : m_iBlockSize( a_iBlockSize )
    , m_iMaxNumBlocks( a_iMaxSize / a_iBlockSize )
    , m_uiFlags( a_uiFlags )
    , m_iNumBlocks( 0 )
    , m_iNumUsedBlocks( 0 )
    , m_pCurrent( TNULL )
    , m_pEnd( TNULL )
    , m_pFirstBlock( TNULL )
    , m_pCurrentBlock( TNULL )
    , m_iNumAllocs( 0 )
    , m_iWastedSpace( 0 )
{
}

// NOTE: doesn't free the blocks
// $JPOG: FUNCTION TKernelInterface.dll+00021860
TPile::~TPile()
{
}

// $JPOG: FUNCTION TKernelInterface.dll+00021790
void* TPile::RawAlloc( TINT a_iSize, TINT a_iAlignment )
{
	m_iNumAllocs++;

	TUINTPTR uiAlignMask = ~TUINTPTR( a_iAlignment - 1 );
	TUINT8*  pMem        = TREINTERPRETCAST( TUINT8*, ( TREINTERPRETCAST( TUINTPTR, m_pCurrent ) + a_iAlignment - 1 ) & uiAlignMask );
	m_pCurrent           = pMem + a_iSize;

	if ( m_pCurrent > m_pEnd )
	{
		// The current block is full, use the next one
		Block* pBlock;
		TINT   iNumUsedBlocks = m_iNumUsedBlocks;

		if ( iNumUsedBlocks < m_iNumBlocks )
		{
			// Reuse a block that was kept by Reset
			pBlock          = m_pCurrentBlock->pNext;
			m_pCurrentBlock = pBlock;
		}
		else
		{
			if ( m_iMaxNumBlocks != 0 && m_iNumBlocks >= m_iMaxNumBlocks )
				return TNULL;

			if ( m_uiFlags & Flags_AlignBlocks )
				pBlock = TSTATICCAST( Block, TMemalign( ( m_iBlockSize + sizeof( Block ) + 31 ) & ~31, 32 ) );
			else
				pBlock = TSTATICCAST( Block, TMalloc( m_iBlockSize + sizeof( Block ) ) );

			pBlock->pNext = TNULL;

			if ( m_pCurrentBlock )
				m_pCurrentBlock->pNext = pBlock;
			else
				m_pFirstBlock = pBlock;

			m_pCurrentBlock = pBlock;
			iNumUsedBlocks  = m_iNumUsedBlocks;
			m_iNumBlocks++;
		}

		m_pEnd           = TREINTERPRETCAST( TUINT8*, pBlock + 1 ) + m_iBlockSize;
		pMem             = TREINTERPRETCAST( TUINT8*, ( TREINTERPRETCAST( TUINTPTR, pBlock ) + a_iAlignment + sizeof( Block ) - 1 ) & uiAlignMask );
		m_iNumUsedBlocks = iNumUsedBlocks + 1;
		m_pCurrent       = pMem + a_iSize;
	}

	return pMem;
}

// $JPOG: FUNCTION TKernelInterface.dll+000216c0
TBOOL TPile::RawIsInside( void* a_pMem )
{
	for ( Block* pBlock = m_pFirstBlock; pBlock; pBlock = pBlock->pNext )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: JPOG adds the block size to a DWORD pointer, so the range checked is 4 times the size
		// of the block and memory allocated outside of the pile right after a block would be leaked
		if ( a_pMem > pBlock && a_pMem < TREINTERPRETCAST( TUINT8*, pBlock ) + m_iBlockSize )
			return TTRUE;
#else
		if ( a_pMem > pBlock && a_pMem < TREINTERPRETCAST( TUINT32*, pBlock ) + m_iBlockSize )
			return TTRUE;
#endif
	}

	return TFALSE;
}

// $JPOG: FUNCTION TKernelInterface.dll+00021700
void TPile::Free()
{
	while ( m_pFirstBlock )
	{
		Block* pNext = m_pFirstBlock->pNext;
		TFree( m_pFirstBlock );
		m_pFirstBlock = pNext;
	}

	m_iNumBlocks = 0;
	Reset();
}

// $JPOG: FUNCTION TKernelInterface.dll+00021750
void TPile::Reset()
{
	if ( m_pFirstBlock )
	{
		m_pCurrent       = TREINTERPRETCAST( TUINT8*, m_pFirstBlock + 1 );
		m_pEnd           = m_pCurrent + m_iBlockSize;
		m_iNumUsedBlocks = 1;
	}
	else
	{
		m_pCurrent       = TNULL;
		m_pEnd           = TNULL;
		m_iNumUsedBlocks = 0;
	}

	m_pCurrentBlock = m_pFirstBlock;
	m_iNumAllocs    = 0;
	m_iWastedSpace  = 0;
}

TOSHI_NAMESPACE_END
