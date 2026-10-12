#include "pch.h"
#include "AAnticHighScoreState.h"
#include "AAnticTrophyState.h"
#include "Helpers/AHighScoreManager.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"
#include "MiniGame/AMiniGameManager.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AAnticHighScoreState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00445ac0
AAnticHighScoreState::AAnticHighScoreState( AMiniGameState::S1* a_pResults )
    : m_bDone( TFALSE )
    , m_pResults( a_pResults )
{
}

// $Barnyard: FUNCTION 00445b20
// $Barnyard: FUNCTION 00445bc0
AAnticHighScoreState::~AAnticHighScoreState()
{
}

// $Barnyard: FUNCTION 00445b80
TBOOL AAnticHighScoreState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand != AInputCommand_OK || !m_oPressOK.IsVisible() )
		return TFALSE;

	m_bDone = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00445d40
TBOOL AAnticHighScoreState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_bDone )
		return AGameState::OnUpdate( a_fDeltaTime );

	// NOTE: retail removes this state if the main profile is TNULL (it can't be, it's a member of APlayerProfileManager) and then carries on
	m_pOwnerState->ReplaceChildState( this, new AAnticTrophyState( AAnticTrophyState::GetResult( m_pResults ) ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 00445be0
void AAnticHighScoreState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );
	AMiniGameManager::GetSingleton()->GetNextMiniGame();

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	// "Press OK to continue"
	ALocale::FormatInputTags( m_wszPressOK, 128, g_pLocaleManager->GetString( 167 ) );

	m_oPressOK.Create( pFont, pFont->GetTextWidth( m_wszPressOK ) );
	m_oPressOK.SetText( m_wszPressOK );
	m_oPressOK.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oPressOK.Show();

	T2GUITransform oScale;
	oScale.m_aMatrixRows[ 0 ] = TVector2( 0.75f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 0.75f );
	oScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oPressOK.GetTransform().PreMultiply( oScale );

	AGUI2::GetRootElement()->AddChildTail( m_oPressOK );

	// Shows the high score chart of the minigame
	TINT iRank = -1;
	AHighScoreManager::GetSingleton()->SetChartEnabled( TFALSE );
	m_bDone = !AHighScoreManager::GetSingleton()->ShowLastRecord( iRank, TFALSE );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00445d10
void AAnticHighScoreState::OnRemoval()
{
	m_oPressOK.RemoveSelf();

	AHighScoreManager::GetSingleton()->HideChart();

	AGameState::OnRemoval();
}
