#include "pch.h"
#include "AMiniGame.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00469a00
AMiniGame::AMiniGame()
    : m_iNumVariants( 0 )
    , m_iNumStateNames( 0 )
    , m_eLoadFlags( LOADFLAGS_NONE )
    , field6_0x18( 1 )
{
}

// NOTE: compiler generated in retail
// $Barnyard: FUNCTION 005057a0
AMiniGame::AMiniGame( const AMiniGame& a_rcOther )
    : m_strMiniGameName( a_rcOther.m_strMiniGameName )
    , m_eLocaleTitle( a_rcOther.m_eLocaleTitle )
    , m_eLocaleDescription( a_rcOther.m_eLocaleDescription )
    , m_pGameStateClass( a_rcOther.m_pGameStateClass )
    , m_iMinNumPlayers( a_rcOther.m_iMinNumPlayers )
    , m_iMaxNumPlayers( a_rcOther.m_iMaxNumPlayers )
    , field6_0x18( a_rcOther.field6_0x18 )
    , field7_0x1c( a_rcOther.field7_0x1c )
    , m_szIconName( a_rcOther.m_szIconName )
    , m_pLoadingScreenName( a_rcOther.m_pLoadingScreenName )
    , m_eInstancesType( a_rcOther.m_eInstancesType )
    , m_iDefaultVariant( a_rcOther.m_iDefaultVariant )
    , m_iNumSelectableVariants( a_rcOther.m_iNumSelectableVariants )
    , m_iNumVariants( a_rcOther.m_iNumVariants )
    , m_iNumStateNames( a_rcOther.m_iNumStateNames )
    , m_eStartHudKind( a_rcOther.m_eStartHudKind )
    , m_eLoadFlags( a_rcOther.m_eLoadFlags )
#ifdef BARNYARD_LATEST
    , m_strXUILoadScreen( a_rcOther.m_strXUILoadScreen )
#endif
{
	TUtil::MemCopy( m_aVariants, a_rcOther.m_aVariants, sizeof( m_aVariants ) );
	TUtil::MemCopy( m_aszAlternativeStateNames, a_rcOther.m_aszAlternativeStateNames, sizeof( m_aszAlternativeStateNames ) );
}

AMiniGame::~AMiniGame()
{
}

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 801870d8
void AMiniGame::SetXUILoadScreen( const TPString8& a_rcName )
{
	m_strXUILoadScreen = a_rcName;
}

#endif // BARNYARD_LATEST

const AMiniGame::Variant& AMiniGame::GetVariant( TINT a_iIndex )
{
	TASSERT( a_iIndex < m_iNumVariants );
	return m_aVariants[ a_iIndex ];
}

AMiniGame& AMiniGame::AddVariant( TINT a_eLocaleTitle, TINT a_eLocaleHint, TINT a_eLocaleDescription, const TCHAR* a_szAlternativeStateName )
{
	TASSERT( m_iNumVariants < MAX_NUM_VARIANTS );

	Variant& rVariant           = m_aVariants[ m_iNumVariants++ ];
	rVariant.eLocaleTitle       = a_eLocaleTitle;
	rVariant.eLocaleHint        = a_eLocaleHint;
	rVariant.eLocaleDescription = a_eLocaleDescription;

	m_aszAlternativeStateNames[ m_iNumStateNames++ ] = a_szAlternativeStateName;
	m_iNumSelectableVariants++;

	return *this;
}
