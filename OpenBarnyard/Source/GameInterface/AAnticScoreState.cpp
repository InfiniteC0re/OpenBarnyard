#include "pch.h"
#include "AAnticScoreState.h"
#include "Helpers/AHighScoreManager.h"
#include "AAnticHighScoreState.h"
#include "Helpers/AGameLoader.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"
#include "Cameras/ACameraManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AAnticScoreState );

TPSTRING8_DECLARE( DialogInputMap );

// Texture sections of the grades
static const TCHAR* s_aGradeTextures[ AAnticScoreState::GRADE_NUMOF ] = {
	"Grade_Star",
	"Grade_A",
	"Grade_B",
	"Grade_C",
	"Grade_D",
	"Grade_F",
}; // 0x007735e0

// Locale strings of the grades
static const TINT s_aGradeTexts[ AAnticScoreState::GRADE_NUMOF ] = {
	241, 244, 245, 246, 247, 276
}; // 0x0071b8c8

static TFLOAT s_fStatsDelay        = 0.5f;    // 0x00773604
static TFLOAT s_fSmokeDuration     = 0.75f;   // 0x00773600
static TFLOAT s_fBrandAcceleration = 2000.0f; // 0x007735fc
static TFLOAT s_fStatPulseSpeed    = 10.0f;   // 0x007735f8

// Speed of the branding iron
static TFLOAT s_fBrandIronSpeed; // 0x00781d3c

//-----------------------------------------------------------------------------
// AAnticScoreState::ScoreStat
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00445ef0
AAnticScoreState::ScoreStat::ScoreStat()
{
}

// $Barnyard: FUNCTION 00445f70
// $Barnyard: FUNCTION 00446170
AAnticScoreState::ScoreStat::~ScoreStat()
{
	m_oName.RemoveSelf();
	m_oScore.RemoveSelf();
	m_oTicker.RemoveSelf();
	m_oBacking.RemoveSelf();
}

// $Barnyard: FUNCTION 00446430
TBOOL AAnticScoreState::ScoreStat::Create( const TWCHAR* a_wszName, TINT a_iScore )
{
	if ( a_wszName == TNULL )
		return TFALSE;

	m_eState = STATE_HIDDEN;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	// Name
	m_oName.Create( pFont, 0.0f );
	m_oName.SetTextAndFitWidth( a_wszName );
	m_oName.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oName.GetTransform().SetTranslation( 40.0f, 0.0f );
	m_oName.GetTransform().Scale( 2.0f, 2.0f );
	m_oName.Hide();
	m_oName.SetColour( 0xffffffff );

	// Score (the ticker shows it, this text box is never added to the GUI)
	TStringManager::String16Format( m_wszScore, TARRAYSIZE( m_wszScore ), L"%d", a_iScore );
	m_oScore.Create( pFont, 0.0f );
	m_oScore.SetTextAndFitWidth( m_wszScore );
	m_oScore.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oScore.SetColour( 0xffffc800 );

	m_oBacking.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "ScoreBacking" ) );
	AddChildTail( m_oBacking );

	TFLOAT fScreenWidth = AGUI2::GetRootElement()->GetWidth();

	// Ticker
	m_oTicker.Create( 4, TNULL, -1.0f, -1.0f );
	m_oTicker.Reset();
	m_oTicker.SetScore( a_iScore );
	m_oTicker.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oTicker.GetTransform().Scale( 1.1f, 1.1f );
	m_oTicker.GetTransform().SetTranslation( fScreenWidth * -0.05f, 2.0f );
	m_oBacking.AddChildTail( m_oTicker );

	m_oBacking.T2GUIElement::SetDimensions( fScreenWidth * 0.8f, m_oTicker.GetHeight() );
	SetDimensions( m_oBacking.GetWidth(), m_oBacking.GetHeight() );
	m_oBacking.AddChildTail( m_oName );

	// Make the name fit between the left side and the ticker
	TFLOAT fNameWidth = m_oBacking.GetWidth() - m_oTicker.GetWidth() - fScreenWidth * 0.15f;
	m_oName.SetWidth( fNameWidth );

	TFLOAT fScaledNameWidth = m_oName.GetWidth() * m_oName.GetTransform().m_aMatrixRows[ 0 ].x;

	if ( fScaledNameWidth > fNameWidth )
	{
		TFLOAT fScale = fNameWidth / fScaledNameWidth;
		m_oName.GetTransform().Scale( fScale, fScale );
	}

	m_fScale = m_oName.GetTransform().m_aMatrixRows[ 0 ].x;
	m_oName.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_fTime = 0.0f;

	SetDimensions( m_oBacking.GetWidth(), TMath::Max( m_oName.GetHeight(), m_oBacking.GetHeight() ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00446040
void AAnticScoreState::ScoreStat::Update( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_TICKING )
	{
		TVector2 vecTranslation = m_oName.GetTransform().GetTranslation();

		if ( TMath::PI / s_fStatPulseSpeed < m_fTime )
		{
			// Done pulsing
			m_oName.GetTransform().SetIdentity();
			m_oName.GetTransform().SetTranslation( vecTranslation );
			m_oName.GetTransform().Scale( m_fScale, m_fScale );

			if ( !m_oTicker.IsSpinning() )
			{
				m_eState = STATE_DONE;
				m_oName.SetColour( 0xffffc800 );
			}
		}
		else
		{
			// The name pulses while it appears
			TFLOAT fScale = TMath::Sin( s_fStatPulseSpeed * m_fTime ) * 0.2f + m_fScale;
			m_oName.Show();
			m_oName.GetTransform().SetIdentity();
			m_oName.GetTransform().SetTranslation( vecTranslation );
			m_oName.GetTransform().Scale( fScale, fScale );
		}

		m_fTime += a_fDeltaTime;
	}
	else if ( m_eState == STATE_DONE )
	{
		m_oName.SetColour( 0xffffc800 );
		m_oTicker.ShowFinalScore();
	}
}

// $Barnyard: FUNCTION 004466f0
void AAnticScoreState::ScoreStat::SetState( STATE a_eState )
{
	if ( a_eState == STATE_DONE )
	{
		TVector2 vecTranslation = m_oName.GetTransform().GetTranslation();
		m_oName.GetTransform().SetIdentity();
		m_oName.GetTransform().SetTranslation( vecTranslation );
		m_oName.GetTransform().Scale( m_fScale, m_fScale );
		m_eState = STATE_DONE;
	}
	else
	{
		if ( a_eState == STATE_TICKING )
		{
			m_fTime = 0.0f;
			m_oTicker.SetSpinning( TTRUE );
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_SCORE_TOTAL );
		}

		m_eState = a_eState;
	}
}

//-----------------------------------------------------------------------------
// AAnticScoreState
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00446780
AAnticScoreState::AAnticScoreState( AMiniGameState::S1* a_pResults )
    : m_eState( STATE_WAIT )
    , m_fTimer( 0.0f )
    , m_bQuitMiniGame( TFALSE )
    , m_bRemove( TFALSE )
    , m_bContinue( TFALSE )
    , m_pStats( TNULL )
    , m_iNumStats( 0 )
    , m_pResults( a_pResults )
    , m_vecBrandTarget( TVector2::VEC_ZERO )
    , m_iCurrentStat( 0 )
    , m_eGrade( GRADE_C )
    , m_bGradeFaded( TFALSE )
{
	APlayerManager*             pPlayerManager = APlayerManager::GetSingleton();
	TINT                        iPlayer        = pPlayerManager->GetCurrentPlayerIterator();
	AMiniGameState::PlayerInfo& rPlayerInfo    = m_pResults->oPlayerList[ iPlayer ];

	// The total is shown in an extra row
	TINT iHasTotal = ( m_pResults->bFlag4 == TTRUE ) ? 1 : 0;
	m_iNumStats    = rPlayerInfo.vecRows.Size() + iHasTotal;

	// NOTE: the original only allocates the rows if there's a total row
	m_pStats = ( iHasTotal != 0 ) ? new ScoreStat[ m_iNumStats ] : TNULL;

	for ( TINT i = 0; i < m_iNumStats - iHasTotal; i++ )
		m_pStats[ i ].Create( rPlayerInfo.vecRows[ i ].wszName, rPlayerInfo.vecRows[ i ].iScore );

	if ( iHasTotal > 0 )
	{
		m_pStats[ m_iNumStats - 1 ].Create( rPlayerInfo.wszTotalName, rPlayerInfo.iTotal );

		if ( m_pResults->bWon )
		{
			// The grade of the total in the chart of the minigame
			m_eGrade = GRADE( AHighScoreManager::GetLastRecordGrade( TFLOAT( rPlayerInfo.iTotal ) ) );
		}
		else
		{
			m_eGrade = GRADE_F;
		}
	}

	TINT iNumHumanPlayers = pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount();

	// Multiplayer: sum the totals of the rounds
	if ( iNumHumanPlayers > 1 )
		AMiniGameManager::GetSingleton()->GetSessionResults().oPlayerList[ iPlayer ].iUnk2 += rPlayerInfo.iTotal;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	const TWCHAR* wszResult;

	if ( m_pResults->bWon == TTRUE )
	{
		wszResult = g_pLocaleManager->GetString( 325 );
	}
	else if ( iNumHumanPlayers > 1 && AMiniGameManager::GetSingleton()->GetSessionResults().oPlayerList[ 0 ].iUnk2 == AMiniGameManager::GetSingleton()->GetSessionResults().oPlayerList[ 1 ].iUnk2 )
	{
		// Draw
		wszResult = g_pLocaleManager->GetString( 324 );
	}
	else
	{
		wszResult = g_pLocaleManager->GetString( 327 );
	}

	m_oResultText.Create( pFont, pFont->GetTextWidth( wszResult ) );
	m_oResultText.SetText( wszResult );

	// "Press OK to continue"
	ALocale::FormatInputTags( m_wszPressOK, 128, g_pLocaleManager->GetString( 167 ) );

	m_oPressOK.Create( pFont, pFont->GetTextWidth( m_wszPressOK ) );
	m_oPressOK.SetText( m_wszPressOK );
	m_oPressOK.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oPressOK.Hide();

	T2GUITransform oScale;
	oScale.m_aMatrixRows[ 0 ] = TVector2( 0.75f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 0.75f );
	oScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oPressOK.GetTransform().PreMultiply( oScale );

	AGUI2::GetRootElement()->AddChildTail( m_oPressOK );

	if ( m_pStats == TNULL )
	{
		// Nothing to show, wait for OK
		m_eState = STATE_BRAND_RETURN;
		m_oPressOK.Show();
		m_bGradeFaded = TFALSE;
	}
}

// $Barnyard: FUNCTION 004461d0
// $Barnyard: FUNCTION 00446c10
AAnticScoreState::~AAnticScoreState()
{
}

// $Barnyard: FUNCTION 00448030
TBOOL AAnticScoreState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eState == STATE_STATS )
	{
		if ( a_eInputCommand != AInputCommand_OK )
			return TFALSE;

		// Skip the current row
		if ( m_iCurrentStat < m_iNumStats )
		{
			m_pStats[ m_iCurrentStat ].SetState( ScoreStat::STATE_DONE );
			m_iCurrentStat++;

			if ( m_iCurrentStat < m_iNumStats )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::UI_SCORE_TOTAL );
				m_pStats[ m_iCurrentStat ].SetState( ScoreStat::STATE_TICKING );
			}
		}

		return TTRUE;
	}

	if ( m_eState == STATE_TOTAL )
	{
		if ( a_eInputCommand != AInputCommand_OK )
			return TFALSE;

		m_pStats[ m_iNumStats - 1 ].SetState( ScoreStat::STATE_DONE );
		return TTRUE;
	}

	if ( a_eInputCommand == AInputCommand_OK && m_oPressOK.IsVisible() )
	{
		m_bContinue = TTRUE;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00447620
TBOOL AAnticScoreState::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Smoke puffs grow upwards
	for ( TINT i = 0; i < NUM_SMOKE_PUFFS; i++ )
	{
		T2GUIRectangle& rSmoke = m_aSmoke[ i ];

		if ( rSmoke.IsVisible() )
		{
			T2GUITransform& rTransform     = rSmoke.GetTransform();
			TFLOAT          fScaleY        = rTransform.m_aMatrixRows[ 1 ].y;
			TFLOAT          fScaleX        = rTransform.m_aMatrixRows[ 0 ].x;
			TVector2        vecTranslation = rTransform.GetTranslation();
			TFLOAT          fAlpha         = rSmoke.GetAlpha();

			if ( fScaleY <= 2.0f )
				fAlpha = TMath::LERP( fAlpha, 0.75f, a_fDeltaTime * 5.0f );
			else
				fAlpha = TMath::LERP( fAlpha, 0.0f, a_fDeltaTime );

			rSmoke.SetAlpha( fAlpha );

			vecTranslation.y -= a_fDeltaTime * 3.0f;
			fScaleY = TMath::LERP( fScaleY, 5.0f, a_fDeltaTime * 0.5f );

			rTransform.SetIdentity();

			T2GUITransform oScale;
			oScale.m_aMatrixRows[ 0 ] = TVector2( fScaleX, 0.0f );
			oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, fScaleY );
			oScale.m_vecTranslation   = TVector2::VEC_ZERO;
			rTransform.PreMultiply( oScale );

			rTransform.SetTranslation( vecTranslation );
		}
	}

	switch ( m_eState )
	{
		case STATE_WAIT:
			m_fTimer += a_fDeltaTime;

			if ( m_fTimer >= s_fStatsDelay )
			{
				m_eState       = STATE_STATS;
				m_fTimer       = 0.0f;
				m_iCurrentStat = 0;
				m_pStats[ 0 ].SetState( ScoreStat::STATE_TICKING );
			}

			break;
		case STATE_STATS:
		{
			ScoreStat* pNextStat;

			if ( m_iCurrentStat < m_iNumStats - 1 )
			{
				// Wait for the current row
				if ( m_pStats[ m_iCurrentStat ].GetState() != ScoreStat::STATE_DONE )
					break;

				m_iCurrentStat++;

				if ( m_iCurrentStat < m_iNumStats - 1 )
				{
					pNextStat = &m_pStats[ m_iCurrentStat ];
				}
				else
				{
					m_eState  = STATE_TOTAL;
					pNextStat = &m_pStats[ m_iNumStats - 1 ];
				}
			}
			else
			{
				m_eState  = STATE_TOTAL;
				pNextStat = &m_pStats[ m_iNumStats - 1 ];
			}

			pNextStat->SetState( ScoreStat::STATE_TICKING );
			break;
		}
		case STATE_TOTAL:
			if ( m_pStats[ m_iNumStats - 1 ].GetState() == ScoreStat::STATE_DONE )
			{
				m_fTimer += a_fDeltaTime;
				m_oGradeText.Show();

				if ( m_eGrade <= GRADE_B )
					m_oGradeTextFlash.Show();

				m_eState = STATE_BRAND;
				m_oBrandIron.Show();
				m_fTimer          = 0.0f;
				s_fBrandIronSpeed = 2000.0f;
				m_vecBrandTarget  = TVector2( -120.0f, 0.0f );
			}

			break;
		case STATE_BRAND:
		{
			// The branding iron moves to the grade
			T2GUITransform& rTransform = m_oBrandIron.GetTransform();
			TVector2        vecPos     = rTransform.GetTranslation();
			TVector2        vecDir     = m_vecBrandTarget - vecPos;

			if ( vecDir.x * vecDir.x + vecDir.y * vecDir.y > 0.1f )
			{
				vecDir.Normalize();
				vecDir.x *= s_fBrandIronSpeed;
				vecDir.y *= s_fBrandIronSpeed;

				TFLOAT fX = vecDir.x * a_fDeltaTime + vecPos.x;
				TFLOAT fY = vecDir.y * a_fDeltaTime + vecPos.y;

				if ( fX > m_vecBrandTarget.x )
					fX = m_vecBrandTarget.x;

				if ( fY < m_vecBrandTarget.y )
					fY = m_vecBrandTarget.y;

				TFLOAT fOldScale = rTransform.m_aMatrixRows[ 0 ].x;
				rTransform.SetIdentity();
				rTransform.SetTranslation( fX, fY );

				TFLOAT fScaleX = TMath::LERP( fOldScale, 1.5f, a_fDeltaTime + a_fDeltaTime );
				TFLOAT fScaleY = TMath::LERP( fScaleX, 1.5f, a_fDeltaTime + a_fDeltaTime );
				rTransform.Scale( fScaleX, fScaleY );
			}
			else
			{
				// Stamp the grade
				m_eState = STATE_SMOKE;
				m_aSmoke[ 0 ].Show();
				m_aSmoke[ 0 ].SetAlpha( 0.2f );
				m_oGrade.Show();
				m_fTimer = 0.0f;

				ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::UI_BRAND, ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation() );
			}

			break;
		}
		case STATE_SMOKE:
			m_fTimer += a_fDeltaTime;

			if ( m_fTimer < s_fSmokeDuration )
			{
				if ( m_fTimer >= 0.5f )
				{
					m_aSmoke[ 2 ].Show();
					m_aSmoke[ 2 ].SetAlpha( 0.2f );
				}
				else if ( m_fTimer >= 0.25f )
				{
					m_aSmoke[ 1 ].Show();
					m_aSmoke[ 1 ].SetAlpha( 0.2f );
				}
			}
			else
			{
				m_eState          = STATE_BRAND_RETURN;
				m_fTimer          = 0.0f;
				m_vecBrandTarget  = TVector2( -400.0f, 80.0f );
				s_fBrandIronSpeed = 1000.0f;
				m_bGradeFaded     = TFALSE;
			}

			break;
		case STATE_BRAND_RETURN:
		{
			// The branding iron goes back
			s_fBrandIronSpeed += s_fBrandAcceleration * a_fDeltaTime;

			T2GUITransform& rTransform = m_oBrandIron.GetTransform();
			TVector2        vecPos     = rTransform.GetTranslation();
			TVector2        vecDir     = m_vecBrandTarget - vecPos;

			if ( vecDir.x * vecDir.x + vecDir.y * vecDir.y > 0.1f )
			{
				vecDir.Normalize();
				vecDir.x *= s_fBrandIronSpeed;
				vecDir.y *= s_fBrandIronSpeed;

				TFLOAT fX = vecDir.x * a_fDeltaTime + vecPos.x;
				TFLOAT fY = vecDir.y * a_fDeltaTime + vecPos.y;

				if ( fX < m_vecBrandTarget.x )
					fX = m_vecBrandTarget.x;

				if ( fY > m_vecBrandTarget.y )
					fY = m_vecBrandTarget.y;

				TFLOAT fOldScale = rTransform.m_aMatrixRows[ 0 ].x;
				rTransform.SetIdentity();
				rTransform.SetTranslation( fX, fY );

				TFLOAT fScaleX = TMath::LERP( fOldScale, 2.0f, a_fDeltaTime + a_fDeltaTime );
				TFLOAT fScaleY = TMath::LERP( fScaleX, 2.0f, a_fDeltaTime + a_fDeltaTime );
				rTransform.Scale( fScaleX, fScaleY );
			}

			m_fTimer += a_fDeltaTime;

			if ( m_fTimer >= 1.0f )
			{
				// The brand cools down
				TUINT32 uiColour = m_oGrade.GetColour();
				TFLOAT  fAlpha   = ( ( uiColour >> 24 ) & 0xff ) * ( 1.0f / 255.0f );
				TFLOAT  fRed     = ( ( uiColour >> 16 ) & 0xff ) * ( 1.0f / 255.0f );
				TFLOAT  fGreen   = ( ( uiColour >> 8 ) & 0xff ) * ( 1.0f / 255.0f );
				TFLOAT  fBlue    = ( uiColour & 0xff ) * ( 1.0f / 255.0f );

				if ( m_bGradeFaded )
				{
					fRed -= a_fDeltaTime * 0.01f;
					fAlpha -= a_fDeltaTime * 0.01f;
				}
				else
				{
					fGreen -= a_fDeltaTime * 0.1f;
					fBlue -= a_fDeltaTime * 0.1f;

					if ( fGreen <= 0.0f && fBlue <= 0.0f )
						m_bGradeFaded = TTRUE;
				}

				TMath::Clip( fRed, 0.0f, 1.0f );
				TMath::Clip( fGreen, 0.0f, 1.0f );
				TMath::Clip( fBlue, 0.0f, 1.0f );
				TMath::Clip( fAlpha, 0.7f, 1.0f );

				m_oGrade.SetColour(
				    ( TUINT32( fAlpha * 255.0f ) << 24 ) |
				    ( TUINT32( fRed * 255.0f ) << 16 ) |
				    ( TUINT32( fGreen * 255.0f ) << 8 ) |
				    TUINT32( fBlue * 255.0f )
				);
			}

			if ( m_fTimer >= 2.0f )
				m_oPressOK.Show();

			break;
		}
	}

	if ( m_oGradeTextFlash.IsVisible() )
	{
		// The flash grows and fades out
		TFLOAT fAlpha = TMath::LERP( m_oGradeTextFlash.GetAlpha(), 0.0f, a_fDeltaTime + a_fDeltaTime );
		m_oGradeTextFlash.SetAlpha( fAlpha );

		T2GUITransform& rTransform = m_oGradeTextFlash.GetTransform();
		TFLOAT          fScaleX    = TMath::LERP( rTransform.m_aMatrixRows[ 0 ].x, 2.0f, a_fDeltaTime + a_fDeltaTime );
		TFLOAT          fScaleY    = TMath::LERP( fScaleX, 2.0f, a_fDeltaTime + a_fDeltaTime );

		rTransform.SetIdentity();
		rTransform.SetTranslation( 0.0f, 0.0f );

		T2GUITransform oScale;
		oScale.m_aMatrixRows[ 0 ] = TVector2( fScaleX, 0.0f );
		oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, fScaleY );
		oScale.m_vecTranslation   = TVector2::VEC_ZERO;
		rTransform.PreMultiply( oScale );
	}

	for ( TINT i = 0; i < m_iNumStats; i++ )
		m_pStats[ i ].Update( a_fDeltaTime );

	if ( m_bQuitMiniGame == TTRUE )
	{
		AGameLoader::LeaveMiniGame( TTRUE );
		return TTRUE;
	}

	if ( m_bRemove != TTRUE )
	{
		if ( m_bContinue != TTRUE )
			return AGameState::OnUpdate( a_fDeltaTime );

		if ( APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount() == 1 )
		{
			// Single player, show the high scores
			m_pOwnerState->ReplaceChildState( this, new AAnticHighScoreState( m_pResults ) );
			return TTRUE;
		}
	}

	Remove();
	return TTRUE;
}

// $Barnyard: FUNCTION 00446c30
void AAnticScoreState::OnInsertion()
{
	TFLOAT fScreenWidth  = AGUI2::GetRootElement()->GetWidth();
	TFLOAT fScreenHeight = AGUI2::GetRootElement()->GetHeight();

	SetInputMap( TPS8( DialogInputMap ) );

	AMiniGame* pMiniGame = AMiniGameManager::GetSingleton()->GetNextMiniGame();

	m_oBoard.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Result_Wood" ) );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	// Name of the minigame
	m_oTitle.Create( pFont, pFont->GetTextWidth( g_pLocaleManager->GetString( pMiniGame->GetLocaleTitle() ) ) );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oTitle.SetTextAndFitWidth( g_pLocaleManager->GetString( pMiniGame->GetLocaleTitle() ) );
	m_oTitle.SetColour( 0xffffffff );

	m_oTitle.GetTransform().SetIdentity();
	m_oSubtitle.GetTransform().SetIdentity();
	m_oBoard.GetTransform().SetIdentity();

	const TWCHAR* wszSubtitle = g_pLocaleManager->GetString( 249 );
	m_oSubtitle.Create( pFont, pFont->GetTextWidth( wszSubtitle ) );
	m_oSubtitle.SetTextAndFitWidth( wszSubtitle );
	m_oSubtitle.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oSubtitle.SetColour( 0xfffff0af );
	m_oTitle.AddChildTail( m_oSubtitle );

	static TFLOAT s_fBoardWidth = AGUI2::GetRootElement()->GetWidth() * 0.8f; // 0x00781d34

	// The board
	m_oBoard.AddChildTail( m_oTitle );
	m_oBoard.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oBoard.SetWidth( s_fBoardWidth );
	m_oBoard.SetTransform( AGUI2::GetRootElement()->GetWidth() * -0.02f, AGUI2::GetRootElement()->GetHeight() * 0.02f );
	m_oTitle.SetTransform( m_oBoard.GetWidth() * 0.05f, 0.0f );
	m_oSubtitle.SetTransform( 25.0f, 0.0f );

	TFLOAT fTitleScale = m_oBoard.GetWidth() / ( m_oTitle.GetWidth() + m_oSubtitle.GetWidth() + 25.0f ) * 0.8f;
	TMath::Clip( fTitleScale, 0.0f, 1.0f );
	m_oTitle.GetTransform().Scale( fTitleScale, fTitleScale );
	AGUI2::GetRootElement()->AddChildTail( m_oBoard );

	// Icon of the minigame
	m_oIcon.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( pMiniGame->GetIconName() ) );
	AGUI2::GetRootElement()->AddChildTail( m_oIcon );
	m_oIcon.GetTransform().SetTranslation( -10.0f, 0.0f );
	m_oIcon.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPRIGHT );
	m_oBoard.AddChildTail( m_oIcon );

	// Grade
	T2GUITextureSection* pGradeTexture = T2GUITextureSectionManager::GetTextureSection( s_aGradeTextures[ m_eGrade ] );

	m_oGradeText.Create( pFont, 200.0f );
	m_oGradeText.SetText( g_pLocaleManager->GetString( s_aGradeTexts[ m_eGrade ] ) );
	m_oGradeText.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oGradeText.SetTransform( fScreenWidth * 0.05f, fScreenHeight * 0.3f );
	m_oGradeText.SetColour( 0xffffc800 );
	m_oGradeText.Hide();

	if ( m_eGrade <= GRADE_B )
	{
		m_oGradeTextFlash.Create( pFont, 200.0f );
		m_oGradeTextFlash.SetText( g_pLocaleManager->GetString( s_aGradeTexts[ m_eGrade ] ) );
		m_oGradeTextFlash.SetColour( 0xffffc800 );
		m_oGradeTextFlash.Hide();
		m_oGradeText.AddChildTail( m_oGradeTextFlash );
	}

	m_oGrade.SetTextureSection( *pGradeTexture );
	m_oGrade.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

	T2GUITransform oScale;
	oScale.m_aMatrixRows[ 0 ] = TVector2( 1.5f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 1.5f );
	oScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oGrade.GetTransform().PreMultiply( oScale );
	m_oGrade.GetTransform().SetTranslation( 0.0f, 20.0f );
	m_oGrade.SetColour( 0xffffffff );
	m_oGrade.Hide();
	m_oGradeText.AddChildTail( m_oGrade );
	AGUI2::GetRootElement()->AddChildTail( m_oGradeText );

	// Smoke of the brand
	T2GUITextureSection* pSmokeTexture = T2GUITextureSectionManager::GetTextureSection( "Smoke" );

	for ( TINT i = 0; i < NUM_SMOKE_PUFFS; i++ )
	{
		m_aSmoke[ i ].SetTextureSection( *pSmokeTexture );
		m_oGrade.AddChildTail( m_aSmoke[ i ] );
		m_aSmoke[ i ].SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );

		oScale.m_aMatrixRows[ 0 ] = TVector2( 1.0f, 0.0f );
		oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 0.8f );
		oScale.m_vecTranslation   = TVector2::VEC_ZERO;
		m_aSmoke[ i ].GetTransform().PreMultiply( oScale );
		m_aSmoke[ i ].Hide();
	}

	// Branding iron
	m_oBrandIron.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Brand_Iron" ) );
	m_oBrandIron.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

	oScale.m_aMatrixRows[ 0 ] = TVector2( 2.0f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 2.0f );
	oScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oBrandIron.GetTransform().PreMultiply( oScale );
	m_oBrandIron.GetTransform().SetTranslation( -400.0f, 80.0f );
	m_oBrandIron.Hide();
	m_oGradeText.AddChildTail( m_oBrandIron );

	// Rows
	TFLOAT fRowY        = fScreenHeight * 0.2f;
	TFLOAT fRowMinScale = 2.0f;

	oScale.m_aMatrixRows[ 0 ] = TVector2( 0.7f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 0.7f );
	oScale.m_vecTranslation   = TVector2::VEC_ZERO;

	if ( m_pResults->bFlag4 == TTRUE )
	{
		for ( TINT i = 0; i < m_iNumStats - 1; i++ )
		{
			ScoreStat& rStat = m_pStats[ i ];
			rStat.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
			rStat.GetTransform().SetTranslation( fScreenWidth * -0.02f, fRowY );
			rStat.GetTransform().PreMultiply( oScale );
			AGUI2::GetRootElement()->AddChildTail( rStat );

			fRowY += rStat.GetHeight() * 0.56f;
			fRowMinScale = TMath::Min( rStat.GetScale(), fRowMinScale );
		}

		// The total is at the bottom
		ScoreStat& rTotal = m_pStats[ m_iNumStats - 1 ];
		rTotal.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
		rTotal.SetTransform( fScreenWidth * -0.02f, fScreenHeight * -0.1f );
		rTotal.GetTransform().PreMultiply( oScale );
		AGUI2::GetRootElement()->AddChildTail( rTotal );
	}
	else
	{
		for ( TINT i = 0; i < m_iNumStats; i++ )
		{
			ScoreStat& rStat = m_pStats[ i ];
			rStat.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
			rStat.SetTransform( fScreenWidth * -0.02f, fRowY );
			rStat.GetTransform().PreMultiply( oScale );
			AGUI2::GetRootElement()->AddChildTail( rStat );

			fRowY += rStat.GetHeight() * 0.56f;
			fRowMinScale = TMath::Min( rStat.GetScale(), fRowMinScale );
		}
	}

	// All the names use the same scale
	for ( TINT i = 0; i < m_iNumStats; i++ )
		m_pStats[ i ].SetScale( fRowMinScale );

	if ( m_pResults->bFlag4 )
		m_pStats[ m_iNumStats - 1 ].SetScale( 2.0f );

	m_oResultText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oResultText.SetTransform( 0.0f, m_oResultText.GetHeight() + m_oResultText.GetHeight() );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 004462d0
void AAnticScoreState::OnRemoval()
{
	m_oTitle.RemoveSelf();
	m_oSubtitle.RemoveSelf();
	m_oBoard.RemoveSelf();
	m_oIcon.RemoveSelf();
	m_oGrade.RemoveSelf();
	m_oBrandIron.RemoveSelf();
	m_oGradeText.RemoveSelf();
	m_oGradeTextFlash.RemoveSelf();
	m_oPressOK.RemoveSelf();

	for ( TINT i = 0; i < NUM_SMOKE_PUFFS; i++ )
		m_aSmoke[ i ].RemoveSelf();

	for ( TINT i = 0; i < m_iNumStats; i++ )
		m_pStats[ i ].RemoveSelf();

	delete[] m_pStats;
	m_pStats = TNULL;

	m_oResultText.RemoveSelf();
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00446410
void AAnticScoreState::OnActivate()
{
	m_bQuitMiniGame = TFALSE;
	m_bRemove       = TFALSE;
	m_bContinue     = TFALSE;
	AGameState::OnActivate();
}
