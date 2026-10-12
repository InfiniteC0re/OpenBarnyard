#include "pch.h"
#include "AFrontEndAttractState.h"
#include "AFrontEndMainMenuState2.h"
#include "AGameStateController.h"
#include "AMovieState.h"
#include "Locale/ALocaleManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/AGameLoader.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AFadeManager.h"
#include "Input/AInputHandler.h"
#include "Cameras/ACameraManager.h"
#include "Tasks/ARootTask.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass, the create and the create-in-place functions of the TClass
// $Barnyard: FUNCTION 00406cf0
// $Barnyard: FUNCTION 004072f0
// $Barnyard: FUNCTION 00407350
TDEFINE_CLASS( AFrontEndAttractState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00406e20
AFrontEndAttractState::AFrontEndAttractState( AWindmillHelper* a_pWindmillHelper )
{
	m_fBlinkTime      = 0.75f;
	m_fIdleTime       = 0.0f;
	m_pWindmillHelper = a_pWindmillHelper;

	SetInputMap( TPS8( DialogInputMap ) );
}

// $Barnyard: FUNCTION 00406d00
// $Barnyard: FUNCTION 00406ee0
AFrontEndAttractState::~AFrontEndAttractState()
{
	m_oFadeOverListener.Disconnect();
}

// $Barnyard: FUNCTION 00406f00
TBOOL AFrontEndAttractState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	return AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) != TTRUE &&
	    !m_oFadeOverListener.IsLinked() &&
	    BaseClass::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 00407390
TBOOL AFrontEndAttractState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) == TTRUE || m_oFadeOverListener.IsLinked() )
		return TFALSE;

	if ( a_eInputCommand != AInputCommand_Start )
		return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	OpenMainMenu();
	return TTRUE;
}

// $Barnyard: FUNCTION 00407410
TBOOL AFrontEndAttractState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( AInputHandler::GetSingleton()->HadInputLastFrame() )
		m_fIdleTime = 0.0f;

	m_fIdleTime += a_fDeltaTime;

	if ( m_fIdleTime < ms_fAttractIdleTime || AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) )
	{
		// The plank blinks
		m_fBlinkTime += a_fDeltaTime;

		if ( m_fBlinkTime >= 0.75f )
		{
			m_fBlinkTime = 0.0f;
			m_oPlank.SetVisible( !m_oPlank.IsVisible() );
		}

		return BaseClass::OnUpdate( a_fDeltaTime );
	}

	// Idle for too long, fade out and play the attract movie
	AFade* pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 0.5f );
	m_oFadeOverListener.Connect( pFade->GetFadeOverEmitter(), this, OnFadeOver, 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 00406f40
void AFrontEndAttractState::OnInsertion()
{
	BaseClass::OnInsertion();

	AGameLoader::Terrain_Set( AGameLoader::Terrain_FrontEnd, TFALSE, TTRUE, 0, 0, 0, 0 );
	AFrontEndMainMenuState2::SetupCamera();

	T2GUIFont*           pFont  = T2GUIFontManager::FindFont( "Rekord18" );
	T2GUITextureSection* pPlank = T2GUITextureSectionManager::GetTextureSection( "Wood_Thin" );

	m_oText.Create( pFont, 300.0f );
	m_oText.SetScale( 1.5f );
	m_oText.SetTextAndFitWidth( g_pLocaleManager->GetString( 290 ) );
	m_oText.SetTransform( 0.0f, 2.0f );

	m_oPlank.SetTextureSection( pPlank );
	m_oPlank.SetDimensions(
	    TMath::Abs( TFLOAT( pPlank->GetPoint2X() - pPlank->GetPoint1X() ) ),
	    TMath::Abs( TFLOAT( pPlank->GetPoint2Y() - pPlank->GetPoint1Y() ) )
	);
	m_oPlank.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oPlank.SetTransform( 0.0f, -20.0f );
	m_oPlank.AddChildTail( m_oText );
	AGUI2::GetRootElement()->AddChildTail( m_oPlank );

	if ( !m_pWindmillHelper )
	{
		m_pWindmillHelper = new AWindmillHelper();
		g_pSimAnimModelHelper->AddModelHelper( m_pWindmillHelper );
		m_pWindmillHelper->CreateBarnSign();
	}

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );

	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTransform( 0.0f, 10.0f );
	m_oTitle.Hide();

	m_oTitleShadow.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitleShadow.SetTransform( 5.0f, 10.0f );
	m_oTitleShadow.SetColour( 0x64000000 );
	m_oTitleShadow.Hide();

	AGUI2::GetRootElement()->AddChildTail( m_oTitleShadow );
	AGUI2::GetRootElement()->AddChildTail( m_oTitle );
}

// $Barnyard: FUNCTION 00406da0
void AFrontEndAttractState::OnRemoval()
{
	m_oTitle.RemoveSelf();
	m_oTitleShadow.RemoveSelf();
	m_oText.RemoveSelf();
	m_oPlank.RemoveSelf();

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00407530
void AFrontEndAttractState::OnActivate()
{
	if ( m_pWindmillHelper )
	{
		// The English barn sign shows the title, the other languages show a localised title on the screen
		T2Locale::Lang eLanguage = g_pLocaleManager->GetLanguage();
		TBOOL          bEnglish  = eLanguage < ALocaleManager::Lang_Japanese;

		m_pWindmillHelper->SetBarnSignAnimated( bEnglish );

		m_oTitle.SetVisible( !bEnglish );

		if ( bEnglish )
		{
			m_oTitleShadow.Hide();
		}
		else
		{
			m_oTitleShadow.Show();

			T2GUITextureSection* pTitle  = T2GUITextureSectionManager::GetTextureSection( "BY_TITLE" );
			TFLOAT               fWidth  = TMath::Abs( TFLOAT( pTitle->GetPoint2X() - pTitle->GetPoint1X() ) );
			TFLOAT               fHeight = TMath::Abs( TFLOAT( pTitle->GetPoint2Y() - pTitle->GetPoint1Y() ) );

			m_oTitle.T2GUIElement::SetDimensions( fWidth, fHeight );
			m_oTitleShadow.T2GUIElement::SetDimensions( fWidth, fHeight );
			m_oTitle.SetTextureSection( pTitle );
			m_oTitleShadow.SetTextureSection( pTitle );

			if ( eLanguage == ALocaleManager::Lang_German || eLanguage == ALocaleManager::Lang_French )
			{
				m_oTitle.SetDimensions( fWidth * 1.1f, fHeight * 1.1f );
				m_oTitleShadow.SetDimensions( fWidth * 1.1f, fHeight * 1.1f );
			}
		}
	}

	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 00406e00
void AFrontEndAttractState::OnDeactivate()
{
	// NOTE: the original gets the memory info here and doesn't use it
	TMemory::MemInfo oMemInfo;
	g_pMemory->GetMemInfo( oMemInfo, TNULL );

	BaseClass::OnDeactivate();
}

// $Barnyard: FUNCTION 00407160
void AFrontEndAttractState::OpenMainMenu()
{
	AGameStateController::GetSingleton()->ReplaceState( new AFrontEndMainMenuState2( m_pWindmillHelper, TTRUE ) );
}

// $Barnyard: FUNCTION 004071e0
TBOOL AFrontEndAttractState::StartAttractMovie()
{
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	AFrontEndAttractState* pAttractState = new AFrontEndAttractState( m_pWindmillHelper );
	AMovieState*           pMovieState   = new AMovieState( "Attract", TTRUE, pAttractState, TFALSE );

	m_oFadeOverListener.Disconnect();
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
	AGameStateController::GetSingleton()->ReplaceState( pMovieState );

	return TTRUE;
}

// $Barnyard: FUNCTION 004073f0
TBOOL __stdcall AFrontEndAttractState::OnFadeOver( AFrontEndAttractState* a_pState, AFade* a_pFade, TBOOL* a_pData )
{
	return a_pState->StartAttractMovie();
}
