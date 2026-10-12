#include "pch.h"
#include "ABarnYardPoolHud.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TINT LOCALE_DUKE        = 132; // name of the default portrait
static constexpr TINT LOCALE_PLAYER_TURN = 638; // turn of the first team's player (the name is a tag)
static constexpr TINT LOCALE_OTHER_TURN  = 639; // turn of the other team

static constexpr TFLOAT MESSAGE_FADE_DELAY = 3.0f; // 00773b5c
static constexpr TFLOAT TEXT_FADE_TIME     = 2.0f;

//-----------------------------------------------------------------------------
// ABarnYardPoolHud::TeamHud
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004d0d00
ABarnYardPoolHud::TeamHud::TeamHud()
{
	m_pYellowBall = TNULL;
	m_pRedBall    = TNULL;
	m_bBallsShown = TFALSE;
}

// $Barnyard: FUNCTION 004d0da0
// $Barnyard: FUNCTION 004d1750
ABarnYardPoolHud::TeamHud::~TeamHud()
{
	for ( TINT i = 0; i < NUM_BALLS; i++ )
		m_aBalls[ i ].RemoveSelf();

	m_oPortrait.RemoveSelf();
	m_oPlank.RemoveSelf();
	m_oName.RemoveSelf();
	RemoveSelf();
}

// $Barnyard: FUNCTION 004d1220
void ABarnYardPoolHud::TeamHud::SetAlpha( TFLOAT a_fAlpha )
{
	m_oPortrait.SetAlpha( a_fAlpha );
	m_oName.SetAlpha( a_fAlpha );
	m_oPlank.SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 004d1020
void ABarnYardPoolHud::TeamHud::SetPanelVisible( TBOOL a_bVisible )
{
	if ( m_bBallsShown == TTRUE )
	{
		for ( TINT i = 0; i < NUM_BALLS; i++ )
			m_aBalls[ i ].SetVisible( a_bVisible == TTRUE );
	}

	m_oPortrait.SetVisible( a_bVisible == TTRUE );
	m_oPlank.SetVisible( a_bVisible == TTRUE );
	m_oName.SetVisible( a_bVisible == TTRUE );
	SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 004d1770
TBOOL ABarnYardPoolHud::TeamHud::Create( TBOOL a_bLeft, T2GUIElement* a_pParent )
{
	if ( !a_pParent )
		a_pParent = AGUI2::GetRootElement();

	m_pRedBall    = T2GUITextureSectionManager::GetTextureSection( "ball_red2" );
	m_pYellowBall = T2GUITextureSectionManager::GetTextureSection( "ball_yellow2" );
	T2GUITextureSectionManager::GetTextureSection( "bezel" ); // NOTE: unused in the original too

	SetPlayer( "Duke", ALocaleManager::Interface()->GetString( LOCALE_DUKE ), a_bLeft );

	// Make the portrait a bit bigger
	T2GUITransform oScale;
	oScale.m_aMatrixRows[ 0 ] = TVector2( 1.2f, 0.0f );
	oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 1.2f );
	oScale.m_vecTranslation   = TVector2( 0.0f, 0.0f );
	m_oPortrait.GetTransform().PreMultiply( oScale );

	// The plank with the name is next to the portrait
	T2GUITextureSection* pPlankSection = T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" );
	m_oPlank.SetDimensions( pPlankSection->GetWidth(), pPlankSection->GetHeight() );
	m_oPlank.SetTextureSection( pPlankSection );
	m_oPlank.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	if ( a_bLeft == TTRUE )
	{
		m_oPortrait.SetTranslation( 0.0f, 0.0f );
		m_oPortrait.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		m_oPlank.SetTransform( m_oPortrait.GetWidth(), 10.0f, 0.0f );
	}
	else
	{
		m_oPortrait.SetTranslation( 0.0f, 0.0f );
		m_oPortrait.FlipU();
		m_oPlank.SetTransform( -m_oPortrait.GetWidth(), 10.0f, 0.0f );
		m_oPortrait.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	}

	AddChildTail( m_oPlank );
	AddChildTail( m_oPortrait );

	// Two rows of 4 balls, the black one is the last one
	T2GUITextureSection* pBallSection = ( a_bLeft == TTRUE ) ? m_pRedBall : m_pYellowBall;
	TFLOAT               fBallWidth   = pBallSection->GetWidth();
	TFLOAT               fBallHeight  = pBallSection->GetHeight();
	TFLOAT               fX           = 0.0f;
	TFLOAT               fY           = -2.0f;

	for ( TINT i = 0; i < NUM_TEAM_BALLS; i++ )
	{
		m_aBalls[ i ].SetDimensions( fBallWidth, fBallHeight );
		m_aBalls[ i ].SetTextureSection( pBallSection );

		if ( a_bLeft == TTRUE )
		{
			m_aBalls[ i ].SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
			m_aBalls[ i ].SetTransform( fX, fY, 0.0f );
			fX += fBallWidth;
		}
		else
		{
			m_aBalls[ i ].SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
			m_aBalls[ i ].SetTransform( fX, fY, 0.0f );
			fX -= fBallWidth;
		}

		m_aBalls[ i ].Hide();
		a_pParent->AddChildHead( m_aBalls[ i ] );

		if ( i == 3 )
		{
			fX = 0.0f;
			fY -= fBallHeight;
		}
	}

	SetHeight( m_oPlank.GetHeight() * 1.1f );

	T2GUITextureSection* pBlackSection = T2GUITextureSectionManager::GetTextureSection( "ball_blk2" );
	m_aBalls[ BLACK_BALL ].SetDimensions( pBlackSection->GetWidth(), pBlackSection->GetHeight() );
	m_aBalls[ BLACK_BALL ].SetTextureSection( pBlackSection );

	T2GUIATTACHMENT eBlackAttachment = ( a_bLeft == TTRUE ) ? T2GUIATTACHMENT_BOTTOMLEFT : T2GUIATTACHMENT_BOTTOMRIGHT;
	m_aBalls[ BLACK_BALL ].SetAttachment( eBlackAttachment, eBlackAttachment );
	m_aBalls[ BLACK_BALL ].SetTransform( fX, fY, 0.0f );
	m_aBalls[ BLACK_BALL ].Hide();
	a_pParent->AddChildHead( m_aBalls[ BLACK_BALL ] );

	m_oPlank.AddChildTail( m_oName );
	m_oName.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	T2GUIATTACHMENT eAttachment = ( a_bLeft == TTRUE ) ? T2GUIATTACHMENT_TOPLEFT : T2GUIATTACHMENT_TOPRIGHT;
	SetAttachment( eAttachment, eAttachment );
	SetWidth( 200.0f );

	a_pParent->AddChildTail( *this );
	return TTRUE;
}

// $Barnyard: FUNCTION 004d0ea0
void ABarnYardPoolHud::TeamHud::SetPlayer( const TCHAR* a_szTexture, const TWCHAR* a_wszName, TBOOL a_bLeft )
{
	T2GUITextureSection* pSection = T2GUITextureSectionManager::GetTextureSection( a_szTexture );
	m_oPortrait.SetDimensions( pSection->GetWidth(), pSection->GetHeight() );
	m_oPortrait.SetTextureSection( pSection );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Don't use the name as the format string
	TStringManager::String16Format( m_wszName, NAME_SIZE, L"%s", a_wszName );
#else
	TStringManager::String16Format( m_wszName, NAME_SIZE, a_wszName );
#endif

	// NOTE: the original computes half the portrait width + 1 here and doesn't use it
	TFLOAT fPortraitWidth  = 0.0f;
	TFLOAT fPortraitHeight = 0.0f;
	m_oPortrait.GetDimensions( fPortraitWidth, fPortraitHeight );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oName.GetTransform().SetIdentity();
	m_oName.Create( pFont, pFont->GetTextWidth( m_wszName, 1.0f ) );
	m_oName.SetTextAndFitWidth( m_wszName );

	if ( a_bLeft == TTRUE )
	{
		m_oName.SetTransform( 0.0f, -3.0f, 0.0f );
	}
	else
	{
		m_oName.SetTransform( 0.0f, -3.0f, 0.0f );
		m_oPortrait.FlipU();
	}

	TFLOAT fHeight    = m_oPortrait.GetHeight();
	TFLOAT fHalfWidth = m_oPortrait.GetWidth() * 0.5f;
	SetDimensions( m_oName.GetWidth() + fHalfWidth, fHeight );
}

// $Barnyard: FUNCTION 004d1190
void ABarnYardPoolHud::TeamHud::SetBallColour( TINT a_eColour )
{
	if ( m_bBallsShown == TFALSE )
	{
		for ( TINT i = 0; i < NUM_TEAM_BALLS; i++ )
		{
			m_aBalls[ i ].SetTextureSection( ( a_eColour == BALLCOLOUR_YELLOW ) ? m_pYellowBall : m_pRedBall );
			m_aBalls[ i ].Show();
		}

		m_aBalls[ BLACK_BALL ].Show();
		m_bBallsShown = TTRUE;
	}
}

// $Barnyard: FUNCTION 004d1150
void ABarnYardPoolHud::TeamHud::SetNumLitBalls( TINT a_iNumBalls )
{
	for ( TINT i = 0; i < NUM_BALLS; i++ )
		m_aBalls[ i ].SetAlpha( ( i < a_iNumBalls ) ? 1.0f : 0.2f );
}

// $Barnyard: FUNCTION 004d11f0
void ABarnYardPoolHud::TeamHud::HideBalls()
{
	m_bBallsShown = TFALSE;

	for ( TINT i = 0; i < NUM_BALLS; i++ )
	{
		m_aBalls[ i ].SetAlpha( 0.5f );
		m_aBalls[ i ].Hide();
	}
}

//-----------------------------------------------------------------------------
// ABarnYardPoolHud
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 004d1260
ABarnYardPoolHud::ABarnYardPoolHud()
{
	m_bMessageFading   = TFALSE;
	m_apTeamNames[ 0 ] = TNULL;
	m_apTeamNames[ 1 ] = TNULL;
}

// $Barnyard: FUNCTION 004d1300
ABarnYardPoolHud::~ABarnYardPoolHud()
{
	// NOTE: the original doesn't detach m_oTitleText (its dtor unlinks it anyway)
	m_oMessage.RemoveSelf();
	m_oTurnText.RemoveSelf();
	m_oInfoText.RemoveSelf();
}

// $Barnyard: FUNCTION 004d1b00
TBOOL ABarnYardPoolHud::Create( T2GUIElement* a_pUnused, T2GUIElement* a_pParent )
{
	if ( !a_pParent )
		a_pParent = AGUI2::GetRootElement();

	if ( !m_aTeams[ 0 ].Create( TTRUE, a_pParent ) || !m_aTeams[ 1 ].Create( TFALSE, a_pParent ) )
		return TFALSE;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );
	m_oMessage.Create( pFont, a_pParent->GetWidth() * 0.65f );
	m_oMessage.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oMessage.SetTranslation( 0.0f, -5.0f );
	a_pParent->AddChildTail( m_oMessage );

	// Three lines of text at the top of the screen
	m_oTitleText.Create( TNULL, a_pParent, T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oInfoText.Create( T2GUIFontManager::FindFont( "Rekord18" ), &m_oTitleText, T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTurnText.Create( T2GUIFontManager::FindFont( "Rekord18" ), &m_oInfoText, T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );

	m_oTitleText.m_fFadeTime = TEXT_FADE_TIME;
	m_oTurnText.m_fFadeTime  = TEXT_FADE_TIME;
	m_oInfoText.m_fFadeTime  = TEXT_FADE_TIME;

	return TTRUE;
}

// $Barnyard: FUNCTION 004d1590
void ABarnYardPoolHud::Reset()
{
	m_aTeams[ 0 ].HideBalls();
	m_aTeams[ 1 ].HideBalls();

	m_oMessage.SetText( L"" );

	m_oTitleText.SetText( L"", 0x5500ff00 );
	m_oTurnText.SetText( L"", 0x5500ff00 );
	m_oInfoText.SetText( L"", 0x5500ff00 );
}

// $Barnyard: FUNCTION 004d1460
void ABarnYardPoolHud::Update( TFLOAT a_fDeltaTime )
{
	m_oTitleText.Update( a_fDeltaTime );
	m_oTurnText.Update( a_fDeltaTime );
	m_oInfoText.Update( a_fDeltaTime );

	m_fMessageTime += a_fDeltaTime;

	// NOTE: retail never sets m_bMessageFading, so the message never fades out
	if ( m_fMessageTime >= MESSAGE_FADE_DELAY && m_bMessageFading == TTRUE && m_oMessage.IsVisible() )
	{
		TFLOAT fAlpha = m_oMessage.GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );

		if ( fAlpha < 0.0f )
		{
			m_bMessageFading = TFALSE;
			fAlpha           = 0.0f;
			m_oMessage.Hide();
		}

		m_oMessage.SetAlpha( fAlpha );
	}
}

// $Barnyard: FUNCTION 004d13c0
void ABarnYardPoolHud::SetTeamsVisible( TBOOL a_bVisible )
{
	m_aTeams[ 0 ].SetPanelVisible( a_bVisible );
	m_aTeams[ 1 ].SetPanelVisible( a_bVisible );
}

// $Barnyard: FUNCTION 004d16c0
void ABarnYardPoolHud::SetFirstTeamActive( TBOOL a_bFirstTeam )
{
	if ( a_bFirstTeam )
	{
		m_aTeams[ 0 ].SetAlpha( 1.0f );
		m_aTeams[ 1 ].SetAlpha( 0.0f );
	}
	else
	{
		m_aTeams[ 0 ].SetAlpha( 0.0f );
		m_aTeams[ 1 ].SetAlpha( 1.0f );
	}
}

// $Barnyard: FUNCTION 004d1600
void ABarnYardPoolHud::SetTeamPlayer( const TCHAR* a_szTexture, const TWCHAR* a_wszName, TINT a_iTeam )
{
	if ( a_iTeam != 0 )
	{
		m_aTeams[ a_iTeam ].SetPlayer( a_szTexture, a_wszName, TFALSE );
		m_apTeamNames[ 1 ] = a_wszName;
	}
	else
	{
		m_aTeams[ 0 ].SetPlayer( a_szTexture, a_wszName, TTRUE );
		m_apTeamNames[ 0 ] = a_wszName;

		ALocale::FormatTags( m_wszTurnText, TARRAYSIZE( m_wszTurnText ), LOCALE_PLAYER_TURN, PPropertyValue( const_cast<TWCHAR*>( a_wszName ), TFALSE ) );
	}
}

// $Barnyard: FUNCTION 004d1530
void ABarnYardPoolHud::SetBallColour( TINT a_iTeam, TINT a_eColour )
{
	m_aTeams[ a_iTeam ].SetBallColour( a_eColour );
}

// $Barnyard: FUNCTION 004d13f0
void ABarnYardPoolHud::SetNumLitBalls( TINT a_iTeam, TINT a_iNumBalls )
{
	m_aTeams[ a_iTeam ].SetNumLitBalls( a_iNumBalls );
}

// $Barnyard: FUNCTION 004d1410
void ABarnYardPoolHud::SetMessage( const TWCHAR* a_wszText )
{
	if ( a_wszText )
	{
		m_oMessage.SetText( a_wszText );
		m_oMessage.SetAlpha( 1.0f );
		m_oMessage.Show();
		m_bMessageFading = TFALSE;
		m_fMessageTime   = 0.0f;
	}
	else
	{
		m_oMessage.Hide();
	}
}

// $Barnyard: FUNCTION 004d1550
void ABarnYardPoolHud::ShowTurn( TBOOL a_bOther )
{
	if ( a_bOther )
		m_oTurnText.SetLocaleText( LOCALE_OTHER_TURN, -1 );
	else
		m_oTurnText.SetText( m_wszTurnText, 0xffffffff );
}

// $Barnyard: FUNCTION 004d1710
void ABarnYardPoolHud::SetTitleText( const TWCHAR* a_wszText )
{
	m_oTitleText.SetText( a_wszText, 0xffffffff );
}

// $Barnyard: FUNCTION 004d1730
void ABarnYardPoolHud::SetInfoText( const TWCHAR* a_wszText )
{
	m_oInfoText.SetText( a_wszText, 0xffffffff );
}
