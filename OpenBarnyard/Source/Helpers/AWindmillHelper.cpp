#include "pch.h"
#include "AWindmillHelper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054d320
AWindmillHelper::AWindmillHelper()
    : m_iNumBarnSigns( 0 )
{
}

// $Barnyard: FUNCTION 0054c7a0
// $Barnyard: FUNCTION 0054d3e0
AWindmillHelper::~AWindmillHelper()
{
}

TBOOL AWindmillHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	TASSERT( !"Not implemented" );
	return TTRUE;
}

void AWindmillHelper::OnDestroy()
{
	TASSERT( !"Not implemented" );
}

// $Barnyard: FUNCTION 0054d7c0
void AWindmillHelper::CreateBarnSign()
{
	TASSERT( m_iNumBarnSigns < TARRAYSIZE( m_aBarnSigns ) );
	m_aBarnSigns[ m_iNumBarnSigns++ ].SpawnAtLocator( "loc_barnsign" );
}
