#include "pch.h"
#include "AGateCrashMinigameState.h"
#include "Sound/APositionalSoundSet.h"
#include "Helpers/AHighScoreManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "AGateCrashPlayerAi.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "Units/AUnitManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitTransform.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimationBreakpointCallback.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/ALerpCameraHelper.h"
#include "Terrain/ATerrainInterface.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/APlayerProgress.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Player/APlayerProfileManager.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AGameTimeFXManager.h"
#include "GUI/AFadeManager.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Units/ASSeqController.h"
#include "Locale/ALocaleManager.h"

#include <File/TTRB.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TString8.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_LATEST
// Base of the maximum gate speed (latest version, 806af0dc), set by OnInsertion from the difficulty
static TFLOAT s_fMaxGateSpeedBase = AGateCrashMinigameState::GATE_MAX_SPEED_BASE_NORMAL;
#endif

TPSTRING8_DECLARE( AGateCrashMinigameState );
TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( cb_llocator );
TPSTRING8_DECLARE( end );
TPSTRING8_DECLARE( gcpipe );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( impact );
TPSTRING8_DECLARE( jump_apex );
TPSTRING8_DECLARE( jump_down );
TPSTRING8_DECLARE( jump_end );
TPSTRING8_DECLARE( jump_liftoff );
TPSTRING8_DECLARE( jump_start );
TPSTRING8_DECLARE( jump_up );
TPSTRING8_DECLARE( look_l );
TPSTRING8_DECLARE( look_r );
TPSTRING8_DECLARE( ms_contact );
TPSTRING8_DECLARE( ms_gen_cow_anims );
TPSTRING8_DECLARE( ms_gen_goat_anims );
TPSTRING8_DECLARE( ms_gen_horse );
TPSTRING8_DECLARE( ms_impact_l );
TPSTRING8_DECLARE( ms_impact_r );
TPSTRING8_DECLARE( ms_kick_l );
TPSTRING8_DECLARE( ms_kick_r );
TPSTRING8_DECLARE( ms_serve );
TPSTRING8_DECLARE( ms_serve_idle );
TPSTRING8_DECLARE( player_mudjump );
TPSTRING8_DECLARE( victory );

// NOTE: GetClass 0046cd10, CreateObject 00470670, CreateObjectInPlace 004706d0
TDEFINE_CLASS( AGateCrashMinigameState );

#ifdef BARNYARD_COMMUNITY_PATCH
static TINT  s_iDevNumHumans = 0;
static TBOOL s_bDevAiReady   = TFALSE;
#endif

// $Barnyard: FUNCTION 00470510
AGateCrashMinigameState::AGateCrashMinigameState()
    : AMiniGameState( new AGateCrashPlayerAi() )
{
	m_oImpactListeners.pCaller    = this;
	m_oImpactListeners.fnCallback = TREINTERPRETCAST( TGenericListener::t_Callback, OnImpactAnimEventCallback );

	m_iNumPipes       = 0;
	m_pDripEffect     = TNULL;
	m_pMudDripEffect  = TNULL;
	m_iCreakCue       = -1;
	m_iCue2           = -1;
	m_iMudAmbienceCue = -1;

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( AGateCrashMinigameState ) );
}

// $Barnyard: FUNCTION 00470650
// $Barnyard: FUNCTION 0046e890
AGateCrashMinigameState::~AGateCrashMinigameState()
{
}

// $Barnyard: FUNCTION 0046ce40
TBOOL AGateCrashMinigameState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	TBOOL bResult = TTRUE;
	TINT  iSlot   = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() );

	PlayerSlot& rSlot = m_oGame.aSlots[ iSlot ];

	switch ( a_eInputCommand )
	{
		case AInputCommand_Left:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				rSlot.bKickPressed     = TTRUE;
				rSlot.bDirectionalKick = TTRUE;
				rSlot.bKickLeft        = TTRUE;
				return TTRUE;
			}
			break;
		case AInputCommand_Right:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				rSlot.bKickPressed     = TTRUE;
				rSlot.bDirectionalKick = TTRUE;
				rSlot.bKickLeft        = TFALSE;
				return TTRUE;
			}
			break;
		case AInputCommand_Shoot:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				rSlot.bKickPressed     = TTRUE;
				rSlot.bDirectionalKick = TFALSE;
				return TTRUE;
			}
			break;
		case AInputCommand_Jump:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				rSlot.bJumpPressed = TTRUE;
				return TTRUE;
			}

			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				rSlot.bJumpHeld = TFALSE;
				return TTRUE;
			}
			break;
		default:
			bResult = AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
			break;
	}

	return bResult;
}

// $Barnyard: FUNCTION 0046cdd0
ASoundId AGateCrashMinigameState::GetSound()
{
	return 0x15;
}

// Begins the game once the intro has finished: shows the players and lets the
// serving one move.
// $Barnyard: FUNCTION 0046d980
void AGateCrashMinigameState::OnPause()
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
		TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ i ].GetUnit() )->SetVisible( TTRUE );

	for ( TINT i = 0; i < NUM_SPECTATORS; i++ )
		m_apSpectators[ i ]->SetSkeletonUpdating( TTRUE, TTRUE );

	AAnimatedUnit* pServingUnit = TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ m_oGame.iServingPlayer ].GetUnit() );
	pServingUnit->SetVisible( TTRUE );
	pServingUnit->SetMovement( TTRUE );
	pServingUnit->SetAIEnabled( TTRUE );
	pServingUnit->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	if ( m_bCharityVariant )
		SetCharityActorsSkeletonUpdating( TTRUE );

	// NOTE: retail StartFade (0045cb20) has a 4th argument, TTRUE here, that our signature doesn't have yet
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 0.5f );

	// DOES_NOTHING (005acd80) is called on this
}

// $Barnyard: FUNCTION 0046e960
void AGateCrashMinigameState::OnInsertion()
{
#ifdef BARNYARD_LATEST
	// Latest version (Wii 8018c5c0): the maximum gate speed of the difficulty
	s_fMaxGateSpeedBase = APlayerProgress::GetSingleton()->IsEasyDifficulty() ? GATE_MAX_SPEED_BASE_EASY : GATE_MAX_SPEED_BASE_NORMAL;
#endif

	m_bCharityVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant() == 1;
	ACameraManager::GetSingleton()->DetachCameraHelpers();
	AGameLoader::Terrain_Set( AGameLoader::Terrain_GateCrash, TFALSE, TTRUE, 0, 0, 0, 0 );

	m_fPreStartTimer = 0.0f;
	m_fGameOverTimer = 0.0f;

	// Fill up the empty slots with computer players
	m_iNumPlayers = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount();
#ifdef BARNYARD_COMMUNITY_PATCH
	// The human players, the slots the dev AI plays (UpdateGame)
	s_iDevNumHumans = m_iNumPlayers;
	s_bDevAiReady   = TFALSE;
#endif

	for ( TINT i = MAX_PLAYERS - APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetNumAIPlayers() - m_iNumPlayers; i > 0; i-- )
		APlayerManager::GetSingleton()->GetCurrentPlayerSet()->AddAIPlayer();

	APlayerManager::GetSingleton()->GetCurrentPlayerSet()->FinishUpAddingAIs();

	TINT                iNumAIs   = MAX_PLAYERS - m_iNumPlayers;
	AGateCrashPlayerAi* pPlayerAi = TSTATICCAST( AGateCrashPlayerAi, m_pPlayerAi );
	pPlayerAi->SetGameData( &m_oGame );
	pPlayerAi->Init( m_iNumPlayers, iNumAIs );

	TTRB oMudData;
	if ( oMudData.Load( "Data\\MinigameData\\MudJumpers\\muddata.trb", 0 ) == TTRB::ERROR_OK )
	{
		TFLOAT** ppAIData = oMudData.CastSymbol<TFLOAT*>( "AIData" );

		for ( TINT i = 0; i < iNumAIs; i++ )
			pPlayerAi->SetParams( i, ppAIData[ i ][ 0 ], ppAIData[ i ][ 1 ], ppAIData[ i ][ 2 ], ppAIData[ i ][ 3 ] );

		TINT* pScores = oMudData.CastSymbol<TINT>( "Scores" );
		for ( TINT i = 0; i < SCORE_NUMOF; i++ )
			m_aScores[ i ] = pScores[ i ];

		TINT* pLives = oMudData.CastSymbol<TINT>( "Lives" );
		for ( TINT i = 0; i < MAX_PLAYERS; i++ )
			m_oGame.aSlots[ i ].iLives = pLives[ i ];

		oMudData.Close();
	}

	m_iNumDodges  = 0;
	m_iNumHits    = 0;
	m_iNumKOs     = 0;
	m_iNumPlayers = m_iNumPlayers + iNumAIs;
	m_iNumAlive   = m_iNumPlayers;
	m_vPipePos    = ATerrainInterface::GetSingleton()->GetLocatorPos( "CB_pipe" );

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		PlayerSlot& rSlot = m_oGame.aSlots[ i ];

		APlayerSimProfile* pProfile = TNULL;
		TINT               iCharacter;

		if ( i == 0 )
		{
			pProfile   = APlayerProfileManager::GetSingleton()->GetMainProfile();
			iCharacter = pProfile->GetGender();

			if ( iCharacter == APlayerProfile::GENDER_NONE )
				iCharacter = APlayerProfile::GENDER_MALE;
		}
		else
		{
			iCharacter = i + 1;
		}

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, iCharacter, i, TTRUE );
		AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_mudjump ), &oTransform ) );

		if ( i == 0 )
			pUnit->ApplyMaterial( pProfile->GetCostume(), TTRUE );

		rSlot.pUnitProxy = pUnit->GetSelfProxy();

		TString8 strLocator;
		strLocator.Format( "CB_player%d", i + 1 );

		TQuaternion quatLocator;
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( strLocator, rSlot.vPosition, quatLocator );
		pUnit->GetSteer()->SetPosition( rSlot.vPosition );

		// NOTE: retail hands the locator rotation to SetDirection as is; the direction is set again below
		pUnit->GetSteer()->SetDirection( *TREINTERPRETCAST( TVector4*, &quatLocator ) );

		// Angle of the slot around the pipe
		TVector4 vFromPipe( rSlot.vPosition.x - m_vPipePos.x, 0.0f, rSlot.vPosition.z - m_vPipePos.z, rSlot.vPosition.w );
		vFromPipe.Normalise();

		rSlot.fAngle = vFromPipe.GetAngleBetweenVector( ACamera::sm_vInitialLookDirection );
		vFromPipe.AsVector3().RotateY( TMath::HALF_PI );

		if ( TVector4::DotProduct3( vFromPipe, ACamera::sm_vInitialLookDirection ) < 0.0f )
			rSlot.fAngle *= -1.0f;

		rSlot.fKickTimer    = 0.0f;
		rSlot.fJumpVelocity = 0.0f;
		rSlot.eState        = PLAYERSTATE_READY;
		rSlot.bJumpPressed  = TFALSE;
		rSlot.bKickPressed  = TFALSE;
		rSlot.bDodging      = TFALSE;

		// Face the pipe
		TVector4 vToPipe( m_vPipePos.x - rSlot.vPosition.x, 0.0f, m_vPipePos.z - rSlot.vPosition.z, m_vPipePos.w );
		vToPipe.Normalise();
		pUnit->GetSteer()->SetDirection( vToPipe );

		// Replace the default shadow with one that follows the C bone
		AShadowManager::GetSingleton()->RemoveShadow( pUnit );

		AModelInstanceRef pModelInstance = pUnit->GetAnimatableObject()->GetModelInstance();
		TINT              iShadowBone    = pModelInstance->GetSkeletonInstance()->GetBoneID( "C" );
		AShadowManager::GetSingleton()->AddUnitShadow( pUnit, 1.25f, iShadowBone, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );

		T2GUIATTACHMENT eAttachment;
		switch ( i )
		{
			case 0:
				eAttachment = T2GUIATTACHMENT_BOTTOMLEFT;
				break;
			case 1:
				eAttachment = T2GUIATTACHMENT_TOPRIGHT;
				break;
			case 3:
				eAttachment = T2GUIATTACHMENT_BOTTOMRIGHT;
				break;
			default:
				eAttachment = T2GUIATTACHMENT_TOPLEFT;
				break;
		}

		m_aLifeHuds[ i ].Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", eAttachment, eAttachment, 1.0f, TFALSE );

		m_aLifeHuds[ i ].SetVisible( TFALSE );

		m_aLifeHuds[ i ].SetLives( rSlot.iLives );
	}

	if ( m_bCharityVariant == TTRUE )
	{
		// Everet and Daisy watch the first player
		CreateCharityActors( TFALSE );

		TVector4 vEveretPos( 21.5f, -1.8f, 23.5f, 1.0f );
		vEveretPos.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vEveretPos, 0 );

		TVector4 vEveretDir( m_oGame.aSlots[ 0 ].vPosition.x - vEveretPos.x, 0.0f, m_oGame.aSlots[ 0 ].vPosition.z - vEveretPos.z, m_oGame.aSlots[ 0 ].vPosition.w );
		vEveretDir.Normalise();

		TVector4 vDaisyPos( 20.0f, -1.8f, 23.5f, 1.0f );
		vDaisyPos.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vDaisyPos, 0 );

		TVector4 vDaisyDir( m_oGame.aSlots[ 0 ].vPosition.x - vDaisyPos.x, 0.0f, m_oGame.aSlots[ 0 ].vPosition.z - vDaisyPos.z, m_oGame.aSlots[ 0 ].vPosition.w );
		vDaisyDir.Normalise();

		PlaceCharityActors( vEveretPos, vDaisyPos, vEveretDir, vDaisyDir );
		SetCharityActorsSkeletonUpdating( TTRUE );
	}

	// Listen to the impact events of the first player's animations (rumble)
	AAnimatableObject* pFirstPlayerObject = TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ 0 ].GetUnit() )->GetAnimatableObject();
	const TPString8*   apImpactAnims[]    = { &TPS8( ms_impact_l ), &TPS8( ms_impact_r ) };

	for ( TINT i = 0; i < TARRAYSIZE( apImpactAnims ); i++ )
	{
		AAnimationBreakpointCallback* pBreakpoint = pFirstPlayerObject->GetAnimationEmitter( *apImpactAnims[ i ] );
		ImpactListener*               pListener   = new ImpactListener;

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail doesn't check if the animation has the breakpoint
		if ( pBreakpoint )
#endif
		{
			pListener->pEmitter = &pBreakpoint->GetEmitter();
			pListener->oListener.Connect( pBreakpoint->GetEmitter(), m_oImpactListeners.pCaller, m_oImpactListeners.fnCallback, 0 );
		}

		m_oImpactListeners.oList.InsertTail( pListener );
	}

	m_oGame.iServingPlayer = 0;
	m_oGame.fGateAngle     = m_oGame.aSlots[ 0 ].fAngle;
	m_oGame.fGateSpeed     = 0.0f;
	m_iLastKicker          = 0;
	BuildGatePipe( 0 );

	if ( m_bCharityVariant )
	{
		AAnimatableObjectType* pBuckType = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( barnbuck ) );

		for ( TINT i = 0; i < NUM_CROWD; i++ )
		{
			m_apCrowd[ i ] = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pBuckType, TNULL, 0 );
			m_apCrowd[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );
			m_afCrowdTimers[ i ] = 0.0f;
		}

		for ( TINT i = 0; i < NUM_POSTS; i++ )
		{
			m_apPosts[ i ] = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pBuckType, TNULL, 0 );
			m_apPosts[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );
		}

		m_fPostTimer  = 0.0f;
		m_iCrowdCount = 0;
		m_oPickUpHudElement.m_oBacking.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	}

	// Spectators (00773820 and 0077382c)
	const TPString8* apSpectatorTypes[ NUM_SPECTATORS ]    = { &TPS8( ms_gen_cow_anims ), &TPS8( ms_gen_goat_anims ), &TPS8( ms_gen_horse ) };
	const TCHAR*     apSpectatorLocators[ NUM_SPECTATORS ] = { "CB_genCow_orient", "CB_genGoat_orient", "CB_genHorse_orient" };

	for ( TINT i = 0; i < NUM_SPECTATORS; i++ )
	{
		AAnimatableObjectType* pType = AAnimatableObjectManager::GetSingleton()->FindType( *apSpectatorTypes[ i ] );
		m_apSpectators[ i ]          = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pType, TNULL, 0 );

		TVector4    vPosition;
		TQuaternion quatRotation;
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( apSpectatorLocators[ i ], vPosition, quatRotation );

		// NOTE: retail hands the locator rotation to AModelInstance::SetTransform as the direction
		m_apSpectators[ i ]->GetModelInstance()->SetTransform( vPosition, *TREINTERPRETCAST( TVector4*, &quatRotation ) );
		m_apSpectators[ i ]->SetSkeletonUpdating( TTRUE, TTRUE );
		m_apSpectators[ i ]->PlayAnimation( TPS8( idle ) );
	}

	// DOES_NOTHING (0054df00) is called with AInputHandler + 0x4c and &m_oGame

	m_oScorePopup.Create( &m_GUIElement, "rekord26" );

	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "Gatecrasher" ) );

	AMiniGameState::OnInsertion();
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME | AHudElement::MASK_UNK26 );

	ASSeqController::GetSingleton()->LoadScript( "SSMJ_Main.tso", TVector4::VEC_ZERO );

	m_bIntroPending = TTRUE;
}

// $Barnyard: FUNCTION 0046cf80
void AGateCrashMinigameState::OnRemoval()
{
	AInputHandler::GetSingleton()->StopAllRumbles();
	AInputHandler::GetSingleton()->StopRumble( AInputHandler::GetSingleton()->GetMainController() );
	m_oServeListener.Disconnect();

	APositionalSoundSet::GetSingleton()->Unload();

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	m_oScorePopup.Destroy();

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < NUM_SPECTATORS; i++ )
		pAnimObjManager->DestroyAnimatableObject( m_apSpectators[ i ] );

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		AUnit* pUnit = m_oGame.aSlots[ i ].GetUnit();

		if ( pUnit )
			AUnitManager::GetSingleton()->DestroyUnit( pUnit );
	}

	while ( m_iNumPipes != 0 )
	{
		AAnimatableObject* pPipe   = m_apPipes[ m_iNumPipes - 1 ];
		m_apPipes[ --m_iNumPipes ] = TNULL;
		pAnimObjManager->DestroyAnimatableObject( pPipe );
	}

	if ( m_bCharityVariant )
	{
		for ( TINT i = 0; i < NUM_CROWD; i++ )
			pAnimObjManager->DestroyAnimatableObject( m_apCrowd[ i ] );

		for ( TINT i = 0; i < NUM_POSTS; i++ )
			pAnimObjManager->DestroyAnimatableObject( m_apPosts[ i ] );

		DestroyCharityActors();
	}

	if ( m_pDripEffect )
	{
		m_pDripEffect->GetInstance()->Kill();
		m_pDripEffect = TNULL;
	}

	if ( m_pMudDripEffect )
	{
		m_pMudDripEffect->GetInstance()->Kill();
		m_pMudDripEffect = TNULL;
	}

	ASoundManager::GetSingleton()->StopCue( m_iMudAmbienceCue );

	AUnitManager::GetSingleton()->DeleteDyingUnits();

	TRenderInterface::GetSingleton()->FlushDyingResources();
	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0046da90
void AGateCrashMinigameState::Pause()
{
	m_bIntroPending = TFALSE;

	// Play the intro, the game starts once the sequence state is removed (see AMiniGameState::OnActivate)
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "SSMJ_Main", "SSMJ_Main", "SSMJ_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// Hides the players while the intro plays.
// $Barnyard: FUNCTION 0046db20
void AGateCrashMinigameState::OnPreStart()
{
	if ( !m_bIntroPending )
		return;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
		TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ i ].GetUnit() )->SetVisible( TFALSE );

	for ( TINT i = 0; i < NUM_SPECTATORS; i++ )
		m_apSpectators[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );

	AAnimatedUnit* pServingUnit = TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ m_oGame.iServingPlayer ].GetUnit() );
	pServingUnit->SetVisible( TFALSE );
	pServingUnit->SetMovement( TFALSE );
	pServingUnit->SetAIEnabled( TFALSE );
	pServingUnit->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	if ( m_bCharityVariant )
		SetCharityActorsSkeletonUpdating( TFALSE );
}

// $Barnyard: FUNCTION 0046d3a0
TBOOL AGateCrashMinigameState::IsGameOver()
{
	return !( m_iNumAlive > 1 && m_oGame.aSlots[ 0 ].eState < PLAYERSTATE_KO_LEFT );
}

// $Barnyard: FUNCTION 0046f480
void AGateCrashMinigameState::UpdateAlways( TFLOAT a_fDeltaTime )
{
	UpdateGatePosition( m_vPipePos, m_oGame.fGateAngle );

	// NOTE: the result isn't used
	TVector4 vGateDir;
	ACamera::CreateCameraDirection_RotateY( vGateDir, m_oGame.fGateAngle );

	UpdateDrips();
}

// $Barnyard: FUNCTION 0046f4c0
void AGateCrashMinigameState::UpdateGame( TFLOAT a_fDeltaTime )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" runs let a computer player of its own jump and kick
	// for the human players (the same AI the other slots use, with steady reactions)
	if ( T2String8::FindString( GetCommandLineA(), "-devautoplay" ) && s_iDevNumHumans > 0 )
	{
		static AGateCrashPlayerAi* s_pDevAi = TNULL;

		if ( !s_pDevAi )
			s_pDevAi = new AGateCrashPlayerAi();

		if ( !s_bDevAiReady )
		{
			s_bDevAiReady = TTRUE;
			s_pDevAi->SetGameData( &m_oGame );
			s_pDevAi->Init( 0, s_iDevNumHumans );

			for ( TINT i = 0; i < s_iDevNumHumans; i++ )
				s_pDevAi->SetParams( i, 0.35f, 0.05f, 0.05f, 0.0f );

			TINFO( "[devautoplay] mudjumpers: AI for %d human player(s)\n", s_iDevNumHumans );
		}

		s_pDevAi->Update( a_fDeltaTime );
	}
#endif

	if ( m_oGame.iServingPlayer >= 0 )
	{
		// Serving: the gate follows the serving player's hands
		PlayerSlot&        rServingSlot    = m_oGame.aSlots[ m_oGame.iServingPlayer ];
		AAnimatedUnit*     pServingUnit    = TSTATICCAST( AAnimatedUnit, rServingSlot.GetUnit() );
		AAnimatableObject* pServingAnimObj = pServingUnit->GetAnimatableObject();

		if ( pServingAnimObj->IsAnimPlaying( TPS8( ms_serve ), 0.0f ) )
		{
			AModelInstance* pModelInstance = pServingAnimObj->GetModelInstance();

			TMatrix44 mLocator;
			pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_llocator ) ), mLocator );

			TVector4 vFromPipe( mLocator.m_f41 - m_vPipePos.x, 0.0f, mLocator.m_f43 - m_vPipePos.z, mLocator.m_f44 );
			TFLOAT   fAngle = vFromPipe.GetAngleBetweenVector( ACamera::sm_vInitialLookDirection );
			vFromPipe.AsVector3().RotateY( TMath::HALF_PI );

			if ( TVector4::DotProduct3( vFromPipe, ACamera::sm_vInitialLookDirection ) < 0.0f )
				fAngle *= -1.0f;

			m_oGame.fGateAngle = fAngle;
			m_fPrevGateAngle   = fAngle;
		}

#ifdef BARNYARD_COMMUNITY_PATCH
		// Development: "-devautoplay" runs press kick for the serving player
		if ( T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
			rServingSlot.bKickPressed = TTRUE;
#endif

		if ( !m_oServeListener.IsLinked() && rServingSlot.bKickPressed )
		{
			pServingAnimObj->PlayAnimation( TPS8( ms_serve ) );

			AAnimationBreakpointCallback* pBreakpoint = pServingAnimObj->GetAnimationEmitter( TPS8( ms_serve ) );
			m_oServeListener.Connect( pBreakpoint->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnServeFinished ), 0 );
		}

		for ( TINT i = 0; i < m_iNumPlayers; i++ )
		{
			m_oGame.aSlots[ i ].bJumpPressed = TFALSE;
			m_oGame.aSlots[ i ].bKickPressed = TFALSE;
		}

		return;
	}

	// NOTE: the result isn't used
	TVector4 vGateDir;
	ACamera::CreateCameraDirection_RotateY( vGateDir, m_oGame.fGateAngle );

	TFLOAT fStartGateSpeed = m_oGame.fGateSpeed;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		PlayerSlot& rSlot = m_oGame.aSlots[ i ];

		TFLOAT fDelta = rSlot.fAngle - m_oGame.fGateAngle;
		TMath::NormaliseAngle( fDelta );
		TFLOAT fAbsDelta = TMath::Abs( fDelta );

		AAnimatedUnit*     pUnit    = TSTATICCAST( AAnimatedUnit, rSlot.GetUnit() );
		AAnimatableObject* pAnimObj = pUnit->GetAnimatableObject();

		if ( !IsGameOver() )
		{
			if ( rSlot.eState == PLAYERSTATE_STUNNED )
				rSlot.fStunTimer += a_fDeltaTime;

			if ( rSlot.eState <= PLAYERSTATE_KICKING )
			{
				// Has the gate reached or passed the slot this frame?
				TBOOL bGateReached =
				    fAbsDelta < GATE_HIT_ANGLE ||
				    ( m_fPrevGateAngle < rSlot.fAngle && fStartGateSpeed > 0.0f && ( m_oGame.fGateAngle > rSlot.fAngle || m_oGame.fGateAngle < m_fPrevGateAngle ) ) ||
				    ( m_fPrevGateAngle > rSlot.fAngle && fStartGateSpeed < 0.0f && ( m_oGame.fGateAngle < rSlot.fAngle || m_oGame.fGateAngle > m_fPrevGateAngle ) );

				if ( bGateReached )
				{
					if ( rSlot.eState != PLAYERSTATE_STUNNED )
					{
						if ( rSlot.vPosition.y <= -0.5f )
						{
							// High enough in the air
							rSlot.bDodging = TTRUE;
						}
						else
						{
							OnGateHit( i );
							m_oGame.fGateSpeed *= GATE_HIT_SPEED_SCALE;
						}
					}
				}
				else if ( rSlot.eState == PLAYERSTATE_STUNNED && rSlot.fStunTimer > 0.2f )
				{
					rSlot.eState = PLAYERSTATE_READY;
				}
			}

			switch ( rSlot.eState )
			{
				case PLAYERSTATE_READY:
					if ( !pAnimObj->IsAnimPlaying( TPS8( ms_serve ), 0.0f ) )
					{
						if ( rSlot.bJumpPressed == TTRUE )
						{
							rSlot.fJumpVelocity = 12.0f;
							rSlot.eState        = PLAYERSTATE_JUMPING;
							rSlot.bJumpHeld     = TTRUE;
							pAnimObj->PlayAnimation( TPS8( jump_start ) );
						}

						if ( rSlot.bKickPressed == TTRUE )
						{
							rSlot.fKickTimer = 0.2f;
							pAnimObj->KillAllAnimations();

							const TPString8* pKickAnim;

							if ( rSlot.bDirectionalKick == TTRUE )
								pKickAnim = ( rSlot.bKickLeft == TTRUE ) ? &TPS8( ms_kick_l ) : &TPS8( ms_kick_r );
							else
								pKickAnim = ( m_oGame.fGateSpeed > 0.0f ) ? &TPS8( ms_kick_l ) : &TPS8( ms_kick_r );

							pAnimObj->PlayAnimation( *pKickAnim );
							rSlot.eState = PLAYERSTATE_KICKING;
						}
					}
					break;
				case PLAYERSTATE_JUMPING:
					if ( rSlot.bJumpHeld && pAnimObj->IsAnimPlaying( TPS8( jump_up ), 0.0f ) )
						rSlot.fJumpVelocity += a_fDeltaTime * 20.0f;

					if ( !pAnimObj->IsAnimPlaying( TPS8( jump_start ), 0.0f ) )
					{
						// NOTE: the height grows downwards
						rSlot.vPosition.y -= a_fDeltaTime * rSlot.fJumpVelocity;

						if ( rSlot.vPosition.y < 0.0f )
						{
							rSlot.fJumpVelocity -= a_fDeltaTime * 49.0f;

							if ( m_bCharityVariant && i == 0 )
							{
								// The first player collects the crowd's barnbucks by jumping into them
								for ( TINT k = 0; k < NUM_CROWD; k++ )
								{
									TVector4 vCrowdPos;
									m_apCrowd[ k ]->GetModelInstance()->GetPosition( vCrowdPos );

									if ( m_afCrowdTimers[ k ] == -1.0f && rSlot.vPosition.y - 2.0f <= vCrowdPos.y )
									{
										StartCollectEffect( m_apCrowd[ k ], 1, 30.0f );
										m_afCrowdTimers[ k ] = 0.0f;
									}
								}
							}
						}
						else if ( rSlot.fJumpVelocity != 0.0f )
						{
							// Landed
							rSlot.fJumpVelocity = 0.0f;
							rSlot.vPosition.y   = 0.0f;
							pAnimObj->KillAllAnimations();
							pAnimObj->PlayAnimation( TPS8( jump_end ) );
							rSlot.eState = PLAYERSTATE_READY;

							if ( i == 0 && rSlot.bDodging )
								m_iNumDodges++;

							rSlot.bDodging = TFALSE;
						}

						if ( pAnimObj->IsAnimPlaying( TPS8( jump_liftoff ), 0.0f ) )
						{
							ASoundManager::GetSingleton()->PlayCueAtPosition( JUMP_CUES[ i ], rSlot.vPosition );
							pAnimObj->PlayAnimation( TPS8( jump_up ) );
						}

						if ( pAnimObj->IsAnimPlaying( TPS8( jump_up ), 0.0f ) && rSlot.fJumpVelocity < 4.0f )
							pAnimObj->PlayAnimation( TPS8( jump_apex ) );

						if ( pAnimObj->IsAnimPlaying( TPS8( jump_apex ), 0.0f ) && rSlot.fJumpVelocity < -4.0f )
							pAnimObj->PlayAnimation( TPS8( jump_down ) );

						pUnit->SetSteerPosition( rSlot.vPosition );
					}
					break;
				case PLAYERSTATE_KICKING:
					rSlot.fKickTimer -= a_fDeltaTime;

					if ( rSlot.fKickTimer > 0.0f && fAbsDelta < KICK_HIT_ANGLE )
					{
						TFLOAT fGateDelta = m_oGame.fGateAngle - rSlot.fAngle;
						TMath::NormaliseAngle( fGateDelta );

						// A directional kick has to face the incoming gate
						TBOOL bRightSide = TTRUE;

						if ( rSlot.bDirectionalKick == TTRUE )
						{
							if ( m_oGame.fGateSpeed > 0.0f )
								bRightSide = rSlot.bKickLeft == TTRUE;
							else
								bRightSide = rSlot.bKickLeft != TTRUE;
						}

						if ( fGateDelta * m_oGame.fGateSpeed < 0.0f && bRightSide == TTRUE )
						{
							// Kicked the gate back
							m_oGame.fGateSpeed = GetGateSpeedSign() * -0.3f - m_oGame.fGateSpeed * 1.0f;

							if ( i == 0 )
								m_iNumHits++;

							m_iLastKicker = i;

							if ( m_bCharityVariant && i == 0 && m_fPostTimer == -1.0f )
							{
								for ( TINT k = 0; k < NUM_POSTS; k++ )
									StartCollectEffect( m_apPosts[ k ], 1, 30.0f );

								m_fPostTimer = 0.0f;
							}

							ASoundManager::GetSingleton()->PlayCueAtPosition( 0x3c, rSlot.vPosition );
							ASoundManager::GetSingleton()->PlayCueAtPosition( 0x36, rSlot.vPosition );

							APlayerSlot* pPlayerSlot = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlot( i );

							if ( pPlayerSlot->m_pController )
							{
								AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 0.25f, 0.0f, 1.0f, 1.0f }, 2 };
								AInputHandler::GetSingleton()->SetRumble( pPlayerSlot->m_pController, &oRumble );
							}

							// NOTE: the transform of the bone that kicked isn't used
							const TCHAR* pszBone = pAnimObj->IsAnimPlaying( TPS8( ms_kick_l ), 0.0f ) ? "c_l_foot" : "c_r_hand";
							TINT         iBone   = pUnit->GetModelInstance()->GetSkeletonInstance()->GetBoneID( pszBone );

							TMatrix44 mKickBone;
							pUnit->GetModelInstance()->GetBoneTransform( iBone, mKickBone );
						}
					}

					if ( rSlot.fKickTimer < -0.3f )
						rSlot.eState = PLAYERSTATE_READY;
					break;
			}

			rSlot.bJumpPressed = TFALSE;
			rSlot.bKickPressed = TFALSE;

			if ( rSlot.eState == PLAYERSTATE_READY || rSlot.eState == PLAYERSTATE_JUMPING )
			{
				// Look at the incoming gate
				if ( fAbsDelta >= TMath::PI * 0.75f )
				{
					pAnimObj->FinishAnimation( TPS8( look_l ) );
					pAnimObj->FinishAnimation( TPS8( look_r ) );
				}
				else if ( fDelta > 0.0f )
				{
					if ( !pAnimObj->IsAnimPlaying( TPS8( look_l ), 0.0f ) )
					{
						pAnimObj->FinishAnimation( TPS8( look_r ) );
						pAnimObj->PlayAnimation( TPS8( look_l ) );
					}
				}
				else
				{
					if ( !pAnimObj->IsAnimPlaying( TPS8( look_r ), 0.0f ) )
					{
						pAnimObj->FinishAnimation( TPS8( look_l ) );
						pAnimObj->PlayAnimation( TPS8( look_r ) );
					}
				}
			}
		}

		if ( !rSlot.bMudSplashed && ( rSlot.eState == PLAYERSTATE_KO_LEFT || rSlot.eState == PLAYERSTATE_KO_RIGHT ) )
		{
			// Splash once the body hits the mud
			TINT iBone = pUnit->GetModelInstance()->GetSkeletonInstance()->GetBoneID( "C_Spine1" );

			TMatrix44 mSpine;
			pUnit->GetModelInstance()->GetBoneTransform( iBone, mSpine );

			TVector4 vSpinePos = mSpine.GetTranslation();

			if ( vSpinePos.y > 0.2f )
			{
				rSlot.bMudSplashed = TTRUE;
				ASoundManager::GetSingleton()->PlayCueAtPosition( 0x37, vSpinePos );
				AParticleEffectManager::GetSingleton()->PlayEffect( AParticleEffectManager::FindEffect( "mudsplashbig" ), vSpinePos, TVector4::VEC_NEGY );
			}
		}

		if ( ( rSlot.eState == PLAYERSTATE_KO_LEFT && pAnimObj->IsAnimationFinished( TPS8( ms_impact_l ) ) ) ||
		     ( rSlot.eState == PLAYERSTATE_KO_RIGHT && pAnimObj->IsAnimationFinished( TPS8( ms_impact_r ) ) ) )
		{
			rSlot.eState = PLAYERSTATE_OUT;

			// The others react to the knock out
			for ( TINT k = 0; k < m_iNumPlayers; k++ )
			{
				if ( ( k != 1 || i != 0 ) && k != i )
				{
					TINT iCue = ( k == 0 ) ? ( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetGender() == 1 ) : k + 1;
					ASoundManager::GetSingleton()->PlayCueAtPosition( KO_CUES[ iCue ], m_oGame.aSlots[ k ].vPosition );
				}
			}
		}

		if ( rSlot.eState > PLAYERSTATE_KICKING && m_aLifeHuds[ i ].IsVisible() && m_aLifeHuds[ i ].m_fSpin == m_aLifeHuds[ i ].m_fSpinTarget )
		{
			m_aLifeHuds[ i ].SetVisible( TFALSE );
		}
	}

	// Slow the gate down, but not below the minimum speed that grows over time
	TINT iSign = ( m_oGame.fGateSpeed > 0.0f ) ? 1 : -1;
	m_oGame.fGateSpeed -= iSign * a_fDeltaTime * GATE_SPEED_DECAY;
	m_fMinGateSpeed += a_fDeltaTime * GATE_MIN_SPEED_RAMP;

#ifdef BARNYARD_LATEST
	// NOTE: the latest version (Wii 8018dbd4) lowers the minimum speed and caps the speed (s_fMaxGateSpeedBase) on both
	// difficulties. Here only Easy does, so Normal plays like PC retail.
	const TBOOL bEasy            = APlayerProgress::GetSingleton()->IsEasyDifficulty();
	TFLOAT      fMinGateSpeedCap = ( MAX_PLAYERS - m_iNumAlive ) * GATE_MIN_SPEED_ALIVE + ( bEasy ? GATE_MIN_SPEED_BASE_WII : GATE_MIN_SPEED_BASE );
#else
	TFLOAT fMinGateSpeedCap = ( MAX_PLAYERS - m_iNumAlive ) * GATE_MIN_SPEED_ALIVE + GATE_MIN_SPEED_BASE;
#endif

	if ( fMinGateSpeedCap < m_fMinGateSpeed )
		m_fMinGateSpeed = fMinGateSpeedCap;

	if ( TMath::Abs( m_oGame.fGateSpeed ) < m_fMinGateSpeed && m_fGameOverTimer == 0.0f )
	{
		iSign              = ( m_oGame.fGateSpeed > 0.0f ) ? 1 : -1;
		m_oGame.fGateSpeed = iSign * m_fMinGateSpeed;
	}
#ifdef BARNYARD_LATEST
	else if ( bEasy )
	{
		TFLOAT fMaxGateSpeed = ( MAX_PLAYERS - m_iNumAlive ) * GATE_MIN_SPEED_ALIVE + s_fMaxGateSpeedBase;

		if ( TMath::Abs( m_oGame.fGateSpeed ) > fMaxGateSpeed )
		{
			iSign              = ( m_oGame.fGateSpeed > 0.0f ) ? 1 : -1;
			m_oGame.fGateSpeed = iSign * fMaxGateSpeed;
		}
	}
#endif

	m_fPrevGateAngle = m_oGame.fGateAngle;
	m_oGame.fGateAngle += a_fDeltaTime * m_oGame.fGateSpeed;
	TMath::NormaliseAngle( m_oGame.fGateAngle );
	UpdateGatePosition( m_vPipePos, m_oGame.fGateAngle );

	// Random creaking
	if ( TMath::Abs( m_oGame.fGateSpeed ) > 0.0f &&
	     !ASoundManager::GetSingleton()->IsCuePlaying( m_iCreakCue ) &&
	     ARandom::GetSingleton()->m_oRandom.GetFloat() < a_fDeltaTime * 0.5f )
	{
		m_iCreakCue = ASoundManager::GetSingleton()->PlayCueAtPosition( 0x35, m_vPipePos );
	}

	if ( m_bCharityVariant )
	{
		TMatrix44 mBuck = TMatrix44::IDENTITY;
		mBuck.RotateY( m_fElapsedTime + m_fElapsedTime );
		mBuck.RotateX( TMath::HALF_PI );

		// The crowd's barnbucks pop up one after another in front of the first player
		for ( TINT k = 0; k < NUM_CROWD; k++ )
		{
			if ( m_afCrowdTimers[ k ] >= 0.0f && m_iCrowdCount < MAX_CROWD_BUCKS )
			{
				m_afCrowdTimers[ k ] += a_fDeltaTime;

				if ( m_afCrowdTimers[ k ] > 1.0f )
				{
					m_apCrowd[ k ]->SetSkeletonUpdating( TTRUE, TTRUE );
					m_afCrowdTimers[ k ] = -1.0f;
					m_iCrowdCount++;
				}
			}

			if ( m_afCrowdTimers[ k ] == -1.0f )
			{
				mBuck.m_f41 = m_oGame.aSlots[ 0 ].vPosition.x;
				mBuck.m_f42 = -2.5f - k * 0.6f;
				mBuck.m_f43 = m_oGame.aSlots[ 0 ].vPosition.z;
				mBuck.m_f44 = m_oGame.aSlots[ 0 ].vPosition.w;
				m_apCrowd[ k ]->GetModelInstance()->SetTransform( mBuck );
			}
		}

		if ( m_fPostTimer >= 0.0f && m_iCrowdCount < MAX_CROWD_BUCKS )
		{
			m_fPostTimer += a_fDeltaTime;

			if ( m_fPostTimer > 1.0f )
			{
				for ( TINT k = 0; k < NUM_POSTS; k++ )
					m_apPosts[ k ]->SetSkeletonUpdating( TTRUE, TTRUE );

				m_fPostTimer = -1.0f;
				m_iCrowdCount += NUM_POSTS;
			}
		}

		if ( m_fPostTimer == -1.0f )
		{
			// The posts' barnbucks ride along the gate
			for ( TINT k = 0; k < NUM_POSTS; k++ )
			{
				TVector4 vAlongGate = ACamera::sm_vInitialLookDirection;
				vAlongGate.AsVector3().RotateY( -m_oGame.fGateAngle );
				vAlongGate.Multiply( k * 0.6f + 1.0f );

				mBuck.m_f41 = vAlongGate.x + m_vPipePos.x;
				mBuck.m_f42 = vAlongGate.y + m_vPipePos.y - 1.0f;
				mBuck.m_f43 = vAlongGate.z + m_vPipePos.z;
				mBuck.m_f44 = vAlongGate.w;
				m_apPosts[ k ]->GetModelInstance()->SetTransform( mBuck );
			}
		}
	}
}

// $Barnyard: FUNCTION 0046dd50
void AGateCrashMinigameState::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	if ( m_fGameOverTimer == 0.0f )
		PlayEndPoses();

	// The gate stops within 2 seconds
	TFLOAT fZero = 0.0f;
	m_fGameOverTimer += a_fDeltaTime;
	TFLOAT fSpeedScale = 1.0f - m_fGameOverTimer * 0.5f;
	TINT   iSign       = ( m_oGame.fGateSpeed > 0.0f ) ? 1 : -1;
	m_oGame.fGateSpeed = iSign * ( ( fSpeedScale < 0.0f ) ? fZero : fSpeedScale );

	if ( m_oGame.aSlots[ 0 ].eState <= PLAYERSTATE_KICKING )
	{
		// The first player has survived: move the camera to them
		ALerpCameraHelper* pLerpHelper = TSTATICCAST( ALerpCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_LERP ) );
		ACameraManager::GetSingleton()->SetSmoothTransition( pLerpHelper, 2.0f );

		TVector4 vCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
		ACameraManager::GetSingleton()->GetCurrentCamera();

		TVector4 vTarget = m_oGame.aSlots[ 0 ].vPosition;
		vTarget.y -= 1.0f;

		TVector4 vToTarget( vTarget.x - vCameraPos.x, vTarget.y - vCameraPos.y, vTarget.z - vCameraPos.z, vTarget.w );
		vToTarget.Normalise();

		TVector4 vOffset;
		vOffset.Multiply( vToTarget, 5.0f );

		TVector4 vNewCameraPos( vTarget.x - vOffset.x, vTarget.y - vOffset.y, vTarget.z - vOffset.z, vTarget.w );

		pLerpHelper->SetPosition( vNewCameraPos, vTarget );
		pLerpHelper->SetDestination( vNewCameraPos, vTarget );
	}

	if ( m_fGameOverTimer >= GAME_OVER_DELAY )
	{
		m_oGame.fGateSpeed = 0.0f;
		UpdateDrips();

		for ( TINT i = 0; i < m_iNumPlayers; i++ )
			TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ i ].GetUnit() )->GetAnimatableObject()->KillAllAnimations();

		ASoundManager::GetSingleton()->StopCue( m_iMudAmbienceCue );

		AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
	}
}

// $Barnyard: FUNCTION 0046d130
void AGateCrashMinigameState::UpdatePreStart( TFLOAT a_fDeltaTime )
{
	if ( m_fPreStartTimer == 0.0f )
	{
		m_vCameraPos.x = PRESTART_CAMERA_OFFSET[ 0 ] + m_vPipePos.x;
		m_vCameraPos.y = PRESTART_CAMERA_OFFSET[ 1 ] + m_vPipePos.y;
		m_vCameraPos.z = PRESTART_CAMERA_OFFSET[ 2 ] + m_vPipePos.z;
		m_vCameraPos.w = PRESTART_CAMERA_OFFSET[ 3 ];
		ACameraManager::GetSingleton()->GetCurrentCamera()->LookAt( m_vCameraPos, m_vPipePos );

		for ( TINT i = 0; i < m_iNumPlayers; i++ )
		{
			AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ i ].GetUnit() );
			pUnit->GetAnimatableObject()->PlayAnimation( ( i == m_oGame.iServingPlayer ) ? TPS8( ms_serve_idle ) : TPS8( idle_lp ) );
		}

		m_iMudAmbienceCue = ASoundManager::GetSingleton()->PlayCueAtPosition( 0x39, m_vPipePos );

		APositionalSoundSet::GetSingleton()->Load( "Data/Sound/Mudjump.trb" );
	}

	m_fPreStartTimer += a_fDeltaTime;
}

// $Barnyard: FUNCTION 0046e050
AMiniGameState::S1* AGateCrashMinigameState::GetResults()
{
	if ( IsGameOver() && m_oGame.aSlots[ 0 ].eState <= PLAYERSTATE_KICKING )
		m_oResults.iResult = 0;
	else
		m_oResults.iResult = -1;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();
	m_oResults.bWon           = m_oResults.iResult == 0;

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 0x1f5 ), m_iNumDodges * m_aScores[ SCORE_DODGE ] } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 0x1f6 ), m_iNumHits * m_aScores[ SCORE_HIT ] } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 0x1f7 ), m_iNumKOs * m_aScores[ SCORE_KO ] } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( 0x1f8 ), m_aScores[ SCORE_LIFE ] * m_oGame.aSlots[ 0 ].iLives } );
	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

		if ( iVariant == 0 || iVariant == 1 )
		{
			AHighScoreManager::AddScore( 0x35, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
		}
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 0046d250
void AGateCrashMinigameState::OnStartHudFinished()
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		m_oGame.aSlots[ i ].bJumpPressed = TFALSE;
		m_oGame.aSlots[ i ].bKickPressed = TFALSE;

		m_aLifeHuds[ i ].SetVisible( TTRUE );
	}

	if ( m_bCharityVariant == TTRUE )
	{
		m_oPickUpHudElement.SetNumber( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetMiniGamePickups() );
		m_oPickUpHudElement.SetForceHide( TFALSE );
	}

	m_fMinGateSpeed = 1.2f;
}

// $Barnyard: FUNCTION 0046cd20
TINT AGateCrashMinigameState::GetGateSpeedSign() const
{
	return ( m_oGame.fGateSpeed > 0.0f ) ? 1 : -1;
}

// $Barnyard: FUNCTION 0046d2d0
void AGateCrashMinigameState::OnImpactAnimEvent( void* a_pEventData )
{
	AAnimationBreakpointCallback::Event* pEvent = TSTATICCAST( AAnimationBreakpointCallback::Event, a_pEventData );

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, m_oGame.aSlots[ i ].GetUnit() );

		if ( pEvent->pObject == pUnit->GetAnimatableObject() )
		{
			APlayerSlot* pPlayerSlot = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlot( i );

			if ( pPlayerSlot->m_pController )
			{
				AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f }, 2 };
				AInputHandler::GetSingleton()->SetRumble( pPlayerSlot->m_pController, &oRumble );
			}

			return;
		}
	}
}

// $Barnyard: FUNCTION 0046dd30
TBOOL __stdcall AGateCrashMinigameState::OnImpactAnimEventCallback( AGateCrashMinigameState* a_pThis, void* a_pOwner, void* a_pData )
{
	a_pThis->OnImpactAnimEvent( a_pData );
	return TTRUE;
}

// $Barnyard: FUNCTION 0046d3c0
void AGateCrashMinigameState::PlayEndPoses()
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		PlayerSlot&    rSlot = m_oGame.aSlots[ i ];
		AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, rSlot.GetUnit() );

		if ( rSlot.eState < PLAYERSTATE_KO_LEFT )
		{
			// Survivors turn to the camera and celebrate
			pUnit->GetAnimatableObject()->KillAllAnimations();

			TVector4 vToCamera( m_vCameraPos.x - rSlot.vPosition.x, 0.0f, m_vCameraPos.z - rSlot.vPosition.z, m_vCameraPos.w );
			vToCamera.Normalise();
			pUnit->GetSteer()->SetDirection( vToCamera );

			rSlot.vPosition.y = 0.0f;
			pUnit->SetSteerPosition( rSlot.vPosition );
			pUnit->GetAnimatableObject()->PlayAnimation( TPS8( victory ) );
		}
	}

	for ( TINT i = 0; i < NUM_SPECTATORS; i++ )
		m_apSpectators[ i ]->PlayAnimation( TPS8( end ) );
}

// $Barnyard: FUNCTION 0046d4f0
void AGateCrashMinigameState::OnGateHit( TINT a_iSlot )
{
	PlayerSlot&        rSlot    = m_oGame.aSlots[ a_iSlot ];
	AAnimatedUnit*     pUnit    = TSTATICCAST( AAnimatedUnit, rSlot.GetUnit() );
	AAnimatableObject* pAnimObj = pUnit->GetAnimatableObject();

	m_aLifeHuds[ a_iSlot ].AddLives( -1 );

	APlayerSlot* pPlayerSlot = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlot( a_iSlot );

	if ( pPlayerSlot->m_pController )
	{
		AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f }, 2 };
		AInputHandler::GetSingleton()->SetRumble( pPlayerSlot->m_pController, &oRumble );
	}

	TINT iLastKicker = m_iLastKicker;
	m_iLastKicker    = -1;

	if ( --rSlot.iLives > 0 )
	{
		pAnimObj->PlayAnimation( TPS8( ms_contact ) );
		rSlot.eState        = PLAYERSTATE_STUNNED;
		rSlot.fStunTimer    = 0.0f;
		rSlot.vPosition.y   = 0.0f;
		rSlot.fJumpVelocity = 0.0f;
		rSlot.fKickTimer    = 0.0f;
		pUnit->SetSteerPosition( rSlot.vPosition );
		return;
	}

	// Knocked out
	rSlot.eState        = ( m_oGame.fGateSpeed > 0.0f ) ? PLAYERSTATE_KO_RIGHT : PLAYERSTATE_KO_LEFT;
	rSlot.vPosition.y   = 0.0f;
	rSlot.fJumpVelocity = 0.0f;
	rSlot.fKickTimer    = 0.0f;
	rSlot.bMudSplashed  = TFALSE;

	if ( iLastKicker == 0 && a_iSlot != 0 )
	{
		// The first player has kicked the gate into this one
		m_iNumKOs++;
		m_oGame.aSlots[ 0 ].iLives++;
		m_aLifeHuds[ 0 ].AddLives( 1 );
	}

	if ( m_bCharityVariant && a_iSlot == 0 )
	{
		for ( TINT i = 0; i < NUM_CROWD; i++ )
			m_apCrowd[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );

		for ( TINT i = 0; i < NUM_POSTS; i++ )
			m_apPosts[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );
	}

	m_iNumAlive--;
	pUnit->SetSteerPosition( rSlot.vPosition );
	pAnimObj->KillAllAnimations();
	pAnimObj->PlayAnimation( ( rSlot.eState == PLAYERSTATE_KO_RIGHT ) ? TPS8( ms_impact_r ) : TPS8( ms_impact_l ) );

	for ( TINT i = 0; i < NUM_SPECTATORS; i++ )
		m_apSpectators[ i ]->PlayAnimation( TPS8( impact ) );
}

// $Barnyard: FUNCTION 0046d700
void AGateCrashMinigameState::UpdateDrips()
{
	// Tip of the gate
	TVector4 vGateDir;
	ACamera::CreateCameraDirection_RotateY( vGateDir, m_oGame.fGateAngle );

	TVector4 vTip;
	vTip.Multiply( vGateDir, GATE_TIP_DISTANCE );
	vTip.x += m_vPipePos.x;
	vTip.y += m_vPipePos.y;
	vTip.z += m_vPipePos.z;
	vTip.w = m_vPipePos.w;

	// Side the gate is moving away from
	TVector4 vSide;
	ACamera::CreateCameraDirection_RotateY( vSide, m_oGame.fGateAngle );
	TINT iSign = ( m_oGame.fGateSpeed > 0.0f ) ? 1 : -1;
	vSide.AsVector3().RotateY( -( iSign * TMath::HALF_PI ) );
	vSide.Normalise();

	TVector4 vSideOffset;
	vSideOffset.Multiply( vSide, 0.2f );

	TVector4 vHalfSideOffset;
	vHalfSideOffset.Divide( vSideOffset, 2.0f );

	TVector4 vDripPos;
	vDripPos.x = vTip.x - vHalfSideOffset.x;
	vDripPos.y = ( vTip.y - vHalfSideOffset.y ) - 0.2f;
	vDripPos.z = vTip.z - vHalfSideOffset.z;
	vDripPos.w = vTip.w;

	if ( !m_pDripEffect )
	{
		m_pDripEffect = AParticleEffectManager::GetSingleton()->PlayEffect( AParticleEffectManager::FindEffect( "waterdrip" ), vDripPos, TVector4::VEC_POSY );
	}
	else
	{
		m_pDripEffect->GetInstance()->GetTransform().GetTranslation() = vDripPos;
		m_pDripEffect->GetInstance()->GetTransform().LookAtDirection( vSide, TVector4::VEC_NEGY );
	}

	if ( !m_pMudDripEffect )
	{
		m_pMudDripEffect = AParticleEffectManager::GetSingleton()->PlayEffect( AParticleEffectManager::FindEffect( "waterdripmud" ), vDripPos, TVector4::VEC_POSY );
	}
	else
	{
		m_pMudDripEffect->GetInstance()->GetTransform().GetTranslation() = vDripPos;
		m_pMudDripEffect->GetInstance()->GetTransform().LookAtDirection( vSide, TVector4::VEC_NEGY );
	}

	ASoundManager::GetSingleton()->SetCuePosition( m_iMudAmbienceCue, vDripPos, 0.0f );
}

// $Barnyard: FUNCTION 0046dcd0
TBOOL __stdcall AGateCrashMinigameState::OnServeFinished( AGateCrashMinigameState* a_pThis, void* a_pOwner, void* a_pData )
{
	// The gate goes away from the serving player
	a_pThis->m_oGame.fGateSpeed = ( a_pThis->m_oGame.fGateAngle > a_pThis->m_oGame.aSlots[ a_pThis->m_oGame.iServingPlayer ].fAngle ) ? 1.0f : -1.0f;
	a_pThis->m_oServeListener.Disconnect();
	a_pThis->m_oGame.iServingPlayer = -1;

	return TTRUE;
}

// $Barnyard: FUNCTION 0046e250
void AGateCrashMinigameState::BuildGatePipe( TINT a_iNumSegments )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pPipeType       = pAnimObjManager->FindType( TPS8( gcpipe ) );

	TMatrix44 mPosition = TMatrix44::IDENTITY;
	mPosition.m_f41     = m_vPipePos.x;
	mPosition.m_f42     = m_vPipePos.y;
	mPosition.m_f43     = m_vPipePos.z;
	mPosition.m_f44     = m_vPipePos.w;
	mPosition.RotateX( TMath::HALF_PI );

	TMatrix44 mRotation = TMatrix44::IDENTITY;
	mRotation.RotateZ( 0.0f );

	TMatrix44 mIdentity = TMatrix44::IDENTITY;

	TMatrix44 mTemp;
	mTemp.Multiply( mPosition, mRotation );

	TMatrix44 mTransform;
	mTransform.Multiply( mTemp, mIdentity );

	// Reserve the head
	m_apPipes[ m_iNumPipes++ ] = TNULL;

	TINT iNumPipes = a_iNumSegments + 1;

	if ( iNumPipes > 1 )
	{
		TFLOAT fNumPipesSq = TFLOAT( iNumPipes * iNumPipes );

		for ( TINT i = a_iNumSegments; i != 0; i-- )
		{
			AAnimatableObject* pPipe = pAnimObjManager->CreateAnimatableObject( pPipeType, TNULL, 0 );
			pPipe->GetModelInstance()->SetTransform( m_vPipePos, TVector4::VEC_POSX );
			pPipe->SetSkeletonUpdating( TTRUE, TTRUE );
			pPipe->SetVisible( TTRUE );

			// Segments get thinner the further they are from the head
			TSceneObject* pSceneObject = pPipe->GetModelInstance()->GetSceneObject();
			pSceneObject->SetFlags( pSceneObject->GetFlags() | TSceneObject::FLAGS_OPACITY );
			pSceneObject->GetFadeParams().y = ( TFLOAT( i * i ) / fNumPipesSq ) * 0.5f;

			pPipe->GetModelInstance()->SetTransform( mTransform );
			m_apPipes[ m_iNumPipes++ ] = pPipe;
		}
	}

	AAnimatableObject* pHead = pAnimObjManager->CreateAnimatableObject( pPipeType, TNULL, 0 );
	pHead->GetModelInstance()->SetTransform( m_vPipePos, TVector4::VEC_POSX );
	pHead->SetSkeletonUpdating( TTRUE, TTRUE );
	pHead->SetVisible( TTRUE );

	TSceneObject* pSceneObject = pHead->GetModelInstance()->GetSceneObject();
	pSceneObject->SetFlags( pSceneObject->GetFlags() | TSceneObject::FLAGS_OPACITY );
	pSceneObject->GetFadeParams().y = 1.0f;

	pHead->GetModelInstance()->SetTransform( mTransform );
	m_apPipes[ 0 ] = pHead;
}

// $Barnyard: FUNCTION 0046e610
void AGateCrashMinigameState::UpdateGatePosition( const Toshi::TVector4& a_rcPosition, TFLOAT a_fAngle )
{
	TMatrix44 mPosition = TMatrix44::IDENTITY;
	mPosition.m_f41     = a_rcPosition.x;
	mPosition.m_f42     = a_rcPosition.y;
	mPosition.m_f43     = a_rcPosition.z;
	mPosition.m_f44     = a_rcPosition.w;
	mPosition.RotateX( TMath::HALF_PI );

	TMatrix44 mRotation = TMatrix44::IDENTITY;
	mRotation.RotateZ( a_fAngle );

	TMatrix44 mIdentity = TMatrix44::IDENTITY;

	// Every segment takes the previous transform of the one in front of it
	TMatrix44 mTransform;

	for ( TINT i = m_iNumPipes - 1; i > 0; i-- )
	{
		m_apPipes[ i - 1 ]->GetModelInstance()->GetLocalTransform( mTransform );
		m_apPipes[ i ]->GetModelInstance()->SetTransform( mTransform );
	}

	TMatrix44 mTemp;
	mTemp.Multiply( mPosition, mRotation );
	mTransform.Multiply( mTemp, mIdentity );

	m_apPipes[ 0 ]->GetModelInstance()->SetTransform( mTransform );
}

// $Barnyard: FUNCTION 0046e210
void AGateCrashMinigameState::ImpactListeners::FreeAll()
{
	while ( !oList.IsEmpty() )
	{
		ImpactListener* pListener = oList.Head();
		pListener->Remove();
		delete pListener;
	}
}
