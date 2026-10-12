#include "pch.h"
#include "AFrontEndMainMenuState2.h"
#include "AGameStateController.h"
#include "Locale/ALocaleManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "Player/APlayerManager.h"
#include "Helpers/AWindmillHelper.h"
#include "GUI/AGUI2.h"
#include "GUI/AFadeManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/music.h"
#include "Sound/AMusicManager.h"
#include "Input/AInputHandler.h"
#include "GameInterface/AMovieState.h"
#include "GameInterface/AFrontEndMiniGameState2.h"
#include "GameInterface/AFrontEndPlayersState.h"
#include "GameInterface/AFrontEndNewGameSelectState.h"
#include "GameInterface/AFrontEndLoadState.h"
#include "GameInterface/AFrontEndBonusState.h"
#include "GameInterface/AFrontEndCharacterSelectState.h"
#include "Helpers/AGameLoader.h"
#include "AppBoot.h"
#include "GameInterface/SaveLoadSKU.h"
#include "Helpers/APlayerProgress.h"
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

// NOTE: GetClass 00408de0, CreateObject 004095c0, CreateObjectInPlace 00409620
TDEFINE_CLASS( AFrontEndMainMenuState2 );

// $Barnyard: FUNCTION 00408f20
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

// $Barnyard: FUNCTION 00408df0
// $Barnyard: FUNCTION 00409050
AFrontEndMainMenuState2::~AFrontEndMainMenuState2()
{
}

// $Barnyard: FUNCTION 004094c0
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

// $Barnyard: FUNCTION 004093d0
TBOOL AFrontEndMainMenuState2::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( !AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) && !m_oFadeOverListener.IsLinked() )
	{
		return ABYardMenuState::ProcessInput( a_pInputEvent );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00409410
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

// $Barnyard: FUNCTION 00409660
static TBOOL __stdcall OnIntroFadeOver( AFrontEndMainMenuState2* a_pFrontEndState, AFade* a_pFade, TBOOL* a_pData )
{
	return a_pFrontEndState->StartIntroVideo();
}

// $Barnyard: FUNCTION 00409910
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
			    pFade->GetFadeOverEmitter(), this, OnIntroFadeOver, 0
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
	//	AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );

	ms_bIsInserted = TTRUE;

	if ( !m_pWindmillHelper )
	// Create windmill helper and a barn sign
	{
		m_pWindmillHelper = new AWindmillHelper();
		g_pSimAnimModelHelper->AddModelHelper( m_pWindmillHelper );

		m_pWindmillHelper->CreateBarnSign();
	}

	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	for ( TINT i = 0; i < FRONTENDBUTTON_NUMOF; i++ )
	{
		const TWCHAR* wszText = g_pLocaleManager->GetString( FRONTENDBUTTON_TEXTS[ i ] );

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
	m_oMenu.SetItemFocusCallback( MenuItemFocusCallback );

	m_oDialogTitleBackground.Hide();
	m_Rectangle1.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_Rectangle1.SetTransform( 0.0f, 10.0f );
	m_Rectangle1.Hide();

	m_Rectangle2.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
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

// $Barnyard: FUNCTION 00409340
void AFrontEndMainMenuState2::OnRemoval()
{
	ms_bIsInserted = TFALSE;


	AMusicManager::GetSingleton()->StopBackgroundMusic();
	ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( m_pWindmillHelper );
	m_pWindmillHelper->OnDestroy();

	if ( m_pWindmillHelper )
		delete m_pWindmillHelper;

	m_pWindmillHelper = TNULL;

	for ( TINT i = 0; i < FRONTENDBUTTON_NUMOF; i++ )
	{
		m_aButtons[ i ].RemoveSelf();
	}

	m_Rectangle1.RemoveSelf();
	m_Rectangle2.RemoveSelf();

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00409680
void AFrontEndMainMenuState2::OnActivate()
{
	m_fAFKTime           = 0.0f;
	m_iActivatedButtonID = -1;

	m_Rectangle1.Show();
	m_Rectangle2.Show();

	AInputHandler* pInputHandler = AInputHandler::GetSingleton();

	if ( pInputHandler->IsRecordingLostControllers() == TTRUE )
		pInputHandler->SetRecordLostControllers( TFALSE );

	APlayerManager::GetSingleton()->GetCurrentPlayerSet()->Reset();

	BaseClass::OnActivate();
	m_eMenuState = MENUSTATE_MENU_APPEAR;

	m_oDialogTitleBackground.Hide();

	// NOTE: the original also clears a global here (DAT_007810e9) that is never read

	if ( m_pWindmillHelper )
	{
		// The English barn sign shows the title, the other languages show a localised title on the screen
		T2Locale::Lang eLanguage = g_pLocaleManager->GetLanguage();
		TBOOL          bEnglish  = eLanguage < ALocaleManager::Lang_Japanese;

		m_pWindmillHelper->SetBarnSignAnimated( bEnglish );

		if ( bEnglish )
		{
			m_Rectangle1.Hide();
			m_Rectangle2.Hide();
		}
		else
		{
			m_Rectangle1.Show();
			m_Rectangle2.Show();

			T2GUITextureSection* pTitle  = T2GUITextureSectionManager::GetTextureSection( "BY_TITLE" );
			TFLOAT               fWidth  = TMath::Abs( TFLOAT( pTitle->GetPoint2X() - pTitle->GetPoint1X() ) );
			TFLOAT               fHeight = TMath::Abs( TFLOAT( pTitle->GetPoint2Y() - pTitle->GetPoint1Y() ) );

			m_Rectangle1.T2GUIElement::SetDimensions( fWidth, fHeight );
			m_Rectangle2.T2GUIElement::SetDimensions( fWidth, fHeight );
			m_Rectangle1.SetTextureSection( pTitle );
			m_Rectangle2.SetTextureSection( pTitle );

			if ( eLanguage == ALocaleManager::Lang_German || eLanguage == ALocaleManager::Lang_French )
			{
				m_Rectangle1.SetDimensions( fWidth * 1.1f, fHeight * 1.1f );
				m_Rectangle2.SetDimensions( fWidth * 1.1f, fHeight * 1.1f );
			}
		}
	}
}

// $Barnyard: FUNCTION 00408ea0
void AFrontEndMainMenuState2::OnDeactivate()
{
	m_Rectangle1.Hide();
	m_Rectangle2.Hide();

	BaseClass::OnDeactivate();
}

// $Barnyard: FUNCTION 00408ec0
void AFrontEndMainMenuState2::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );

	m_iActivatedButtonID = pButton->GetId();
	OnButtonActivated( pButton );

	m_eMenuState = MENUSTATE_MENU_VISIBLE;
}

// $Barnyard: FUNCTION 004099f0
void AFrontEndMainMenuState2::OnMenuClose()
{
	if ( m_iActivatedButtonID == -1 )
		return;

	AGameState* pPushGameState = TNULL;

	switch ( m_iActivatedButtonID )
	{
		case FRONTENDBUTTON_NEWGAME:
		{
			// Load the save file, then choose the slot of the new game
			SaveLoadSKU* pSaveLoad = new SaveLoadSKU( APlayerProgress::GetSingleton(), TFALSE, new AFrontEndNewGameSelectState() );
			pSaveLoad->StartMode3();
			pPushGameState = pSaveLoad;
			break;
		}
		case FRONTENDBUTTON_LOADGAME:
		{
			// Load the save file, then choose the slot to continue from
			SaveLoadSKU* pSaveLoad = new SaveLoadSKU( APlayerProgress::GetSingleton(), TFALSE, new AFrontEndLoadState() );
			pSaveLoad->StartMode2();
			pPushGameState = pSaveLoad;
			break;
		}
		case FRONTENDBUTTON_OPTIONS:
			pPushGameState = new AOptionsState();
			break;
		case FRONTENDBUTTON_BONUSES:
			pPushGameState = new AFrontEndBonusState( AFrontEndBonusState::MODE_BONUSES );
			break;
		case FRONTENDBUTTON_ANTICS:
#ifdef BARNYARD_COMMUNITY_PATCH
			// Development: "-multiplayerantics" opens the abandoned player count selection test
			// (AFrontEndPlayersState, recovered from the original source) before the minigames
			if ( TStringManager::String8FindString( GetCommandLineA(), "-multiplayerantics" ) )
			{
				pPushGameState = new AFrontEndPlayersState();
				break;
			}

			// Development: "-devsseq" opens the scripted sequence debug menu (AFrontEndDebugSSeqState)
			// after choosing the character, the way AFrontEndCharacterSelectState supports it (unused in retail)
			if ( TStringManager::String8FindString( GetCommandLineA(), "-devsseq" ) )
			{
				pPushGameState = new AFrontEndCharacterSelectState( TTRUE, TFALSE, TTRUE );
				break;
			}
#endif // BARNYARD_COMMUNITY_PATCH

			// Choose the character, then the minigame
			pPushGameState = new AFrontEndCharacterSelectState( TTRUE, TFALSE, TFALSE );
			break;
		case FRONTENDBUTTON_QUIT:
		{
			AMessagePopupState* pPopup = new AMessagePopupState(
			    g_pLocaleManager->GetString( 333 ),
			    g_pLocaleManager->GetString( 1065 ),
			    TPString8(),
			    11,
			    TFALSE,
			    AInputCommand_Yes,
			    AInputCommand_No,
			    AInputCommand_Empty,
			    AInputCommand_Empty,
			    TFALSE,
			    60.0f,
			    0
			);

			m_oQuitPopupListener.Connect( pPopup->GetResponseEmitter(), this, OnQuitPopupResponse, 0 );
			pPushGameState = pPopup;
			break;
		}
	}

	if ( pPushGameState )
		AGameStateController::GetSingleton()->PushState( pPushGameState );
}

// $Barnyard: FUNCTION 00408ef0
TBOOL __stdcall AFrontEndMainMenuState2::OnQuitPopupResponse( AFrontEndMainMenuState2* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	if ( a_pEvent->eButton == AInputCommand_Yes )
	{
		AGameLoader::UnloadForExit();
		g_oTheApp.Destroy();
	}
	else
	{
		a_pPopup->Remove();
	}

	return TFALSE;
}
