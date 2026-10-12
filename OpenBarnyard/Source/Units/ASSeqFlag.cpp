#include "pch.h"
#include "ASSeqFlag.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0062b4c0, CreateObject 0062b4d0, CreateObjectInPlace 0062b540
TDEFINE_CLASS( ASSeqFlag );

// $Barnyard: FUNCTION 0062b630
ASSeqFlag::ASSeqFlag( TINT a_iNumFlags )
{
	m_iNumFlags = a_iNumFlags;
	m_puiFlags  = TSTATICCAST( TUINT32, TMalloc( ( a_iNumFlags / 32 + 1 ) * sizeof( TUINT32 ) ) );

	TINT iNumWords = m_iNumFlags / 32 + 1;

	for ( TINT i = 0; i < iNumWords; i++ )
		m_puiFlags[ i ] = 0;
}

// $Barnyard: FUNCTION 0062b610
// $Barnyard: FUNCTION 0062b590
ASSeqFlag::~ASSeqFlag()
{
	TFree( m_puiFlags );
}

// $Barnyard: FUNCTION 0062b5b0
void ASSeqFlag::Clear()
{
	TINT iNumWords = m_iNumFlags / 32 + 1;

	for ( TINT i = 0; i < iNumWords; i++ )
		m_puiFlags[ i ] = 0;
}
