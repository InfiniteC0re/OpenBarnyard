#include "pch.h"
#include "AObjectHash.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0061f760
AObjectHash::AObjectHash( TINT a_iNumNodes )
{
	m_fTotalX                  = 0.0f;
	m_fTotalY                  = 0.0f;
	m_fNegativeMinX            = 0.0f;
	m_fNegativeMinY            = 0.0f;
	m_fNumCellsX               = 0;
	m_fNumCellsY               = 0;
	m_fMaxXOverNumCells        = 0.0f;
	m_fMaxYOverNumCells        = 0.0f;
	m_fMaxXOverNumCellsOverOne = 0.0f;
	m_fMaxYOverNumCellsOverOne = 0.0f;

	m_pAllNodes = new Node[ a_iNumNodes ];
	for ( TINT i = 0; i < a_iNumNodes; i++ )
		m_FreeList.InsertHead( &m_pAllNodes[ i ] );
}

// $Barnyard: FUNCTION 0061f920
// $Barnyard: FUNCTION 0061f880
AObjectHash::~AObjectHash()
{
	m_FreeList.RemoveAll();

	if ( m_pAllNodes )
		delete[] m_pAllNodes;
}
