#include "pch.h"
#include "AAnticTrophyState.h"
#include "MiniGame/APrecisionSquirt.h"
#include "AGameStateController.h"
#include "GUI/AGUI2.h"
#include "GUI/AFadeManager.h"
#include "Helpers/AGameLoader.h"
#include "Locale/ALocale.h"
#include "MiniGame/AMiniGameManager.h"
#include "MiniGame/ACarChaseMiniGameState.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AAnticTrophyState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00448300
AAnticTrophyState::AAnticTrophyState( RESULT a_eResult )
    : m_eResult( a_eResult )
    , m_bLeave( TFALSE )
    , m_bTryAgain( TFALSE )
    , m_bFadingSound( TFALSE )
    , m_fPulseTime( 0.0f )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail leaves these uninitialised until used
	m_wszBucks[ 0 ] = L'\0';
	m_fVolume       = 1.0f;
#endif
}

// $Barnyard: FUNCTION 00448420
// $Barnyard: FUNCTION 00448160
AAnticTrophyState::~AAnticTrophyState()
{
	m_oFadeOverListener.Disconnect();
}

AAnticTrophyState::RESULT AAnticTrophyState::GetResult( const AMiniGameState::S1* a_pResults )
{
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	// The last story minigame (act 3, tier 16) is "Game Over" when lost
	if ( a_pResults->bFlag6 == TTRUE || ( pProfile->GetSavedAct() == 3 && pProfile->GetSavedTier() == 16 && !a_pResults->bWon ) )
		return RESULT_LOST_STORY;

	if ( a_pResults->bWon )
		return ( pProfile->GetMiniGamePickups() > 0 ) ? RESULT_WON_BUCKS : RESULT_WON;

	return RESULT_LOST;
}

// $Barnyard: FUNCTION 00448560
TBOOL AAnticTrophyState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	// No input while fading out
	if ( m_oFadeOverListener.IsLinked() )
		return TFALSE;

	// NOTE: AGUI2Menu::ProcessInputEvent is inlined here in the original
	return m_oMenu.ProcessInputEvent( a_pInputEvent );
}

// $Barnyard: FUNCTION 00448580
TBOOL AAnticTrophyState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_oFadeOverListener.IsLinked() )
		return TFALSE;

	return m_oMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent );
}

// $Barnyard: FUNCTION 00448440
TBOOL AAnticTrophyState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bFadingSound )
	{
		// Fade the sound out together with the screen
		m_fVolume -= a_fDeltaTime + a_fDeltaTime;

		if ( m_fVolume >= 0.0f )
		{
			ASoundManager::GetSingleton()->SetGlobalVolume( m_fVolume );
		}
		else
		{
			m_fVolume      = 0.0f;
			m_bFadingSound = TFALSE;
		}
	}

	// Pulse the focused button
	TFLOAT fScale = TMath::Sin( TMath::TWO_PI * m_fPulseTime ) * 0.05f + 1.0f;
	m_oMenu.GetFocusedMenuItem()->GetTransform().SetScale( fScale, fScale );
	m_fPulseTime += a_fDeltaTime;

	if ( m_oMenu.IsVisible() && m_oMenu.IsFocused() )
	{
		m_oMenu.Update( a_fDeltaTime );
		return TTRUE;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00448780
void AAnticTrophyState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );

	// The picture and the title
	TINT                 iTitle   = -1;
	T2GUITextureSection* pPicture = TNULL;

	switch ( m_eResult )
	{
		case RESULT_WON:
		case RESULT_WON_BUCKS:
			iTitle   = 326; // "Congratulations!"
			pPicture = T2GUITextureSectionManager::GetTextureSection( "Winner_Cow" );
			break;
		case RESULT_LOST:
			iTitle   = 276; // "Bad Luck!"
			pPicture = T2GUITextureSectionManager::GetTextureSection( "Looser_Cow" );
			break;
		case RESULT_LOST_STORY:
			iTitle   = 587; // "Game Over"
			pPicture = T2GUITextureSectionManager::GetTextureSection( "Looser_Cow" );
			break;
	}

	m_oPicture.SetTextureSection( *pPicture );
	AGUI2::GetRootElement()->AddChildTail( m_oPicture );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	m_oTitle.Create( pFont, 0.0f );
	m_oTitle.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( iTitle ) );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTransform( 0.0f, AGUI2::GetRootElement()->GetHeight() * 0.1f );

	T2GUITransform oTitleScale;
	oTitleScale.m_aMatrixRows[ 0 ] = TVector2( 1.25f, 0.0f );
	oTitleScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 1.25f );
	oTitleScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oTitle.GetTransform().PreMultiply( oTitleScale );

	AGUI2::GetRootElement()->AddChildTail( m_oTitle );

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	// The bucks are only kept when they're shown
	if ( m_eResult != RESULT_WON_BUCKS )
		pProfile->SetMiniGamePickups( 0 );

	if ( m_eResult == RESULT_WON_BUCKS )
	{
		// "x 5" with the buck icon on its left
		m_oBucks.Create( pFont, 0.0f );
		// NOTE: retail passes 0x80 as the size of the 33 characters buffer
		TStringManager::String16Format( m_wszBucks, TARRAYSIZE( m_wszBucks ), L"x %d", pProfile->GetMiniGamePickups() );
		m_oBucks.SetTextAndFitWidth( m_wszBucks );
		m_oBucks.SetColour( 0xffffffff );
		m_oBucks.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_BOTTOMLEFT );
		m_oBucks.GetTransform().SetTranslation( 0.0f, -10.0f );

		m_oBuckIcon.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "BarnYard_Buck" ) );
		m_oBuckIcon.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
		m_oBucks.AddChildTail( m_oBuckIcon );

		// "Coins Collected"
		m_oCoinsCollected.Create( pFont, 0.0f );
		m_oCoinsCollected.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 274 ) );
		m_oCoinsCollected.SetColour( 0xffffc800 );
		m_oCoinsCollected.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_TOPCENTER );
		m_oCoinsCollected.GetTransform().SetTranslation( 0.0f, 15.0f );

		AGUI2::GetRootElement()->AddChildTail( m_oBucks );
		AGUI2::GetRootElement()->AddChildTail( m_oCoinsCollected );
	}

	// The buttons: no "Try again" after winning a story minigame, and the
	// other button goes back to the barnyard in story mode
	T2GUIFont* pButtonFont = T2GUIFontManager::FindFont( "Rekord26" );

	TBOOL bStoryMode = AGameLoader::IsStoryMode();
	TBOOL bTryAgain  = !bStoryMode || m_eResult == RESULT_LOST || m_eResult == RESULT_LOST_STORY;
	TINT  iLeaveText = ( bStoryMode && m_eResult != RESULT_LOST_STORY ) ? 337 /* "Return to Barnyard" */ : 275 /* "Return to Main Menu" */;

	auto fnSetupButton = [ this, pButtonFont ]( BUTTON a_eButton, const TWCHAR* a_wszText ) {
		AGUI2Button& rButton = m_aButtons[ a_eButton ];
		rButton.Create( pButtonFont, pButtonFont->GetTextWidth( a_wszText ), a_wszText, 0.0f );
		rButton.SetFocusedColour( 0xffffffff );
		rButton.SetEnabledColour( 0xffb56f22 );
		rButton.SetDisabledColour( 0xff4b4b4b );
		rButton.SetFocusedScale( 1.0f );
		rButton.OnFocusLost();
		rButton.SetId( a_eButton );

		AGUI2TextBox& rTextBox = rButton.GetTextBox();
		rTextBox.SetShadow( TTRUE, 0xa0000000 );
		rTextBox.SetShadowAlpha( 0.6f );
		rTextBox.SetShadowOffset( 5.0f, 5.0f );

		m_oMenu.AddMenuItem( rButton );
	};

	if ( bTryAgain )
		fnSetupButton( BUTTON_TRY_AGAIN, ALocaleManager::Interface()->GetString( 338 ) ); // "Try again"

	fnSetupButton( BUTTON_LEAVE, ALocaleManager::Interface()->GetString( iLeaveText ) );

	m_oMenu.SetBackSound( ASOUNDID_INVALID );
	m_oMenu.SetNegativeSound( ASOUNDID_INVALID );
	m_oMenu.ReflowChildrenVertically();
	m_oMenu.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oMenu.SetTransform( 0.0f, -30.0f );

	T2GUITransform oMenuScale;
	oMenuScale.m_aMatrixRows[ 0 ] = TVector2( 0.75f, 0.0f );
	oMenuScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, 0.75f );
	oMenuScale.m_vecTranslation   = TVector2::VEC_ZERO;
	m_oMenu.GetTransform().PreMultiply( oMenuScale );

	m_oMenu.SetFocus( TTRUE );
	m_oMenu.SetCallbackUserData( this );
	m_oMenu.SetItemActivationCallback( OnMenuItemActivated );
	AGUI2::GetRootElement()->AddChildTail( m_oMenu );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00448260
void AAnticTrophyState::OnRemoval()
{
	m_oTitle.RemoveSelf();
	m_oBucks.RemoveSelf();
	m_oBuckIcon.RemoveSelf();
	m_oCoinsCollected.RemoveSelf();
	m_aButtons[ BUTTON_TRY_AGAIN ].RemoveSelf();
	m_aButtons[ BUTTON_LEAVE ].RemoveSelf();
	m_oMenu.RemoveSelf();
	m_oPicture.RemoveSelf();

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00448760
void AAnticTrophyState::OnMenuItemActivated( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	TSTATICCAST( AAnticTrophyState, a_pUserData )->OnButtonActivated( a_pItem );
}

// $Barnyard: FUNCTION 00448680
void AAnticTrophyState::OnButtonActivated( AGUI2MenuItem* a_pItem )
{
	TINT iButton = TSTATICCAST( AGUI2Button, a_pItem )->GetId();

	if ( iButton == BUTTON_TRY_AGAIN )
	{
		if ( m_bTryAgain )
			return;

		m_bTryAgain = TTRUE;
		m_bLeave    = TFALSE;
	}
	else if ( iButton == BUTTON_LEAVE && !m_bLeave )
	{
		m_bLeave    = TTRUE;
		m_bTryAgain = TFALSE;
	}
	else
	{
		return;
	}

	// Fade the screen and the sound out
	AFade* pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 0.5f );

	m_oFadeOverListener.Disconnect();
	m_oFadeOverListener.Connect(
	    pFade->GetFadeOverEmitter(),
	    this,
	    []( AAnticTrophyState* a_pState, AFade* a_pFade, TBOOL* a_pThrowFadeOver ) {
		    return a_pState->OnFadeOver( a_pFade, a_pThrowFadeOver );
	    },
	    0
	);

	m_bFadingSound = TTRUE;
	m_fVolume      = 1.0f;
}

// $Barnyard: FUNCTION 00448660
// $Barnyard: FUNCTION 004485b0
TBOOL AAnticTrophyState::OnFadeOver( AFade* a_pFade, TBOOL* a_pThrowFadeOver )
{
	if ( *a_pThrowFadeOver )
		return TFALSE;

	if ( m_bLeave == TTRUE )
	{
		AGameLoader::LeaveMiniGame( TTRUE );
		return TTRUE;
	}

	if ( m_bTryAgain != TTRUE )
		return TFALSE;

	// NOTE: deletes this state
	Remove();

	AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentState();

	const TClass* pCurrentClass = pCurrentState->GetClass();

	// These keep the screen black themselves
	if ( ( pCurrentClass == &TGetClass( ACarChaseMiniGameState ) && AMiniGameManager::GetSingleton()->GetMiniGameVariant() == 4 ) ||
	     pCurrentClass == &TGetClass( APrecisionSquirt ) )
	{
		a_pFade->GetRectangleElement().SetAlpha( 0.0f );
	}

	// Update the minigame right away so the game over state restarts it (or lets it continue) while the screen is black
	if ( pCurrentClass->IsA( &TGetClass( AMiniGameState ) ) )
		pCurrentState->OnUpdate( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );

	return TTRUE;
}
