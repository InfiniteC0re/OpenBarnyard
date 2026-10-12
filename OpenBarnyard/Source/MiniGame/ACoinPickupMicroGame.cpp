#include "pch.h"
#include <Render/TCameraObject.h>
#include "ACoinPickupMicroGame.h"
#include "ALoadScreen.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "FSM/AFunctionPointer.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInventoryState2.h"
#include "GameInterface/ASimState.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/ABikeHelpers.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AHintManager.h"
#include "Helpers/AReplayManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/AShadowManager.h"
#include "Sound/AMusicManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ACoinPickupCoinItem.h"
#include "Units/AInventoryComponentInterface.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "ARandom.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00535470, CreateObject 005362c0, CreateObjectInPlace 00536320
TDEFINE_CLASS_INIT( ACoinPickupMicroGame, 1, 0 );

TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( ABarnExploreMinigameState );
TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( coinspawner );
TPSTRING8_DECLARE( coinspawnerstump );
TPSTRING8_DECLARE( coinspawnerlever );
TPSTRING8_DECLARE( goldrush_token );
TPSTRING8_DECLARE( activate );
TPSTRING8_DECLARE( player_loc );
TPSTRING8_DECLARE( precisioncoin );

// DAT_00783af8
AFunctionTable ACoinPickupMicroGame::ms_oFunctionTable( &TGetClass( ACoinPickupMicroGame ), TNULL );

TFLOAT  ACoinPickupMicroGame::ms_fTimeLimit       = 30.0f;
TINT    ACoinPickupMicroGame::ms_iSplineID        = -1;
TINT    ACoinPickupMicroGame::ms_iNumCoins        = 60;
AUnit** ACoinPickupMicroGame::ms_apStumps         = TNULL;
AUnit** ACoinPickupMicroGame::ms_apLevers         = TNULL;
TINT    ACoinPickupMicroGame::ms_iLeverPlayerBone = -1;

// Locale strings
static constexpr TINT LOCALE_COLLECT_COINS   = 343; // shown when the coins pop out
static constexpr TINT LOCALE_COINS_COLLECTED = 344; // the result, "%d" coins
static constexpr TINT LOCALE_NO_TOKEN        = 345;
static constexpr TINT LOCALE_NO_TOKEN_TITLE  = 342;

namespace
{

// Returns the sim state (or the barn explore minigame, which is an ASimState too) under the micro game
ASimState* FindSimState()
{
	AGameState* pState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) );

	if ( !pState )
		pState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) );

	return TSTATICCAST( ASimState, pState );
}

} // namespace

// $Barnyard: FUNCTION 00536360
void ACoinPickupMicroGame::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AFunctionPointer1Args<TFLOAT>::Create( "SetTimeLimit", &ACoinPickupMicroGame::SetTimeLimit ) );
	ms_oFunctionTable.AddFunction( AFunctionPointer1Args<TINT>::Create( "SetSplineID", &ACoinPickupMicroGame::SetSplineID ) );
	ms_oFunctionTable.AddFunction( AFunctionPointer1Args<TINT>::Create( "SetNumberOfCoins", &ACoinPickupMicroGame::SetNumberOfCoins ) );

	ms_apStumps = TSTATICCAST( AUnit*, TMalloc( sizeof( AUnit* ) * MAX_NUM_COIN_SPAWNERS ) );
	ms_apLevers = TSTATICCAST( AUnit*, TMalloc( sizeof( AUnit* ) * MAX_NUM_COIN_SPAWNERS ) );

	for ( TINT i = 0; i < MAX_NUM_COIN_SPAWNERS; i++ )
	{
		ms_apStumps[ i ] = TNULL;
		ms_apLevers[ i ] = TNULL;
	}
}

// $Barnyard: FUNCTION 00535480
void ACoinPickupMicroGame::DeinitialiseClass()
{
	for ( TINT i = 0; i < MAX_NUM_COIN_SPAWNERS; i++ )
	{
		ms_apStumps[ i ] = TNULL;
		ms_apLevers[ i ] = TNULL;
	}

	TFree( ms_apStumps );
	TFree( ms_apLevers );

	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00535830
ACoinPickupMicroGame::ACoinPickupMicroGame()
    : m_iNumCoins( 0 )
    , m_iNumCoinsLeft( 0 )
    , m_fTimeLeft( 0.0f )
    , m_iScore( 0 )
    , m_eState( STATE_PULLING_LEVER )
    , m_bUnkB54( TFALSE )
    , m_pPlayer( TNULL )
    // NOTE: retail leaves the load state uninitialised, OnInsertion sets it
    , m_eLoadState( LOADSTATE_LOADING )
    // NOTE: uninitialised in retail, only accumulated while the terrain loads
    , m_fLoadTime( 0.0f )
{
	SetInputMap( TPS8( NormalBarnyard ) );
}

// $Barnyard: FUNCTION 00535590
// $Barnyard: FUNCTION 00535910
ACoinPickupMicroGame::~ACoinPickupMicroGame()
{
	m_pPlayer = TNULL;
	m_oPopupListener.Disconnect();
}

// $Barnyard: FUNCTION 00535540
void ACoinPickupMicroGame::SetTimeLimit( TFLOAT a_fTimeLimit )
{
	ms_fTimeLimit = a_fTimeLimit;
}

// $Barnyard: FUNCTION 00535550
void ACoinPickupMicroGame::SetSplineID( TINT a_iSplineID )
{
	ms_iSplineID = a_iSplineID;
	TMath::Clip( ms_iSplineID, 0, NUM_SPLINES - 1 );
}

// $Barnyard: FUNCTION 00535570
void ACoinPickupMicroGame::SetNumberOfCoins( TINT a_iNumCoins )
{
	ms_iNumCoins = a_iNumCoins;
	TMath::Clip( ms_iNumCoins, 1, MAX_NUM_COINS );
}

// $Barnyard: FUNCTION 00535640
AFunctionTable* ACoinPickupMicroGame::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00535930
TBOOL ACoinPickupMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eLoadState != LOADSTATE_RUNNING )
		return TTRUE;

	if ( a_eInputCommand == AInputCommand_Cancel || a_eInputCommand == AInputCommand_Quit )
	{
		AGameStateController::GetSingleton()->StartMenuIfPossible();
		return TTRUE;
	}

	if ( a_eInputCommand == AInputCommand_Leave )
	{
		Remove();
		return TTRUE;
	}

	ACameraHelper* pHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pHelper && pHelper->ProcessInputCommand( (void*)a_eInputCommand, (void*)a_pInputEvent ) )
		return TTRUE;

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00536160
void ACoinPickupMicroGame::OnRender( TFLOAT a_fDeltaTime )
{
	// The world is rendered by the sim under the micro game
	ASimState* pSimState = FindSimState();

	if ( pSimState )
		pSimState->OnRender( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00536b30
TBOOL ACoinPickupMicroGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eLoadState == LOADSTATE_LOADING )
	{
		m_fLoadTime += a_fDeltaTime;

		if ( ATerrainInterface::GetSingleton()->IsLoaded() && m_fLoadTime > 1.0f )
			m_eLoadState = LOADSTATE_LOADED_LOADINGSCREEN;
		else
			g_oLoadScreen.Update( 1.0f, TTRUE );

		return TTRUE;
	}

	if ( m_eLoadState == LOADSTATE_LOADED_LOADINGSCREEN )
	{
		AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );
		g_oLoadScreen.Reset();
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
	}

	if ( m_eLoadState == LOADSTATE_LOADED || m_eLoadState == LOADSTATE_LOADED_LOADINGSCREEN )
	{
		m_eLoadState = LOADSTATE_RUNNING;
		Start();
		return TTRUE;
	}

	// The sim keeps going under the micro game
	ASimState* pSimState = FindSimState();

	if ( pSimState )
	{
		pSimState->UpdateSim( a_fDeltaTime );

		AGameState* pInventoryState = pSimState->FindChildState( &TGetClass( AInventoryState2 ) );

		if ( pInventoryState )
			pInventoryState->OnUpdate( a_fDeltaTime );
	}

	ABikeManager::GetSingleton()->RespawnBikes( a_fDeltaTime, TFALSE );
	m_oText.Update( a_fDeltaTime );

	// NOTE: the original adds 4 to the singleton's address and compares the result to TNULL
	if ( !ABikeManager::GetSingleton()->IsPlayerOnBike() && APlayerProfileManager::GetSingleton() )
	{
		TFLOAT                    fStamina = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetStamina() / 5.0f;
		ASimPlayerMainHudElement* pMainHud = AHudManager::GetSingleton()->FindElement<ASimPlayerMainHudElement>();

		if ( pMainHud )
			pMainHud->SetStamina( fStamina );
	}

	if ( m_eState == STATE_COLLECTING )
		m_fTimeLeft -= a_fDeltaTime;

	// The coins fly to the player once collected
	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] );

		if ( !pCoin->IsCollectDone() )
			pCoin->SetTarget( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	}

	switch ( m_eState )
	{
		case STATE_PULLING_LEVER:
		{
			AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

			if ( !TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->IsAnimPlaying( TPS8( coinspawner ), 0.0f ) )
			{
				ASteer*  pSteer      = pPlayer->GetSteer();
				TVector4 vecPosition = *pSteer->GetPosition();

				PutOnGround( vecPosition, *pSteer->GetDirection(), 0.4f );
				pPlayer->SetSteerPosition( vecPosition );

				m_eState = STATE_START;
			}

			break;
		}
		case STATE_START:
		{
			m_eState    = STATE_COLLECTING;
			m_fTimeLeft = ms_fTimeLimit;

			m_HUDParams.SetFlags( AHudElement::MASK_UNK2 | AHudElement::MASK_UNK15 );

			AHudManager::GetSingleton()->AddElement( &m_oTimeLeftHud, TFALSE, TTRUE, TFALSE );
			m_oTimeLeftHud.SetForceHide( TTRUE );
			m_oTimeLeftHud.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
			m_oTimeLeftHud.Setup( ms_fTimeLimit );
			m_oTimeLeftHud.SetTime( m_fTimeLeft );
			m_oTimeLeftHud.SetScale( 0.855f, 0.727f );

			SpawnCoins();
			ShowText( g_pLocaleManager->GetString( LOCALE_COLLECT_COINS ), 2.0f );

			break;
		}
		case STATE_COLLECTING:
		{
			if ( m_oTimeLeftHud.GetTimer().IsTickComplete() )
			{
				// Out of time
				SetCoinsVisible( TFALSE );
				m_fTimeLeft = 0.0f;
				m_eState    = STATE_RESULT;

				ALocale::FormatTags( m_wszText, TARRAYSIZE( m_wszText ) - 1, LOCALE_COINS_COLLECTED, PPropertyValue( m_iScore ) );
				ShowText( m_wszText, 2.0f );

				break;
			}

			m_oTimeLeftHud.SetForceHide( TFALSE );
			m_oTimeLeftHud.SetTime( m_fTimeLeft );

			TVector4 vecPlayerPosition = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();
			TFLOAT   fRadius           = ABikeManager::GetSingleton()->IsPlayerOnBike() == TTRUE ? 4.0f : 2.5f;

			// Collect the coins the player is close to
			for ( TINT i = 0; i < m_iNumCoins; i++ )
			{
				ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] );

				if ( !pCoin->IsCollected() && TVector4::Distance( vecPlayerPosition, *pCoin->GetSteer()->GetPosition() ) < fRadius )
				{
					pCoin->SetCollected( TTRUE );
					pCoin->SetFlashing( TFALSE );
					m_iNumCoinsLeft--;
				}
			}

			// The coins that reached the player go to the inventory
			for ( TINT i = 0; i < m_iNumCoins; i++ )
			{
				ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] );

				if ( pCoin->IsPendingCollect() == TTRUE && pCoin->IsCollectDone() == TTRUE )
				{
					m_iScore += pCoin->GetValue();
					AInventoryGroupComponent::CollectUnit( pCoin, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );
					pCoin->SetPendingCollect( TFALSE );
				}
			}

			if ( m_iNumCoinsLeft <= 0 )
			{
				// Wait for the coins that are still flying to the player
				TINT iNumPending = 0;

				for ( TINT i = 0; i < m_iNumCoins; i++ )
				{
					ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] );

					if ( pCoin->IsPendingCollect() == TTRUE || pCoin->IsVisible() == TTRUE )
						iNumPending++;
				}

				if ( iNumPending <= 0 )
				{
					// Every coin is collected
					SetCoinsVisible( TFALSE );
					m_fTimeLeft = 0.0f;
					m_oTimeLeftHud.SetTime( 0.0f );
					m_eState = STATE_RESULT;

					ALocale::FormatTags( m_wszText, TARRAYSIZE( m_wszText ) - 1, LOCALE_COINS_COLLECTED, PPropertyValue( m_iScore ) );
					ShowText( m_wszText, 2.0f );
				}
			}

			break;
		}
		case STATE_RESULT:
			// Wait for the text to fade out
			if ( m_oText.GetAlpha() == 0.0f )
				m_eState = STATE_EXIT;

			break;
		case STATE_EXIT:
			Remove();
			return TTRUE;
		case STATE_NO_TOKEN:
			m_eState = STATE_EXIT;
			ShowMessage( g_pLocaleManager->GetString( LOCALE_NO_TOKEN ) );
			break;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005359c0
void ACoinPickupMicroGame::OnInsertion()
{
	if ( ATerrainInterface::GetSingleton()->IsLoaded() )
	{
		m_eLoadState = LOADSTATE_LOADED;
	}
	else
	{
		g_oLoadScreen.Reset();
		g_oLoadScreen.StartLoading( 0, TTRUE );
		m_eLoadState = LOADSTATE_LOADING;
		AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
	}

	// NOTE: the original calls the virtual directly
	AHintManager::GetSingleton()->OnDeactivate();

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00535a40
void ACoinPickupMicroGame::OnRemoval()
{
	for ( TINT i = 0; i < m_iNumCoins; i++ )
		TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] )->Destroy();

	for ( TINT i = 0; i < m_iNumCoins; i++ )
		m_apCoins[ i ] = TNULL;

	m_iNumCoins = 0;

	m_oTimeLeftHud.Detach();
	AGameTimeManager::GetSingleton()->PopUpdateState();

	AReplayManager::GetSingleton()->SetReplaysDisabled( TFALSE );
	AReplayManager::GetSingleton()->UpdateReplays();

	if ( m_pPlayer )
	{
		APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponentExact( &TGetClass( APlayerControlComponent ) ) );

		if ( pControl )
		{
			pControl->SetInventoryDisabled( TFALSE );
			pControl->SetUnitInteractions( TTRUE );
		}
	}

	m_oSplineTRB.Close();

	// NOTE: the original calls the virtual directly
	AHintManager::GetSingleton()->OnActivate();

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00535b20
void ACoinPickupMicroGame::OnActivate()
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();

	if ( !pCameraManager->GetCurrentCameraHelper() )
	{
		if ( ABikeManager::GetSingleton()->IsPlayerOnBike() )
			pCameraManager->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
		else
			pCameraManager->SetSplitscreenCameraHelper( 1 );
	}

	if ( m_pPlayer )
	{
		APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponentExact( &TGetClass( APlayerControlComponent ) ) );

		if ( pControl )
		{
			pControl->SetInventoryDisabled( TTRUE );
			pControl->SetUnitInteractions( TFALSE );
		}
	}

	ASimState* pSimState = FindSimState();

	if ( pSimState )
		pSimState->SetActive( TTRUE );

	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00535bc0
void ACoinPickupMicroGame::OnDeactivate()
{
	if ( m_pPlayer )
	{
		APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponentExact( &TGetClass( APlayerControlComponent ) ) );

		if ( pControl )
		{
			pControl->SetInventoryDisabled( TFALSE );
			pControl->SetUnitInteractions( TTRUE );
		}
	}

	ASimState* pSimState = FindSimState();

	if ( pSimState )
		pSimState->SetActive( TFALSE );

	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 005369b0
void ACoinPickupMicroGame::Start()
{
	m_oSplineTRB.Load( "Data\\treasure\\coinspline4.trb", 0 );
	m_aSplines[ 0 ].Load( m_oSplineTRB, "cointrail1" );
	m_aSplines[ 1 ].Load( m_oSplineTRB, "cointrail2" );
	m_aSplines[ 2 ].Load( m_oSplineTRB, "cointrail3" );
	m_aSplines[ 3 ].Load( m_oSplineTRB, "cointrail4" );
	m_aSplines[ 4 ].Load( m_oSplineTRB, "cointrail5" );
	m_aSplines[ 5 ].Load( m_oSplineTRB, "cointrail6" );
	m_aSplines[ 6 ].Load( m_oSplineTRB, "cointrail7" );
	m_aSplines[ 7 ].Load( m_oSplineTRB, "cointrail_barn" );
	m_oSplineTRB.DeleteSymbolTable();

	AGameTimeManager::GetSingleton()->PushUpdateState();

	AReplayManager::GetSingleton()->SetReplaysDisabled( TTRUE );
	AReplayManager::GetSingleton()->UpdateReplays();

	AUnit*                        pPlayer    = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pPlayer );

	if ( !pInventory || pInventory->GetNumItemsOfType( TPS8( goldrush_token ) ) != 0 )
	{
		m_eState        = STATE_PULLING_LEVER;
		m_iScore        = 0;
		m_iNumCoinsLeft = 0;
		m_bUnkB54       = TFALSE;

		// Pulling the lever costs a token
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail calls this on TNULL if the player has no inventory
		if ( pInventory )
#endif
			pInventory->RemoveItems( TPS8( goldrush_token ), 1, TTRUE );

		m_pPlayer = pPlayer;

		APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponentExact( &TGetClass( APlayerControlComponent ) ) );

		if ( pControl )
		{
			pControl->SetInventoryDisabled( TTRUE );
			pControl->SetUnitInteractions( TFALSE );
		}

		PlacePlayerAtLever( pPlayer );
		ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ResetPosition( TTRUE );
	}
	else
	{
		m_eState = STATE_NO_TOKEN;
	}
}

// $Barnyard: FUNCTION 00535c20
void ACoinPickupMicroGame::SpawnCoins()
{
	TINT iSplineID = ms_iSplineID;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail reads m_aSplines[ -1 ] when no script called SetSplineID
	if ( iSplineID < 0 )
		iSplineID = 0;
#endif

	const ASpline& rSpline   = m_aSplines[ iSplineID ];
	TINT           iNumCoins = TMath::Min( ms_iNumCoins, MAX_NUM_COINS );

	// The coins are spread evenly along the trail, half a metre above it
	TVector4 avecPositions[ MAX_NUM_COINS ];

	for ( TINT i = 0; i < iNumCoins; i++ )
	{
		rSpline.GetPosition( TFLOAT( i ) / TFLOAT( iNumCoins ), avecPositions[ i ] );

		TVector4 vecOffset;
		vecOffset.Multiply( TVector4::VEC_NEGY, 0.5f );
		avecPositions[ i ].x += vecOffset.x;
		avecPositions[ i ].y += vecOffset.y;
		avecPositions[ i ].z += vecOffset.z;
	}

	AShadowManager* pShadowManager = AShadowManager::GetSingleton();

	for ( TINT i = 0; i < iNumCoins; i++ )
	{
		// Random rotation
		TVector3 vecDirection( TVector4::VEC_POSZ.x, TVector4::VEC_POSZ.y, TVector4::VEC_POSZ.z );
		vecDirection.RotateY( TFLOAT( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * 4.656613e-10f * TMath::TWO_PI );

		TVector4 vecDirection4( vecDirection.x, vecDirection.y, vecDirection.z, TVector4::VEC_POSZ.w );

		AUnitTransform oTransform( avecPositions[ i ], vecDirection4, 0, -1, -1, TTRUE );
		oTransform.m_iModelIndex = 3;

		ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, AUnitManager::GetSingleton()->SpawnUnit( TPS8( precisioncoin ), &oTransform ) );
		pCoin->ChangeSteer( &TGetClass( AThrowItemSteer ) );
		pCoin->SetValue( 1 );
		pCoin->SetMovement( TTRUE );
		pCoin->SetFlashing( TFALSE );
		pCoin->SetUseTrackEffect( TTRUE );
		pCoin->SetTarget( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
		pCoin->SetStartPosition( avecPositions[ i ] );
		pShadowManager->SetShadowStatic( pCoin, TTRUE );

		m_apCoins[ m_iNumCoins++ ] = pCoin;
		m_iNumCoinsLeft++;
	}
}

// $Barnyard: FUNCTION 00535660
void ACoinPickupMicroGame::ShowText( const TWCHAR* a_wszText, TFLOAT a_fTime )
{
	m_oText.m_fUnk4     = 1.0f;
	m_oText.m_fUnk3     = 1.0f;
	m_oText.m_fUnk2     = 1.2f;
	m_oText.m_fFadeTime = a_fTime;

	// NOTE: retail writes the integer 2 to this float
	*TREINTERPRETCAST( TINT*, &m_oText.m_fUnk1 ) = 2;

	// NOTE: retail calls DOES_NOTHING (00628f40) on the text with ( 0.5f, 0.5f ) here

	m_oText.SetText( a_wszText, 0x7755ff55 );
}

// $Barnyard: FUNCTION 00535e90
void ACoinPickupMicroGame::SetCoinsVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		if ( m_apCoins[ i ] )
			TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] )->SetVisible( a_bVisible );
	}
}

// $Barnyard: FUNCTION 005368a0
void ACoinPickupMicroGame::ShowMessage( const TWCHAR* a_wszMessage )
{
	TUINT32 uiHUDMask = AGameStateController::GetSingleton()->GetCurrentState()->GetHUDMask();

	AMessagePopupState* pPopup = new AMessagePopupState(
	    g_pLocaleManager->GetString( LOCALE_NO_TOKEN_TITLE ),
	    a_wszMessage,
	    TPString8(),
	    1,
	    TFALSE,
	    AInputCommand_OK,
	    AInputCommand_Empty,
	    AInputCommand_Empty,
	    AInputCommand_Empty,
	    TFALSE,
	    60.0f,
	    uiHUDMask
	);

	m_oPopupListener.Connect( pPopup->GetResponseEmitter(), this, OnMessageResponse, 0 );
	AGameStateController::GetSingleton()->PushState( pPopup );
}

// $Barnyard: FUNCTION 00535650
TBOOL __stdcall ACoinPickupMicroGame::OnMessageResponse( ACoinPickupMicroGame* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	a_pPopup->Remove();
	return TFALSE;
}

// $Barnyard: FUNCTION 005356d0
void ACoinPickupMicroGame::FindClosestCoinSpawner( const TVector4& a_rcPosition, AUnit*& a_rpStump, AUnit*& a_rpLever )
{
	a_rpStump = TNULL;
	a_rpLever = TNULL;

	TFLOAT fClosestDistanceSq = TMath::MAXFLOAT;

	for ( TINT i = 0; i < MAX_NUM_COIN_SPAWNERS; i++ )
	{
		AUnit* pStump = ms_apStumps[ i ];
		AUnit* pLever = ms_apLevers[ i ];

		if ( pStump )
		{
			TFLOAT fDistanceSq = TVector4::DistanceSq( *pStump->GetSteer()->GetPosition(), a_rcPosition );

			if ( fDistanceSq < fClosestDistanceSq )
			{
				fClosestDistanceSq = fDistanceSq;
				a_rpStump          = pStump;
				a_rpLever          = pLever;
			}
		}
	}
}

// $Barnyard: FUNCTION 00536690
void ACoinPickupMicroGame::PlacePlayerAtLever( AUnit* a_pPlayer )
{
	ASteer* pSteer = a_pPlayer->GetSteer();

	AUnit* pStump;
	AUnit* pLever;
	FindClosestCoinSpawner( *pSteer->GetPosition(), pStump, pLever );

	if ( !pLever )
		return;

	// The lever gets pulled
	AAnimatableObject* pLeverObject = TSTATICCAST( AAnimatedUnit, pLever )->GetAnimatableObject();
	pLeverObject->PlayAnimation( TPS8( activate ) );
	pLeverObject->Update( 0.0f );

	TMatrix44 matPlayerLoc;
	pLeverObject->GetModelInstance()->GetBoneTransform( ms_iLeverPlayerBone, matPlayerLoc );

	// The player pulls it
	AAnimatableObject* pPlayerObject = TSTATICCAST( AAnimatedUnit, a_pPlayer )->GetAnimatableObject();
	pPlayerObject->PlayAnimation( TPS8( coinspawner ) );
	pPlayerObject->Update( 0.0f );

	// Stand at the "player_loc" bone of the lever, on the ground if it's lower
	TVector4 vecPosition = matPlayerLoc.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION );

	TVector4 vecOffset;
	vecOffset.Multiply( TVector4::VEC_NEGY, 0.3f );

	TVector4 vecAbove( vecPosition.x + vecOffset.x, vecPosition.y + vecOffset.y, vecPosition.z + vecOffset.z, vecPosition.w );
	TFLOAT   fGroundHeight = ATerrainInterface::GetSingleton()->GetGroundHeight( vecAbove, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

	if ( fGroundHeight != 0.0f && fGroundHeight < vecPosition.y )
		vecPosition.y = fGroundHeight;

	pSteer->SetPosition( vecPosition );
	pSteer->SetDirection( *pStump->GetSteer()->GetDirection() );
}

// $Barnyard: FUNCTION 00535ee0
void ACoinPickupMicroGame::PutOnGround( TVector4& a_rPosition, const TVector4& a_rcDirection, TFLOAT a_fRadius )
{
	TVector4 vecSide;
	vecSide.CrossProduct( TVector4::VEC_NEGY, a_rcDirection );
	vecSide.Normalise();

	TFLOAT fLowestY = a_rPosition.y;

	// NOTE: ABikeHelpers::SnapToGround sets y to 0 if there's no ground, it's taken anyway
	auto CheckGround = [ & ]( const TVector4& a_rcOffset, TFLOAT a_fSign ) {
		TVector4 vecPoint( a_rPosition.x + a_rcOffset.x * a_fSign, a_rPosition.y + a_rcOffset.y * a_fSign, a_rPosition.z + a_rcOffset.z * a_fSign, a_rPosition.w );
		ABikeHelpers::SnapToGround( vecPoint );

		if ( vecPoint.y < fLowestY )
			fLowestY = vecPoint.y;
	};

	CheckGround( TVector4::VEC_ZERO, 1.0f );

	TVector4 vecForward;
	vecForward.Multiply( a_rcDirection, a_fRadius );
	CheckGround( vecForward, -1.0f );

	TVector4 vecRight;
	vecRight.Multiply( vecSide, a_fRadius );
	CheckGround( vecRight, 1.0f );
	CheckGround( vecRight, -1.0f );

	a_rPosition.y = fLowestY;
}

// $Barnyard: FUNCTION 00536470
void ACoinPickupMicroGame::SpawnCoinSpawners()
{
	AInteriorNetwork* pNetwork     = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	TINT              iNumSpawners = pNetwork->CountGoalsContaining( "stumppos" );

	TCHAR szGoalName[ 64 ];
	T2String8::Copy( szGoalName, "stumppos" );
	TCHAR* pNumber = szGoalName + T2String8::Length( szGoalName );

	AShadowManager* pShadowManager = AShadowManager::GetSingleton();

	for ( TINT i = 1; i < iNumSpawners + 1; i++ )
	{
		T2String8::Format( pNumber, "%02d", i );

		AInteriorGoal* pGoal = pNetwork->FindGoalOrFirst( szGoalName );

		// NOTE: the result isn't used
		TVector4 vecGoalPosition;
		pGoal->GetPosition( vecGoalPosition );

		TVector4 vecDirection = pGoal->m_vForward;

		if ( vecDirection.MagnitudeSq() < 0.01f )
			vecDirection = TVector4::VEC_POSX;

		AUnit* pStump = AUnitManager::GetSingleton()->SpawnUnitAtGoal( TPS8( coinspawnerstump ), TVector4::VEC_ZERO, pNetwork, pGoal, TNULL );
		AUnit* pLever = AUnitManager::GetSingleton()->SpawnUnitAtGoal( TPS8( coinspawnerlever ), TVector4::VEC_ZERO, pNetwork, pGoal, TNULL );
		pStump->GetSteer()->SetDirection( vecDirection );
		pLever->GetSteer()->SetDirection( vecDirection );

		pShadowManager->RemoveShadow( pStump );
		pShadowManager->RemoveShadow( pLever );

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail writes past the arrays if the map has more than 8 goals
		if ( i > MAX_NUM_COIN_SPAWNERS )
			continue;
#endif

		ms_apStumps[ i - 1 ] = pStump;
		ms_apLevers[ i - 1 ] = pLever;
	}

	if ( iNumSpawners > 0 && ms_apLevers[ 0 ] )
	{
		AModelInstanceRef pModelInstance = TSTATICCAST( AAnimatedUnit, ms_apLevers[ 0 ] )->GetAnimatableObject()->GetModelInstance();
		ms_iLeverPlayerBone              = pModelInstance->GetBoneID( TPS8( player_loc ) );
	}
}

// $Barnyard: FUNCTION 005357d0
void ACoinPickupMicroGame::DestroyCoinSpawners()
{
	AUnitManager* pUnitManager = AUnitManager::GetSingleton();

	for ( TINT i = 0; i < MAX_NUM_COIN_SPAWNERS; i++ )
	{
		if ( ms_apStumps[ i ] )
		{
			pUnitManager->DestroyUnit( ms_apStumps[ i ] );
			pUnitManager->DestroyUnit( ms_apLevers[ i ] );
		}

		ms_apStumps[ i ] = TNULL;
		ms_apLevers[ i ] = TNULL;
	}
}
