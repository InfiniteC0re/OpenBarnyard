#include "pch.h"
#include "AMiniGameInstructionState.h"
#include "AGameStateController.h"
#include "ALoadScreen.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "Locale/ALocaleManager.h"
#include "Tasks/ARootTask.h"
#include "MiniGame/AMiniGameManager.h"
#include "Helpers/AGameLoader.h"
#include "GUI/AGUISlideshow.h"
#include "Render/ARenderer.h"

#include <Input/TInputDeviceController.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0046c450, CreateObject 0046c580, CreateObjectInPlace 0046c5e0
TDEFINE_CLASS( AMiniGameInstructionState );

TPSTRING8_DECLARE( DialogInputMap );
TPSTRING8_DECLARE( ACowTippingMiniGameState );
TPSTRING8_DECLARE( bkg_Showdown );

// $Barnyard: FUNCTION 0046c460
AMiniGameInstructionState::AMiniGameInstructionState()
    : m_fBlinkTimer( BLINK_INTERVAL )
    , m_fTotalTime( 0.0f )
{
	SetInputMap( TPS8( DialogInputMap ) );
}

// $Barnyard: FUNCTION 0046c4e0
// $Barnyard: FUNCTION 0046c620
AMiniGameInstructionState::~AMiniGameInstructionState()
{
}

// $Barnyard: FUNCTION 0046c640
TBOOL AMiniGameInstructionState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceController ) ) )
	{
		if ( a_pInputEvent->GetDoodad() == TInputDeviceController::BUTTON_START && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			if ( m_pMiniGame->CanDismissInstructions() )
				Dismiss();

			return TTRUE;
		}
	}
	else if ( a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
	{
		TINT iDoodad = a_pInputEvent->GetDoodad();

		if ( ( iDoodad == TInputDeviceKeyboard::KEY_ESCAPE || iDoodad == TInputDeviceKeyboard::KEY_RETURN || iDoodad == TInputDeviceKeyboard::KEY_SPACE ) &&
		     a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			if ( m_pMiniGame->CanDismissInstructions() )
				Dismiss();

			return TTRUE;
		}
	}
	else if ( a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) && a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
	{
		TryDismiss();
		return TTRUE;
	}

	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 0046c560
void AMiniGameInstructionState::TryDismiss()
{
	if ( m_pMiniGame->CanDismissInstructions() )
		Dismiss();
}

// $Barnyard: FUNCTION 0046c780
TBOOL AMiniGameInstructionState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fBlinkTimer += a_fDeltaTime;
	m_fTotalTime += a_fDeltaTime;

	if ( m_fBlinkTimer >= BLINK_INTERVAL )
	{
		m_oTextFrame.SetVisible( !m_oTextFrame.IsVisible() );
		m_fBlinkTimer = 0.0f;
	}

	// NOTE: the screen never times out in retail, the limit is FLT_MAX
	if ( m_fTotalTime > TMath::MAXFLOAT )
	{
		if ( m_pMiniGame->CanDismissInstructions() )
			Dismiss();

		return TTRUE;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0046c830
// $BarnyardWiiJP: FUNCTION 8018ba0c
void AMiniGameInstructionState::OnInsertion()
{
	AGameState::OnInsertion();

	T2GUIFont*           pFont           = T2GUIFontManager::FindFont( "Rekord18" );
	T2GUITextureSection* pTextureSection = T2GUITextureSectionManager::GetTextureSection( "Wood_Thin" );

	// Text
	m_oTextBox.Create( pFont, 300.0f );
	m_oTextBox.SetScale( 1.5f );

	const TWCHAR* wszText = g_pLocaleManager->GetString( 0x20c );
	m_oTextBox.SetText( wszText );
	m_oTextBox.SetWidth( pFont->GetTextWidth( wszText, 1.0f ) * m_oTextBox.GetScale() );
	m_oTextBox.SetTransform( 0.0f, -8.0f );

	TFLOAT fRootWidth  = 0.0f;
	TFLOAT fRootHeight = 0.0f;

	if ( T2GUIElement* pRootElement = AGUI2::GetRootElement() )
	{
		pRootElement->GetDimensions( fRootWidth, fRootHeight );
		m_oTextBox.SetTransform( 0.0f, fRootHeight * -0.06f );
	}

	// Frame of the text
	m_oTextFrame.SetTextureSection( pTextureSection );
	m_oTextFrame.SetDimensions( pTextureSection->GetWidth(), pTextureSection->GetHeight() );
	m_oTextFrame.SetTransform( 0.0f, -20.0f );
	m_oTextFrame.AddChildTail( m_oTextBox );
	m_oTextFrame.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	AGUI2::GetRootElement()->AddChildTail( m_oTextFrame );
	m_oTextFrame.Hide();

	// Loading picture of the minigame (the "Showdown" variant of cow tipping has its own one)
	AMiniGameManager* pMiniGameManager = AMiniGameManager::GetSingleton();
	TINT              iMiniGame        = AGameLoader::GetCurrMiniGame();

	TPString8 strStateClassName;
	TBOOL     bIsShowdown = pMiniGameManager->GetStateClassName( strStateClassName, iMiniGame ) == TPS8( ACowTippingMiniGameState ) && pMiniGameManager->GetMiniGameVariant() == 2;

	TPString8 strPicture;

#ifdef BARNYARD_LATEST
	// Wii: the XUI load screen of the minigame stays until the player dismisses it (the loader already
	// shows it, see AGameLoader's ShowXUILoadScreen). The prompt (m_oTextFrame) is drawn over it.
	TPString8 strXUILoadScreen = bIsShowdown ? TPString8( "CoyoteShowdown_LoadScreen" ) : pMiniGameManager->GetXUILoadScreenName( iMiniGame );

	if ( !strXUILoadScreen.IsEmpty() && AGUI2::GetSingleton()->GetXUI().FindResource( strXUILoadScreen.GetString() ) )
	{
		g_oLoadScreen.ShowXUIBackground( TTRUE, TFALSE );
		g_oLoadScreen.SetXUIScene( strXUILoadScreen.GetString(), 0 );
		ARenderer::GetSingleton()->ForceUpdate30FPS();
	}
	// NOTE: the Wii build always has the scene. On PC the Wii data is optional: without it, the PC picture is shown
	else
#endif
	    if ( bIsShowdown )
	{
		AGUISlideshow::LocaliseBackgroundFileName( strPicture, TPS8( bkg_Showdown ) );
		g_oLoadScreen.SetPicture( strPicture, TTRUE );
	}
	else
	{
		pMiniGameManager->GetLoadingScreenName( strPicture, iMiniGame );
		g_oLoadScreen.SetPicture( strPicture, TFALSE );
	}

	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	ARootTask::GetSingleton()->SetPaused( TTRUE );
	AGameStateController::ms_bPausedByMenu = TTRUE;

	m_pMiniGame->OnInstructionsShown();
}

// $Barnyard: FUNCTION 0046c730
void AMiniGameInstructionState::OnRemoval()
{
	m_oTextFrame.RemoveSelf();
	m_oTextBox.RemoveSelf();

	ARootTask::GetSingleton()->SetPaused( TFALSE );
	AGameStateController::ms_bPausedByMenu = TFALSE;

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0046cac0
void AMiniGameInstructionState::Dismiss()
{
	// NOTE: retail also creates a TPString8 of the minigame's state class name
	// (AMiniGameManager::GetStateClass) here and never uses it.

	g_oLoadScreen.Reset();

	AGameState* pMiniGame = m_pMiniGame;
	Remove();

	if ( pMiniGame )
		pMiniGame->OnStart();
}
