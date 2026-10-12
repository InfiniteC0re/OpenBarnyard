#include "pch.h"
#include <Render/TCameraObject.h>
#include "AFarmerTrainMicroGame.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AStartMenuState.h"
#include "GUI/AGUI2.h"
#include "HUD/AHudManager.h"
#include "HUD/AInteractionHudElement.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Units/AInteractionComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnitPlayer.h"
#include "Units/Projectiles/AProjectileLauncher.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00534570, CreateObject 00534690 and CreateObjectInPlace 005346f0 of the TClass
// $Barnyard: FUNCTION 00534690
// $Barnyard: FUNCTION 005346f0
TDEFINE_CLASS( AFarmerTrainMicroGame );

TPSTRING8_DECLARE( NormalBarnyard );

// $Barnyard: FUNCTION 00534580
AFarmerTrainMicroGame::AFarmerTrainMicroGame()
{
	m_eFlags       = FLAGS_NONE;
	m_fBlinkTime   = 0.0f;
	m_fToggleDelay = 0.0f;
	m_pHudElement  = TNULL;
}

// $Barnyard: FUNCTION 005345f0
// $Barnyard: FUNCTION 00534730
AFarmerTrainMicroGame::~AFarmerTrainMicroGame()
{
}

// $Barnyard: FUNCTION 00534ba0
TBOOL AFarmerTrainMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	a_rStopEvents = TFALSE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
			AGameStateController::GetSingleton()->PushState( new AStartMenuState() );
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
		case AInputCommand_Move:
		case AInputCommand_Interact:
		case AInputCommand_Shoot:
		case AInputCommand_ShootAlt:
		case AInputCommand_Inventory:
			return TTRUE;
		case AInputCommand_RotateCamera:
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
		case AInputCommand_Quad:
			if ( m_fToggleDelay > 0.0f )
				return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

			ToggleSwappedModel();
			return TTRUE;
		default:
			if ( m_eFlags & FLAGS_SWAPPED )
				return TTRUE;

			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 005349f0
TBOOL AFarmerTrainMicroGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Blinks the text
	m_fBlinkTime += a_fDeltaTime;

	if ( m_fBlinkTime > 0.5f )
	{
		m_oText.SetVisible( !m_oText.IsVisible() );
		m_fBlinkTime = 0.0f;
	}

	if ( m_eFlags & FLAGS_SWAPPED )
		m_fSwappedTime += a_fDeltaTime;

	AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayer( 0 ) );

	if ( pPlayer )
	{
		if ( AProjectileLauncher* pLauncher = pPlayer->GetActiveLauncher() )
			pLauncher->Update( a_fDeltaTime );
	}

	if ( m_fSwappedTime > m_fMaxSwappedTime )
		Remove();

	return TTRUE;
}

// $Barnyard: FUNCTION 00534750
void AFarmerTrainMicroGame::OnInsertion()
{
	m_HUDParams.SetFlags( AHudElement::MASK_INTERACTION );
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( NormalBarnyard ) );

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	TSTATICCAST( AInteractionComponent, pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) )->LeaveSequence( TFALSE );

	APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pPlayerControl )
	{
		pPlayerControl->SetContextEnabled( TFALSE );
		pPlayerControl->SetOverrideText( TNULL );
	}

	m_eFlags          = FLAGS_NONE;
	m_wszHudText[ 0 ] = L'\0';

	m_pHudElement = new AInteractionHudElement();
	m_pHudElement->SetForceHide( TTRUE );
	AHudManager::GetSingleton()->AddElement( m_pHudElement, TFALSE, TTRUE, TFALSE );

	T2GUIElement* pRoot = AGUI2::GetRootElement();

	m_oText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 600.0f );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oText.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oText.SetText( g_pLocaleManager->GetString( 920 ) );
	m_oText.Show();
	pRoot->AddChildTail( m_oText );

	m_fSwappedTime    = 0.0f;
	m_fMaxSwappedTime = 5.0f;

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 005348b0
void AFarmerTrainMicroGame::OnRemoval()
{
	m_oText.RemoveSelf();

	AUnit*                   pPlayer        = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pPlayerControl )
		pPlayerControl->SetContextEnabled( TTRUE );

	m_wszHudText[ 0 ] = L'\0';

	if ( m_pHudElement )
	{
		m_pHudElement->Detach();

		if ( m_pHudElement )
			delete m_pHudElement;

		m_pHudElement = TNULL;
	}

	TSTATICCAST( AUnitPlayer, pPlayer )->SetSwappedModel( TFALSE );
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00534940
void AFarmerTrainMicroGame::OnActivate()
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	ACameraManager::GetSingleton()->SetSplitscreenCameraHelper( 1 );
	ACameraManager::GetSingleton()->GetCurrentCameraHelper()->SetTargetUnit( pPlayer );

	if ( m_eFlags & FLAGS_SWAPPED )
	{
		m_pHudElement->SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
		m_pHudElement->SetForceHide( TTRUE );
	}
	else
	{
		ALocale::FormatInputTags( m_wszHudText, 64, 921 );
		m_pHudElement->SetText( m_wszHudText, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
		m_pHudElement->SetForceHide( TFALSE );
	}

	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00534650
void AFarmerTrainMicroGame::OnDeactivate()
{
	m_pHudElement->SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
	m_pHudElement->SetForceHide( TTRUE );

	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00534ac0
void AFarmerTrainMicroGame::ToggleSwappedModel()
{
	if ( m_eFlags & FLAGS_SWAPPED )
	{
		ALocale::FormatInputTags( m_wszHudText, 64, 921 );
		m_pHudElement->SetText( m_wszHudText, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
		m_pHudElement->SetForceHide( TFALSE );
		m_eFlags &= ~FLAGS_SWAPPED;
	}
	else
	{
		m_pHudElement->SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
		m_pHudElement->SetForceHide( TTRUE );
		m_eFlags |= FLAGS_SWAPPED;
	}

	TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )->SetSwappedModel( HASANYFLAG( m_eFlags, FLAGS_SWAPPED ) );

	m_fSwappedTime = 0.0f;
	m_fToggleDelay = ( m_eFlags & FLAGS_SWAPPED ) ? 0.3f : 0.2f;
}
