#include "pch.h"
#include "AFrontEndMainMenuState2.h"
#include "AGameStateController.h"
#include "Locale/ALocaleManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "GUI/AGUI2FontManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AFadeManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/music.h"
#include "Sound/AMusicManager.h"
#include "Input/AInputHandler.h"
#include "GameInterface/AMovieState.h"
#include "AOptionsState.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ABoneAttachCameraHelper.h"
#include "Assets/AKeyFrameLibraryManager.h"

#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AFrontEndMainMenuState2 );

AFrontEndMainMenuState2::AFrontEndMainMenuState2( AWindmillHelper* a_pWindmillHelper, TBOOL a_bFlag )
{
	m_iActivatedButtonID = -1;
	m_pWindmillHelper    = a_pWindmillHelper;
	m_bFlag              = a_bFlag;
	m_fAFKTime           = 0.0f;

	m_pButtonRotations                            = new TFLOAT[ FRONTENDBUTTON_NUMOF ];
	m_pButtonRotations[ FRONTENDBUTTON_NEWGAME ]  = TMath::DegToRad( 2.0f );
	m_pButtonRotations[ FRONTENDBUTTON_LOADGAME ] = TMath::DegToRad( -3.0f );
	m_pButtonRotations[ FRONTENDBUTTON_OPTIONS ]  = TMath::DegToRad( 2.0f );
	m_pButtonRotations[ FRONTENDBUTTON_BONUSES ]  = TMath::DegToRad( -2.0f );
	m_pButtonRotations[ FRONTENDBUTTON_ANTICS ]   = TMath::DegToRad( 2.0f );
	m_pButtonRotations[ FRONTENDBUTTON_QUIT ]     = TMath::DegToRad( -1.0f );
}

AFrontEndMainMenuState2::~AFrontEndMainMenuState2()
{
}

TBOOL AFrontEndMainMenuState2::StartIntroVideo()
{
	AFrontEndMainMenuState2* pFrontEndState = new AFrontEndMainMenuState2();
	AMovieState*             pMovieState    = new AMovieState( "Attract", TTRUE, pFrontEndState, TFALSE );

	// Disconnect event listener and fade out screen
	m_oFadeOverListener.Disconnect();
	AFade* pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
	AGameStateController::GetSingleton()->ReplaceState( pMovieState );

	return TTRUE;
}

// $Barnyard: FUNCTION 004245f0
void AFrontEndMainMenuState2::SetupCamera()
{
	ACameraManager* pCamMngr = ACameraManager::GetSingleton();

	ABoneAttachCameraHelper* pBoneCamera = TSTATICCAST(
	    ABoneAttachCameraHelper,
	    pCamMngr->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH )
	);

	if ( pCamMngr->GetCurrentCameraHelper() != pBoneCamera )
	{
		// Setup the camera helper
		pBoneCamera->SetModel( TPS8D( "data\\models\\envfrontend_cam" ), "camera" );
		pCamMngr->SetCameraHelper( pBoneCamera );

		pBoneCamera->SetAnimation( "camera_loop" );
	}
}

TBOOL AFrontEndMainMenuState2::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( !AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) && !m_oFadeOverListener.IsLinked() )
	{
		return ABYardMenuState::ProcessInput( a_pInputEvent );
	}

	return TFALSE;
}

TBOOL AFrontEndMainMenuState2::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( !AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) && !m_oFadeOverListener.IsLinked() )
	{
		if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
		{
			if ( ( !a_pInputEvent->GetSource()->IsA( &TGetClass( TInputDeviceKeyboard ) ) || a_pInputEvent->GetDoodad() != TInputDeviceKeyboard::KEY_ESCAPE ) &&
			     ( !a_pInputEvent->GetSource()->IsA( &TGetClass( TInputDeviceController ) ) || a_eInputCommand != AInputCommand_Cancel ) )
			{
				return ABYardMenuState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
			}
			else
			{
				// Cancel command was sent
				m_iActivatedButtonID = FRONTENDBUTTON_QUIT;
				m_eMenuState         = MENUSTATE_MENU_DISAPPEAR;
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

TBOOL AFrontEndMainMenuState2::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Reset AFK time
	if ( AInputHandler::GetSingleton()->HadInputLastFrame() )
		m_fAFKTime = 0.0f;

	m_fAFKTime += a_fDeltaTime;

	if ( ms_fPlayIntroAFKTime <= m_fAFKTime )
	{
		if ( !AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) )
		{
			// Fade in screen and start intro video after that
			AFade* pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 0.5f );
			m_oFadeOverListener.Connect(
			    pFade->GetFadeOverEmitter(), this, []( AFrontEndMainMenuState2* a_pFrontEndState, AFade* a_pFade, TBOOL* ) { return a_pFrontEndState->StartIntroVideo(); }, 0
			);
		}
	}

	UpdateMenuButtons( a_fDeltaTime );

	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && m_bIgnoreInputs )
	{
		UpdateFocusedButton( a_fDeltaTime );
	}

	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00409070
void AFrontEndMainMenuState2::OnInsertion()
{
	// Note: commented out because it gives black screen atm
	//if ( !m_bFlag )
	//	AGameStateController::GetSingleton()->SetFlags( 4 );

	ms_bIsInserted = TTRUE;

	if ( !m_pWindmillHelper )
	// Create windmill helper and a barn sign
	{
		m_pWindmillHelper = new AWindmillHelper();
		ASimAnimModelHelperManager::GetSingleton()->AddModelHelper( m_pWindmillHelper );

		m_pWindmillHelper->CreateBarnSign();
	}

	AGUI2Font* pFont = AGUI2FontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	for ( TINT i = 0; i < FRONTENDBUTTON_NUMOF; i++ )
	{
		const TWCHAR* wszText = ALocaleManager::GetSingleton()->GetString( FRONTENDBUTTON_TEXTS[ i ] );

		m_aButtons[ i ].Create( pFont, pFont->GetTextWidth( wszText ), wszText, 5.0f );
		m_aButtons[ i ].SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		m_aButtons[ i ].SetEnabledColour( AGUI2STYLE_COLOR_PRIMARY );
		m_aButtons[ i ].SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		m_aButtons[ i ].SetFocusedScale( 1.0f );
		m_aButtons[ i ].OnFocusLost();
		m_aButtons[ i ].SetId( i );
		m_aButtons[ i ].SetScaleOnFocus( TFALSE );
		m_aButtons[ i ].GetTransform().Rotate( m_pButtonRotations[ i ] );
		m_aButtons[ i ].SetImage( "Panel_01", 1.25f, 1.5f );
		m_aButtons[ i ].SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		m_aButtons[ i ].SetTextShadowOffset( 5.0f, 5.0f );
		m_oMenu.AddMenuItem( m_aButtons[ i ] );
	}

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oMenu.SetItemFocusCallback( []( void* a_pUserData, AGUI2MenuItem* a_pOldFocus, AGUI2MenuItem* a_pNewFocus ) {
		TSTATICCAST( AFrontEndMainMenuState2, a_pUserData )->OnMenuItemFocused( *a_pOldFocus, *a_pNewFocus );
	} );

	m_oDialogTitleBackground.Hide();
	m_Rectangle1.SetAttachment( AGUI2ATTACHMENT_TOPCENTER, AGUI2ATTACHMENT_TOPCENTER );
	m_Rectangle1.SetTransform( 0.0f, 10.0f );
	m_Rectangle1.Hide();

	m_Rectangle2.SetAttachment( AGUI2ATTACHMENT_TOPCENTER, AGUI2ATTACHMENT_TOPCENTER );
	m_Rectangle2.SetTransform( 5.0f, 10.0f );
	m_Rectangle2.SetColour( 0x64000000 );
	m_Rectangle2.Hide();

	AGUI2::GetRootElement()->AddChildTail( m_Rectangle2 );
	AGUI2::GetRootElement()->AddChildTail( m_Rectangle1 );

	AMusicManager::GetSingleton()->PlayBackgroundMusic( soundbank::MUSIC_START_SCREEN );
	BaseClass::OnInsertion();

	m_oMenu.SetFocusSound( soundbank::UI_FE_MOVE );
	m_oMenu.SetSelectSound( soundbank::UI_FE_SELECT );
	m_oMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oMenu.SetNegativeSound( soundbank::UI_NEGATIVE );

	m_oDialog.SetTransform( 0.0f, 50.0f );
	m_oMenu.SetTransform( 0.0f, 50.0f );

	m_oDialog.Hide();
	m_bFocusedElementBloated = TFALSE;
}

void AFrontEndMainMenuState2::OnRemoval()
{
	ms_bIsInserted = TFALSE;


	AMusicManager::GetSingleton()->StopBackgroundMusic();
	TTODO( "FUN_0054b390" );

	for ( TINT i = 0; i < FRONTENDBUTTON_NUMOF; i++ )
	{
		m_aButtons[ i ].RemoveSelf();
	}

	m_Rectangle1.RemoveSelf();
	m_Rectangle2.RemoveSelf();

	BaseClass::OnRemoval();
}

void AFrontEndMainMenuState2::OnActivate()
{
	TIMPLEMENT();

	m_fAFKTime           = 0.0f;
	m_iActivatedButtonID = -1;

	m_Rectangle1.Show();
	m_Rectangle2.Show();

	BaseClass::OnActivate();
	m_eMenuState = MENUSTATE_MENU_APPEAR;

	m_oDialogTitleBackground.Hide();
}

void AFrontEndMainMenuState2::OnDeactivate()
{
	m_Rectangle1.Hide();
	m_Rectangle2.Hide();

	BaseClass::OnDeactivate();
}

void AFrontEndMainMenuState2::OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, &a_rMenuItem );

	m_iActivatedButtonID = pButton->GetId();
	OnButtonActivated( pButton );

	m_eMenuState = MENUSTATE_MENU_VISIBLE;
}

void AFrontEndMainMenuState2::OnMenuClose()
{
	TIMPLEMENT();

	if ( m_iActivatedButtonID == -1 )
		return;

	AGameState* pPushGameState = TNULL;

	switch ( m_iActivatedButtonID )
	{
		case FRONTENDBUTTON_OPTIONS:
			pPushGameState = new AOptionsState();
			break;
	}

	if ( pPushGameState )
		AGameStateController::GetSingleton()->PushState( pPushGameState );
}

void AFrontEndMainMenuState2::UpdateMenuButtons( TFLOAT a_fDeltaTime )
{
	AGUI2MenuItem* pLastMenuItem = m_oMenu.GetLastMenuItem();
	AGUI2MenuItem* pFocused      = m_oMenu.GetFocusedMenuItem();

	if ( pLastMenuItem != TNULL )
	{
		AGUI2Button* pItem = TSTATICCAST( AGUI2Button, m_oMenu.GetLastMenuItem() );

		while ( TTRUE )
		{
			TFLOAT fBackgroundScaleX, fBackgroundScaleY;
			TFLOAT fTextScaleX, fTextScaleY;

			if ( pItem == pFocused )
			{
				TFLOAT fSpeed     = a_fDeltaTime * 10.0f;
				fBackgroundScaleX = TMath::LERPClamped( pItem->GetBackground().GetTransform().m_aMatrixRows[ 0 ].x, ms_fFocusedButtonMaxScale, fSpeed );
				fBackgroundScaleY = TMath::LERPClamped( pItem->GetBackground().GetTransform().m_aMatrixRows[ 1 ].y, ms_fFocusedButtonMaxScale, fSpeed );
				fTextScaleX       = TMath::LERPClamped( pItem->GetTextBox().GetTransform().m_aMatrixRows[ 0 ].x, ms_fFocusedButtonTextMaxScale, fSpeed );
				fTextScaleY       = TMath::LERPClamped( pItem->GetTextBox().GetTransform().m_aMatrixRows[ 1 ].y, ms_fFocusedButtonTextMaxScale, fSpeed );
			}
			else
			{
				TFLOAT fSpeed     = a_fDeltaTime * 20.0f;
				fBackgroundScaleX = TMath::LERPClamped( pItem->GetBackground().GetTransform().m_aMatrixRows[ 0 ].x, 1.0f, fSpeed );
				fBackgroundScaleY = TMath::LERPClamped( pItem->GetBackground().GetTransform().m_aMatrixRows[ 1 ].y, 1.0f, fSpeed );
				fTextScaleX       = TMath::LERPClamped( pItem->GetTextBox().GetTransform().m_aMatrixRows[ 0 ].x, 1.0f, fSpeed );
				fTextScaleY       = TMath::LERPClamped( pItem->GetTextBox().GetTransform().m_aMatrixRows[ 1 ].y, 1.0f, fSpeed );
			}

			pItem->GetBackground().GetTransform().SetScale( fBackgroundScaleX, fBackgroundScaleX );
			pItem->GetTextBox().GetTransform().SetScale( fTextScaleX, fTextScaleY );

			pItem = TSTATICCAST( AGUI2Button, pItem->GetPrevMenuItem() );
			if ( pItem == pLastMenuItem ) break;
		}
	}
}

void AFrontEndMainMenuState2::UpdateFocusedButton( TFLOAT a_fDeltaTime )
{
	AGUI2Button* pFocusedButton = TSTATICCAST( AGUI2Button, m_oMenu.GetFocusedMenuItem() );

	ms_fAnimationTime += a_fDeltaTime;
	TFLOAT fProgress = TMath::Sin( ms_fFocusedButtonAnimSpeed * ms_fAnimationTime );

	if ( ms_fAnimationTime < TMath::PI / ms_fFocusedButtonAnimSpeed )
	{
		TFLOAT fOffset = ms_fFocusedButtonMaxOffset * fProgress;
		pFocusedButton->GetTextBox().SetTranslation( ms_vecActivatedButtonTranslation.x + fOffset, ms_vecActivatedButtonTranslation.y + fOffset );

		TFLOAT fOneMinusProgress = 1.0f - fProgress;
		pFocusedButton->SetShadowAlpha( fOneMinusProgress );
		pFocusedButton->SetTextShadowOffset( ms_vecActivatedButtonShadow.x * fOneMinusProgress, ms_vecActivatedButtonShadow.y * fOneMinusProgress );
	}
	else
	{
		m_bIgnoreInputs = TFALSE;
		m_eMenuState    = MENUSTATE_MENU_DISAPPEAR;

		pFocusedButton->SetTextShadowOffset( ms_vecActivatedButtonShadow.x, ms_vecActivatedButtonShadow.y );
		pFocusedButton->GetTextBox().SetTranslation( ms_vecActivatedButtonTranslation.x, ms_vecActivatedButtonTranslation.y );
	}
}
