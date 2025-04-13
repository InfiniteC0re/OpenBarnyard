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
	TIMPLEMENT_D( "Initialise some values" );

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
