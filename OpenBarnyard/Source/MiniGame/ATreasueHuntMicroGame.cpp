#include "pch.h"
#include <Render/TCameraObject.h>
#include "ATreasueHuntMicroGame.h"
#include "ALoadScreen.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Animation/AToshiAnimationInterface.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FSM/AFunctionPointer.h"
#include "FSM/AMethodPointer.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AMapState.h"
#include "Helpers/ACoinSpawner.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Quests/AQuestManager.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/barnyard.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AInteractionManager.h"
#include "Units/AInventoryComponent.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnitProxy.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObjectSimpleDummy.h"

// NOTE: Opcode #defines PI
#undef PI

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Render/TTexture.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0052ded0, CreateObject 0052fb00, CreateObjectInPlace 0052fb60
TDEFINE_CLASS_INIT( ATreasueHuntMicroGame, 1, 0 );

TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( detector );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp_uphill );
TPSTRING8_DECLARE( idle_lp_uphill );
TPSTRING8_DECLARE( MD_Walk_01 );
TPSTRING8_DECLARE( MD_idle );
TPSTRING8_DECLARE( fixpipe );
TPSTRING8_DECLARE( trap );
TPSTRING8_DECLARE( tin );
TPSTRING8_DECLARE( treasurefound );

// DAT_007839bc
AFunctionTable ATreasueHuntMicroGame::ms_oFunctionTable( &TGetClass( ATreasueHuntMicroGame ), TNULL );

// Locale strings
static constexpr TINT LOCALE_FIX_LEAK = 197; // context text at a leak
static constexpr TINT LOCALE_DIG      = 198; // context text close to a treasure

// Symbols of the pipes in pipespline1.trb (off_00773e8c)
static const TCHAR* s_aszPipes[ ATreasueHuntMicroGame::NUM_PIPES ] = { "PipeLine01", "PipeLine02" };

// $Barnyard: FUNCTION 0052eaf0
void ATreasueHuntMicroGame::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );
	ms_oFunctionTable.AddFunction( AFunctionPointer1Args<TBOOL>::Create( "PushState", &ATreasueHuntMicroGame::PushState ) );
}

// $Barnyard: FUNCTION 0052dee0
void ATreasueHuntMicroGame::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0052eb20
ATreasueHuntMicroGame::ATreasueHuntMicroGame()
    : m_fCameraYaw( 0.0f )
    , m_fCameraPitch( 0.0f )
    , m_pLeak( TNULL )
    , m_fDigCooldown( 0.0f )
    , m_fStillTime( 0.0f )
    , m_bRumbling( TFALSE )
    , m_eLoadState( LOADSTATE_LOADING )
    , m_fLoadTime( 0.0f )
{
	m_oHoles.vecActive.Clear();
	m_oHoles.vecFree.Clear();
	m_oPulses.vecActive.Clear();
	m_oPulses.vecFree.Clear();

	SetInputMap( TPS8( NormalBarnyard ) );

	m_pSensorCollision = new ACollisionObjectSimpleDummy();

	m_pSimState    = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) );
	m_uiSimHUDMask = m_pSimState->GetHUDMask();
}

// $Barnyard: FUNCTION 0052ecb0
// $Barnyard: FUNCTION 0052fba0
ATreasueHuntMicroGame::~ATreasueHuntMicroGame()
{
	m_oTRB.Close();

	if ( m_pSensorCollision )
		delete m_pSensorCollision;
}

// $Barnyard: FUNCTION 0052eca0
AFunctionTable* ATreasueHuntMicroGame::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0052e000
void ATreasueHuntMicroGame::PushState( TBOOL a_bUnused )
{
	APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	AGameStateController::GetSingleton()->PushState( TSTATICCAST( AGameState, TGetClass( ATreasueHuntMicroGame ).CreateObject() ) );
}

// $Barnyard: FUNCTION 0052def0
TBOOL ATreasueHuntMicroGame::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eLoadState != LOADSTATE_RUNNING )
		return TTRUE;

	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 00530810
TBOOL ATreasueHuntMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eLoadState != LOADSTATE_RUNNING )
		return TTRUE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
			AGameStateController::GetSingleton()->StartMenuIfPossible();
			return TTRUE;
		case AInputCommand_Select:
			AGameStateController::GetSingleton()->PushState( new AMapState() );
			return TTRUE;
		case AInputCommand_SuperSpeed:
		case AInputCommand_Shoot:
		case AInputCommand_ShootAlt:
		case AInputCommand_Jump:
		case AInputCommand_Switch:
			return TTRUE;
		case AInputCommand_Interact:
		case AInputCommand_Use:
		{
			ASimplePlayerControlComponent* pControlComponent = TSTATICCAST( ASimplePlayerControlComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( APlayerControlComponent ) ) );

			if ( pControlComponent && !pControlComponent->IsMovementEnabled() )
				return TTRUE;

			if ( FixLeak() )
				Remove();
			else
				Dig();

			return TTRUE;
		}
		case AInputCommand_Inventory:
			Remove();
			return TTRUE;
		default:
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 005309a0
TBOOL ATreasueHuntMicroGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	switch ( m_eLoadState )
	{
		case LOADSTATE_START_LOADING:
			m_pSimState->GetHUDParams().SetFlags( 0 );
			g_oLoadScreen.Reset();
			g_oLoadScreen.StartLoading( 0, TTRUE );
			m_eLoadState = LOADSTATE_LOADING;
			return TTRUE;
		case LOADSTATE_LOADING:
			m_fLoadTime += a_fDeltaTime;

			if ( ATerrainInterface::GetSingleton()->IsLoaded() && m_fLoadTime > 1.0f )
				m_eLoadState = LOADSTATE_LOADED;
			else
				g_oLoadScreen.Update( 1.0f, TTRUE );

			return TTRUE;
		case LOADSTATE_LOADED:
			AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );
			g_oLoadScreen.Reset();
			// Fall through
		case LOADSTATE_START:
			m_eLoadState = LOADSTATE_RUNNING;
			Start();
			return TTRUE;
		default:
			break;
	}

	// NOTE: the result is unused
	GetSensorPosition();

	m_fDigCooldown -= a_fDeltaTime;
	m_fStillTime -= a_fDeltaTime;

	m_oHoles.Update( a_fDeltaTime );
	UpdateSensor( a_fDeltaTime );
	ATreasureManager::GetSingleton()->UpdateRespawns();
	UpdateCamera( a_fDeltaTime );
	// NOTE: retail calls a DOES_NOTHING method (005acd80) of ATreasureManager here

	TVector4 vecSensor = GetSensorPosition();

	TBOOL  bLeak     = TFALSE;
	TFLOAT fDistance = DETECTOR_RANGE;
	TBOOL  bSabotage = TTRUE;

	// The leaks of the sabotage quests are looked for instead of the treasures
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( !pQuestManager->FindAssignedQuest( TPS8D( "Miles Raccoon Sabotage" ), 1 ) &&
	     !pQuestManager->FindAssignedQuest( TPS8D( "Miles Raccoon Sabotage 2" ), 1 ) )
	{
		bSabotage = TFALSE;
	}

	ATreasureManager::Treasure* pClosest = ATreasureManager::GetSingleton()->FindClosestTreasure( vecSensor, DETECTOR_RANGE );

	if ( bSabotage )
	{
		TFLOAT fPipeDistance = GetPipeDistance();

		if ( fPipeDistance < DETECTOR_RANGE )
		{
			bLeak     = TTRUE;
			fDistance = TMath::Min( fPipeDistance, DETECTOR_RANGE );
		}
	}
	else if ( pClosest )
	{
		fDistance = TMath::Min( TVector4::DistanceXZ( pClosest->vecPosition, vecSensor ), DETECTOR_RANGE );
	}

	UpdateDetector( a_fDeltaTime, ( DETECTOR_RANGE - fDistance ) * 0.1f, fDistance, bLeak );

	// Close to a leak?
	if ( pClosest && pClosest->eType == ATreasureManager::TYPE_LEAK )
	{
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( TVector4::DistanceXZ( *pPlayer->GetSteer()->GetPosition(), pClosest->vecPosition ) < 2.0f )
		{
			m_pLeak = pClosest;

			if ( APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) )
			{
				pControlComponent->SetOverrideText( g_pLocaleManager->GetString( LOCALE_FIX_LEAK ) );
				pControlComponent->SetContextEnabled( TTRUE );
			}
		}
	}

	if ( m_iBeepCue != -1 )
		ASoundManager::GetSingleton()->SetCuePosition( m_iBeepCue, vecSensor );

	// Done digging
	if ( m_fStillTime < 0.0f )
	{
		APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer() );

		if ( pSteer->GetState() == APlayerSteer::STATE_STILL )
			pSteer->SetState( APlayerSteer::STATE_WALKING );
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0052df20
void ATreasueHuntMicroGame::OnInsertion()
{
	m_eLoadState = LOADSTATE_START;
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0052eda0
void ATreasueHuntMicroGame::OnRemoval()
{
	m_pSimState->GetHUDParams().SetFlags( m_uiSimHUDMask );

	m_oHoles.Destroy();
	m_oPulses.Destroy();

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer() );

	if ( pSteer->GetState() == APlayerSteer::STATE_STILL )
		pSteer->SetState( APlayerSteer::STATE_WALKING );

	// NOTE: the result is unused
	AUnit* pPlayer = m_oPlayer ? m_oPlayer->GetUnit() : TNULL;
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

	AMusicManager::GetSingleton()->SetVolume( m_fMusicVolume );

	// NOTE: retail calls a RETURNS_TRUE function (0046cde0) here if the sensor collision object is linked

	pPlayer = m_oPlayer ? m_oPlayer->GetUnit() : TNULL;
	TSTATICCAST( ACommonSteer, pPlayer->GetSteer() )->SetDesiredVelocity( TVector4::VEC_ZERO );

	// Put the detector away
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	if ( pInventory->GetCategory( TPS8D( "Hand" ) )->GetNumItems() > 0 )
		AInventoryGroupComponent::PutAwayHandItem();

	FinishUphillAnimations();

	if ( APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) )
	{
		pControlComponent->SetOverrideText( TNULL );
		pControlComponent->SetContextEnabled( TTRUE );
	}

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0052fbc0
void ATreasueHuntMicroGame::Start()
{
	for ( TINT i = 0; i < NUM_PIPES; i++ )
		m_aPipes[ i ].Load( "pipespline1", s_aszPipes[ i ], 0 );

	m_iRumbleTimer = 30;

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	m_oPlayer      = pPlayer->GetSelfProxy();

	ASteer* pSteer      = pPlayer->GetSteer();
	m_vecStartPosition  = *pSteer->GetPosition();
	m_vecStartDirection = *pSteer->GetKinematics()->GetFacingDir();

	if ( APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) )
	{
		pControlComponent->SetContextEnabled( TFALSE );
		pControlComponent->SetOverrideText( TNULL );
	}

	m_HUDParams.SetFlags( AHudElement::MASK_UNK4 | AHudElement::MASK_UNK5 | AHudElement::MASK_MENU_NAV | AHudElement::MASK_UNK9 | AHudElement::MASK_MINIMAP | AHudElement::MASK_UNK11 | AHudElement::MASK_UNK12 | AHudElement::MASK_GAME_TIME | AHudElement::MASK_ITEM_COUNT | AHudElement::MASK_TERRAIN_SECTION | AHudElement::MASK_PLAYER_MAIN | AHudElement::MASK_TIME_LEFT );
	m_fBeepFlash = 0.0f;
	m_fBeepTimer = 0.0f;

	m_pSensorCollision->Create( GetSensorPosition(), 0.0f, 0.0f, 0.0f, 0, TNULL );

	// Only the beeps can be heard
	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	m_fMusicVolume               = pMusicManager->GetVolume();
	pMusicManager->SetVolume( 0.0f );

	m_iBeepCue    = -1;
	m_bPulse      = TFALSE;
	m_fPulseTimer = 0.5f;

	ATreasureManager* pTreasureManager = ATreasureManager::GetSingleton();
	pTreasureManager->SetTypeEnabled( ATreasureManager::TYPE_CANS, TTRUE );
	pTreasureManager->SetTypeEnabled( ATreasureManager::TYPE_FLOWERS, IsGopherTreasureQuestStarted() );
	pTreasureManager->SetTypeEnabled( ATreasureManager::TYPE_TRAPS, IsGopherTrapQuestStarted() );

	// The camera ignores the detector
	ACollisionObject* pDetectorCollision = TSTATICCAST( AVisibleUnit, TSTATICCAST( AAnimatedUnit, pPlayer )->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )->GetCollisionObject();
	pDetectorCollision->SetType( pDetectorCollision->GetType() | ACollisionObject::TYPE_IGNORED );

	m_oHoles.Create();
	m_oPulses.Create();
}

// $Barnyard: FUNCTION 0052f7d0
TBOOL ATreasueHuntMicroGame::IsGopherTreasureQuestStarted()
{
	TPString8      strQuest( "PIP Gopher Treasure" );
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( pQuestManager->IsQuestComplete( strQuest, 1 ) )
		return TTRUE;

	return pQuestManager->FindAssignedQuest( strQuest, 1 ) != TNULL;
}

// $Barnyard: FUNCTION 0052f8a0
TBOOL ATreasueHuntMicroGame::IsGopherTrapQuestStarted()
{
	TPString8      strQuest( "Otis Gopher Trap" );
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( pQuestManager->IsQuestComplete( strQuest, 1 ) )
		return TTRUE;

	return pQuestManager->FindAssignedQuest( strQuest, 1 ) != TNULL;
}

AUnit* ATreasueHuntMicroGame::GetDetector()
{
	AUnit* pPlayer = m_oPlayer ? m_oPlayer->GetUnit() : TNULL;
	return TSTATICCAST( AAnimatedUnit, pPlayer )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );
}

// $Barnyard: FUNCTION 0052f020
TVector4 ATreasueHuntMicroGame::GetSensorPosition()
{
	AUnit* pDetector = GetDetector();
	TINT   iBone     = TSTATICCAST( AAnimatedUnit, pDetector )->GetAnimatableObject()->GetModelInstance()->GetBoneIndex( TPS8D( "sensor" ) );

	TMatrix44 mBone;
	TSTATICCAST( AVisibleUnit, pDetector )->GetModelInstance()->GetBoneTransform( iBone, mBone );

	return mBone.GetTranslation();
}

// $Barnyard: FUNCTION 0052f520
void ATreasueHuntMicroGame::UpdateSensor( TFLOAT a_fDeltaTime )
{
	TFLOAT   fDistance    = 0.8f;
	TVector4 vecSensor    = GetSensorPosition();
	AUnit*   pPlayer      = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	TFLOAT   fStepHeight  = 0.0f;
	TFLOAT   fPenetration = 0.0f;
	TBOOL    bEdgeHit     = TFALSE;
	TVector4 vecPlayer    = *pPlayer->GetSteer()->GetPosition();

	m_pSensorCollision->UpdateHash( vecSensor.AsVector3() );

	// Test a sphere a bit under the player's position
	vecSensor.y = vecPlayer.y - 1.2f;
	m_pSensorCollision->UpdateHash( 0.5f );

	TFLOAT fSphereDistance = 0.5f;
	m_vecStartPosition     = *pPlayer->GetSteer()->GetPosition();
	m_vecStartDirection    = *pPlayer->GetSteer()->GetKinematics()->GetFacingDir();

	TVector4 vecNormal;

	if ( ACollisionManager::CollideSphereWithWorld( vecSensor, m_pSensorCollision, fSphereDistance, vecNormal, ACollisionObject::TYPE_IGNORED, TCollisionCommon::TOSHICGROUPFLAG_NONE ) ||
	     ACollisionManager::GetSingleton()->CollideSweptSphere( vecSensor, 0.5f, m_pSensorCollision, TVector4::VEC_POSY, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, TINT( TUINTPTR( &bEdgeHit ) ), ACollisionObject::TYPE_IGNORED, TNULL, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
	{
		TFLOAT fWeight;

		if ( fDistance == 0.8f )
		{
			fWeight = 1.0f;
		}
		else
		{
			fWeight = ( 0.8f - fDistance ) * 0.7f + 0.1f;
			TMath::Clip( fWeight, 0.1f, 1.0f );
		}

		UpdateAnimations( fWeight, a_fDeltaTime );
	}
	else
	{
		UpdateAnimations( 0.0f, a_fDeltaTime );
	}

	m_pSensorCollision->UpdateHash( 0.0f );
}

// $Barnyard: FUNCTION 0052f150
void ATreasueHuntMicroGame::UpdateAnimations( TFLOAT a_fWeight, TFLOAT a_fDeltaTime )
{
	AUnit*                    pPlayer           = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	AAnimatableObject*        pAnimatableObject = TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject();
	ANamedAnimationSetRef     pDetectorSet      = pAnimatableObject->GetObjectType()->FindAnimationSet( TPS8( detector ) );
	AToshiAnimationInterface& rInterface        = TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->GetToshiAnimInterface();

	// Use the animations of the detector
	if ( rInterface.GetAnimationSet().Get() != pDetectorSet.Get() )
	{
		rInterface.SetAnimationSet( pDetectorSet );
		return;
	}

	TPString8           strAnim;
	TPString8           strOtherAnim;
	TFLOAT              fAnimTime;
	AToshiAnimationRef* pBaseRef;
	const TPString8*    pBaseName;

	if ( pAnimatableObject->IsAnimPlaying( TPS8( walk_lp ), 0.0f ) )
	{
		strAnim      = TPS8( walk_lp_uphill );
		strOtherAnim = TPS8( idle_lp_uphill );
		fAnimTime    = pAnimatableObject->GetAnimationTime( TPS8( walk_lp ) );

		pBaseRef = AToshiAnimationInterface::GetAnimationRef( rInterface.GetAnimRefs(), TPS8( walk_lp ) );

		if ( !pBaseRef )
			pBaseRef = AToshiAnimationInterface::GetAnimationRef( rInterface.GetOverlayAnimRefs(), TPS8( walk_lp ) );

		pBaseName = &TPS8( MD_Walk_01 );
	}
	else if ( pAnimatableObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
	{
		strAnim      = TPS8( idle_lp_uphill );
		strOtherAnim = TPS8( walk_lp_uphill );
		fAnimTime    = pAnimatableObject->GetAnimationTime( TPS8( idle_lp ) );

		pBaseRef = AToshiAnimationInterface::GetAnimationRef( rInterface.GetAnimRefs(), TPS8( idle_lp ) );

		if ( !pBaseRef )
			pBaseRef = AToshiAnimationInterface::GetAnimationRef( rInterface.GetOverlayAnimRefs(), TPS8( idle_lp ) );

		pBaseName = &TPS8( MD_idle );
	}
	else
	{
		return;
	}

	// Only when the base animation is the one of the detector
	if ( *pBaseName != pBaseRef->GetNamedAnimation()->GetExportedName() )
		return;

	AToshiAnimationRef* pRef           = TNULL;
	TFLOAT              fCurrentWeight = a_fWeight;
	TBOOL               bStart         = TTRUE;

	if ( rInterface.IsAnimPlaying( strAnim, TFALSE ) )
	{
		pRef = AToshiAnimationInterface::GetAnimationRef( rInterface.GetAnimRefs(), strAnim );

		if ( !pRef )
			pRef = AToshiAnimationInterface::GetAnimationRef( rInterface.GetOverlayAnimRefs(), strAnim );

		fCurrentWeight = pRef->GetWeight();
		bStart         = TFALSE;
	}
	else if ( rInterface.IsAnimPlaying( strOtherAnim, TFALSE ) )
	{
		// Switch from walking to idling (or back)
		pRef           = rInterface.FindAnimationRef( strOtherAnim );
		fCurrentWeight = pRef->GetWeight();
		TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->FinishAnimation( strOtherAnim );
	}

	// The weight goes up right away and down smoothly
	TFLOAT fDifference = a_fWeight - fCurrentWeight;
	TFLOAT fWeight     = a_fWeight;

	if ( fDifference < 0.0f )
		fWeight = fDifference * a_fDeltaTime + fDifference * a_fDeltaTime + fCurrentWeight;

	if ( bStart )
		pRef = rInterface.PlayAnim( strAnim, fWeight, TFALSE );
	else
		rInterface.SetAnimWeight( pRef, fWeight );

	rInterface.SetAnimTime( pRef, fAnimTime );
	rInterface.SetAnimWeight( pRef, fWeight );
}

// $Barnyard: FUNCTION 0052fdf0
void ATreasueHuntMicroGame::UpdateDetector( TFLOAT a_fDeltaTime, TFLOAT a_fProximity, TFLOAT a_fDistance, TBOOL a_bLeak )
{
	// Close enough to dig (or to the pipe) beeps the same
	if ( a_bLeak )
	{
		if ( a_fDistance <= 1.0f )
			a_fDistance = 0.5f;
	}
	else if ( a_fDistance <= DIG_RADIUS )
	{
		a_fDistance = 1.4f;
	}

	// Beep faster when closer
	TFLOAT fRate = 0.4f / a_fDistance;
	TMath::Clip( fRate, 0.0f, 1.0f );

	TBOOL bBeep  = TFALSE;
	m_fBeepTimer = fRate * a_fDeltaTime * 45.0f + m_fBeepTimer;

	if ( m_fBeepTimer > 4.0f )
	{
		m_fBeepTimer = 0.0f;
		bBeep        = TTRUE;
		m_fBeepFlash = 0.99f;
	}
	else if ( m_fBeepTimer > 2.0f )
	{
		m_fBeepTimer = m_fBeepTimer - 2.0f;
		bBeep        = TTRUE;
		m_fBeepFlash = 0.99f;
	}

	if ( m_fBeepTimer > 0.5f )
	{
		m_fBeepFlash -= a_fDeltaTime * 5.0f;

		if ( m_fBeepFlash < 0.001f )
			m_fBeepFlash = 0.001f;
	}
	else
	{
		m_fBeepFlash = 0.99f;
	}

	TFLOAT fPulseTime = ( a_fProximity * a_fProximity + 1.0f ) * a_fDeltaTime;
	m_oPulses.Update( fPulseTime * 0.5f );

	if ( bBeep )
	{
		AUnit* pPlayer = m_oPlayer ? m_oPlayer->GetUnit() : TNULL;

		if ( TSTATICCAST( AVisibleUnit, TSTATICCAST( AAnimatedUnit, pPlayer )->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )->IsVisible() )
		{
			m_bPulse = TTRUE;

			TFLOAT fVolume = 0.7f;

			if ( a_fDistance < 20.0f )
			{
				TFLOAT fCloseness = ( 20.0f - a_fDistance ) * 0.05f;
				fVolume           = 1.0f - fCloseness * fCloseness * 0.8f;
			}

			if ( a_fDistance < DIG_RADIUS )
				fVolume -= 0.1f;

			if ( fVolume < 0.1f )
				fVolume = 0.1f;

			m_iBeepCue = ASoundManager::GetSingleton()->PlayCueEx( soundbank::BARNYARD_METALDETECTOR2, fVolume, TFALSE, 0.0f, -1 );

			if ( a_fDistance <= DIG_RADIUS || ( a_bLeak && a_fDistance <= 1.0f ) )
			{
				if ( !m_bRumbling )
				{
					m_bRumbling    = TTRUE;
					m_iRumbleTimer = 0;
				}
			}
			else
			{
				m_bRumbling = TFALSE;
			}

			if ( a_fDistance < 20.0f && --m_iRumbleTimer < 0 )
			{
				TFLOAT fStrength = ( 20.0f - a_fDistance ) * 0.04f;
				TMath::Clip( fStrength, 0.3f, 0.9f );

				AInputHandler::RumbleParams oRumble = { { fStrength, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f }, 1 };
				AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );

				m_iRumbleTimer = a_bLeak ? 30 : 20;
			}
		}
	}

	m_fPulseTimer -= fPulseTime;

	if ( m_bPulse && m_fPulseTimer < 0.0f && m_oPulses.vecFree.Size() != 0 )
	{
		m_oPulses.Spawn( GetSensorPosition(), 0, a_fDistance, a_bLeak );
		m_fPulseTimer = 0.5f;
		m_bPulse      = TFALSE;
	}
}

// $Barnyard: FUNCTION 0052f970
TFLOAT ATreasueHuntMicroGame::GetPipeDistance()
{
	TFLOAT fMinDistance = 3.0f;

	for ( TINT i = 0; i < NUM_PIPES; i++ )
	{
		// NOTE: retail calls a DOES_NOTHING function (00628f40) with -1.0f here
		TVector4 vecSensor  = GetSensorPosition();
		TVector4 vecClosest = m_aPipes[ i ].GetClosestPosition( vecSensor, fMinDistance );
		TFLOAT   fDistance  = TVector4::DistanceXZ( vecClosest, vecSensor );

		if ( fDistance < fMinDistance )
			fMinDistance = fDistance;
	}

	if ( fMinDistance < 3.0f )
		return fMinDistance;

	return DETECTOR_RANGE;
}

// $Barnyard: FUNCTION 0052fa10
TBOOL ATreasueHuntMicroGame::FixLeak()
{
	if ( !m_pLeak )
		return TFALSE;

	ATreasureManager::GetSingleton()->RemoveTreasure( m_pLeak );
	m_pLeak = TNULL;

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->PlayAnimation( TPS8( fixpipe ) );
	m_fDigCooldown = 2.5f;

	ATreasureManager* pTreasureManager = ATreasureManager::GetSingleton();

	if ( pTreasureManager->CountType( ATreasureManager::TYPE_LEAK ) != 0 )
		return TFALSE;

	// All the leaks of the sabotage quest are fixed
	ATreasureManager::SABOTAGE iStage = pTreasureManager->GetSabotageStage();

	if ( iStage == ATreasureManager::SABOTAGE_FIRST || iStage == ATreasureManager::SABOTAGE_SECOND )
	{
		if ( APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) )
			pControlComponent->SetUseItemCooldown( 1.0f );
	}

	if ( iStage == ATreasureManager::SABOTAGE_FIRST )
		AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPS8D( "allleaksfixed" ) );
	else if ( iStage == ATreasureManager::SABOTAGE_SECOND )
		AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPS8D( "allleaksfixed2" ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 0052eff0
static TBOOL AssignInteractionToAllUnits( const TPString8& a_rcInteraction )
{
	AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( a_rcInteraction );
	return TFALSE;
}

// Random direction around a_rcDirection (XZ) the pick ups are thrown in
static TVector4 GetThrowDirection( const TVector4& a_rcDirection )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	TFLOAT fX = rRandom.GetFloatMinMax( -1.0f, 1.0f );
	TFLOAT fZ = rRandom.GetFloatMinMax( -1.0f, 1.0f );

	TVector4 vecDirection( a_rcDirection.x + fX, a_rcDirection.y + 0.0f, a_rcDirection.z + fZ, 1.0f );
	vecDirection.Normalise();
	vecDirection.Multiply( rRandom.GetFloatMinMax( 2.0f, 3.0f ) );

	return vecDirection;
}

// $Barnyard: FUNCTION 00530190
TBOOL ATreasueHuntMicroGame::Dig()
{
	if ( m_fDigCooldown >= 0.0f )
		return TFALSE;

	AUnit*        pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	APlayerSteer* pSteer  = TSTATICCAST( APlayerSteer, pPlayer->GetSteer() );

	if ( pSteer->GetState() != APlayerSteer::STATE_WALKING )
		return TFALSE;

	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	pSteer->SetState( APlayerSteer::STATE_STILL );

	m_fDigCooldown   = 0.5f;
	m_fStillTime     = 0.5f;
	m_vecDigPosition = *pSteer->GetPosition();

	TVector4                    vecSensor = GetSensorPosition();
	ATreasureManager::Treasure* pTreasure = ATreasureManager::GetSingleton()->FindClosestTreasure( vecSensor, DIG_RADIUS );
	TBOOL                       bBigHole  = TFALSE;

	if ( pTreasure )
	{
		ATreasureManager::TYPE eType = pTreasure->eType;

		if ( ATreasureManager::GetSingleton()->RemoveTreasure( pTreasure ) )
		{
			if ( eType == ATreasureManager::TYPE_CANS || eType == ATreasureManager::TYPE_TRAPS )
			{
				TVector4 vecForward;
				vecForward.Substract( vecSensor, *pSteer->GetPosition() );
				vecForward.y = 0.0f;
				vecForward.Normalise();

				TVector4 vecDirection = GetThrowDirection( vecForward );

				TVector4 vecOffset;
				vecOffset.Multiply( TVector4::VEC_NEGY, 0.2f );

				TVector4 vecPosition;
				vecPosition.Substract( vecSensor, vecOffset );

				SpawnTreasure( ( eType == ATreasureManager::TYPE_TRAPS ) ? TPS8( trap ) : TPS8( tin ), vecPosition, vecDirection );
			}
			else if ( eType == ATreasureManager::TYPE_FLOWERS )
			{
				TPString8 aBucks[ 3 ];
				aBucks[ 0 ] = TPS8D( "buck_1" );
				aBucks[ 1 ] = TPS8D( "buck_5" );
				aBucks[ 2 ] = TPS8D( "buck_20" );

				TVector4 vecForward;
				vecForward.Substract( vecSensor, *pSteer->GetPosition() );
				vecForward.y = 0.0f;
				vecForward.Normalise();

				TINT iNumBucks = pTreasure->GetNumBucks();

				// NOTE: retail always throws out one buck less than GetNumBucks
				if ( iNumBucks > 20 )
				{
					TINT iCount = TUINT( iNumBucks - 21 ) / 20 + 1;
					iNumBucks -= 20 * iCount;

					for ( ; iCount != 0; iCount-- )
						SpawnTreasure( aBucks[ 2 ], vecSensor, GetThrowDirection( vecForward ) );
				}

				if ( iNumBucks > 5 )
				{
					TINT iCount = TUINT( iNumBucks - 6 ) / 5 + 1;
					iNumBucks -= 5 * iCount;

					for ( ; iCount != 0; iCount-- )
						SpawnTreasure( aBucks[ 1 ], vecSensor, GetThrowDirection( vecForward ) );
				}

				if ( iNumBucks > 1 )
				{
					for ( TINT iCount = iNumBucks - 1; iCount != 0; iCount-- )
						SpawnTreasure( aBucks[ 0 ], vecSensor, GetThrowDirection( vecForward ) );
				}

				bBigHole = TTRUE;

				AssignInteractionToAllUnits( TPS8( treasurefound ) );
			}
		}
	}

	m_oHoles.Spawn( GetSensorPosition(), bBigHole );
	return TTRUE;
}

// $Barnyard: FUNCTION 0052ef30
void ATreasueHuntMicroGame::SpawnTreasure( TPString8 a_strType, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	ACoinSpawner::SpawnCoin( a_strType, a_rcPosition );

	AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f, 1.0f }, 1 };
	AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
}

// $Barnyard: FUNCTION 0052e030
void ATreasueHuntMicroGame::UpdateCamera( TFLOAT a_fDeltaTime )
{
	ACameraHelper* pHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pHelper && pHelper->GetClass() == &TGetClass( AGTAStyleCameraHelper ) )
	{
		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pHelper );
		pGTAHelper->SetYaw( a_fDeltaTime * m_fCameraYaw * -100.0f + pGTAHelper->GetYaw() );
		pGTAHelper->SetPitch( a_fDeltaTime * m_fCameraPitch * 100.0f + pGTAHelper->GetPitch() );
	}

	m_fCameraYaw   = 0.0f;
	m_fCameraPitch = 0.0f;
}

// $Barnyard: FUNCTION 0052e0a0
void ATreasueHuntMicroGame::FinishUphillAnimations()
{
	AAnimatedUnit*            pPlayer    = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	AToshiAnimationInterface& rInterface = pPlayer->GetAnimatableObject()->GetToshiAnimInterface();

	if ( rInterface.IsAnimPlaying( TPS8( walk_lp_uphill ), TFALSE ) )
		pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( walk_lp_uphill ) );
	else if ( rInterface.IsAnimPlaying( TPS8( idle_lp_uphill ), TFALSE ) )
		pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( idle_lp_uphill ) );
}

//-----------------------------------------------------------------------------
// HolePool
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0052f700
void ATreasueHuntMicroGame::HolePool::Create()
{
	for ( TINT i = 0; i < MAX_NUM_HOLES; i++ )
	{
		aHoles[ i ].pModel = g_pModelRepos->InstantiateNewModel( TPS8D( "data\\models\\smallround_sod" ), TNULL );
		vecFree.PushBack( &aHoles[ i ] );
	}

	iDustEffect = AParticleEffectManager::GetSingleton()->FindEffect( "dustpuff" );
}

// $Barnyard: FUNCTION 0052e110
void ATreasueHuntMicroGame::HolePool::Destroy()
{
	for ( TINT i = 0; i < MAX_NUM_HOLES; i++ )
		g_pModelRepos->DestroyModelInstance( aHoles[ i ].pModel );

	vecFree.Clear();
	vecActive.Clear();
}

// $Barnyard: FUNCTION 0052df30
TBOOL ATreasueHuntMicroGame::Hole::Update( TFLOAT a_fDeltaTime )
{
	TVector4 vecPosition;
	pModel->GetPosition( vecPosition );

	fTime -= a_fDeltaTime;

	if ( fTime > 3.0f )
		vecPosition.y -= a_fDeltaTime * 0.32f;

	if ( fTime < 0.5f )
		vecPosition.y += a_fDeltaTime * 0.32f;

	if ( fTime < 0.0f )
	{
		TFLOAT fOpacity = pModel->GetOpacity();

		if ( fOpacity < 0.0f )
			return TFALSE;

		pModel->SetOpacity( fOpacity - a_fDeltaTime * 0.32f );
	}

	pModel->SetTranslation( vecPosition.AsVector3() );
	return TTRUE;
}

// $Barnyard: FUNCTION 0052e6e0
void ATreasueHuntMicroGame::HolePool::Update( TFLOAT a_fDeltaTime )
{
	for ( TINT i = vecActive.Size() - 1; i >= 0; i-- )
	{
		Hole* pHole = vecActive[ i ];

		if ( !pHole->Update( a_fDeltaTime ) )
		{
			vecActive.Erase( decltype( vecActive )::Iterator( i, &vecActive ) );
			vecFree.PushBack( pHole );
		}
	}
}

// $Barnyard: FUNCTION 0052e4e0
TBOOL ATreasueHuntMicroGame::HolePool::Spawn( const TVector4& a_rcPosition, TBOOL a_bBig )
{
	TVector4 vecStart = a_rcPosition;
	vecStart.y -= 0.5f;

	TFLOAT   fDistance    = 5.0f;
	TFLOAT   fStepHeight  = 1.0f;
	TFLOAT   fPenetration = 0.0f;
	TVector4 vecNormal;

	if ( !ATerrainInterface::GetSingleton()->CollideSweptSphere( vecStart, 0.25f, TVector4::VEC_POSY, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
		return TFALSE;

	TVector4 vecGround = vecStart;
	vecGround.y        = vecStart.y + fDistance + 0.4f;

	TVector4 vecDust = vecGround;
	vecDust.y -= 0.18f;
	AParticleEffectManager::GetSingleton()->PlayEffect( iDustEffect, vecDust, TVector4::VEC_NEGY );

	Hole* pHole;

	if ( vecFree.Size() != 0 )
	{
		pHole = vecFree[ vecFree.Size() - 1 ];
		vecActive.PushBack( pHole );
		vecFree.PopBack();
	}
	else
	{
		// Reuse the oldest one
		pHole = vecActive[ 0 ];
		vecActive.Erase( decltype( vecActive )::Iterator( 0, &vecActive ) );
		vecActive.PushBack( pHole );
	}

	pHole->pModel->SetOpacity( 1.0f );
	pHole->pModel->SetTranslation( vecGround.AsVector3() );
	pHole->pModel->SetSkeletonUpdating( TTRUE );

	TMatrix44 mTransform;
	pHole->pModel->GetLocalTransform( mTransform );

	if ( a_bBig )
		mTransform.Scale( 4.0f, 4.0f, 4.0f );
	else
		mTransform.Scale( 1.0f, 1.0f, 1.0f );

	pHole->pModel->SetTransform( mTransform );

	// Lie on the ground
	vecNormal.AsVector3().RotateX( TMath::PI * 0.5f );
	pHole->pModel->SetDirection( vecNormal );

	pHole->fTime = 3.5f;
	return TTRUE;
}

//-----------------------------------------------------------------------------
// PulsePool
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0052e1a0
void ATreasueHuntMicroGame::PulsePool::Create()
{
	AParticleEffectManager* pEffectManager = AParticleEffectManager::GetSingleton();
	iGreenEffect                           = pEffectManager->FindEffect( "pulsegreen" );
	iBlueEffect                            = pEffectManager->FindEffect( "pulseblue" );
	iRedEffect                             = pEffectManager->FindEffect( "pulsered" );

	for ( TINT i = 0; i < MAX_NUM_PULSES; i++ )
	{
		aPulses[ i ].pEffect = TNULL;
		vecFree.PushBack( &aPulses[ i ] );
	}
}

// $Barnyard: FUNCTION 0052e330
void ATreasueHuntMicroGame::PulsePool::Destroy()
{
	for ( TINT i = vecActive.Size() - 1; i >= 0; i-- )
		vecActive[ i ]->pEffect->GetInstance()->Stop();

	// NOTE: the active pulses don't go back to the free list
	vecFree.Clear();
	vecActive.Clear();
}

// $Barnyard: FUNCTION 0052ea10
void ATreasueHuntMicroGame::PulsePool::Update( TFLOAT a_fDeltaTime )
{
	for ( TINT i = vecActive.Size() - 1; i >= 0; i-- )
	{
		Pulse* pPulse = vecActive[ i ];
		pPulse->fTime -= a_fDeltaTime;

		if ( pPulse->fTime >= 0.0f )
		{
			// Grow
			pPulse->fScale += a_fDeltaTime * pPulse->fScaleSpeed;

			TMatrix44& rTransform = pPulse->pEffect->GetInstance()->GetTransform();
			rTransform            = pPulse->mTransform;
			rTransform.Scale( pPulse->fScale, pPulse->fScale, pPulse->fScale );
		}
		else
		{
			pPulse->pEffect->GetInstance()->Kill();
			vecActive.Erase( decltype( vecActive )::Iterator( i, &vecActive ) );
			vecFree.PushBack( pPulse );
		}
	}
}

// $Barnyard: FUNCTION 0052e750
void ATreasueHuntMicroGame::PulsePool::Spawn( const TVector4& a_rcPosition, TINT a_iUnused, TFLOAT a_fDistance, TBOOL a_bLeak )
{
	TVector4 vecGround;
	TINT     iAlpha;
	TVector3 vecRotation;
	TVector4 vecNormal;

	if ( !ATerrainInterface::GetSingleton()->FindGroundPlacement( a_rcPosition, -0.4f, vecGround, iAlpha, vecRotation, vecNormal, 2.0f, 10.0f ) )
		return;

	AUnit*                   pPlayer           = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pControlComponent )
	{
		pControlComponent->SetContextEnabled( TFALSE );
		pControlComponent->SetOverrideText( TNULL );
	}

	// 0: far, 1: at the leak, 2: close enough to dig
	TINT iLevel = 0;

	if ( a_fDistance < DETECTOR_RANGE )
	{
		if ( a_bLeak )
		{
			iLevel = a_fDistance <= 1.0f;
		}
		else if ( a_fDistance <= DIG_RADIUS )
		{
			iLevel = 2;

			if ( pControlComponent )
			{
				pControlComponent->SetOverrideText( g_pLocaleManager->GetString( LOCALE_DIG ) );
				pControlComponent->SetContextEnabled( TTRUE );
			}
		}
	}

	Pulse* pPulse;

	if ( vecFree.Size() != 0 )
	{
		pPulse = vecFree[ vecFree.Size() - 1 ];

		TINT iEffect    = ( iLevel == 0 ) ? iGreenEffect : ( iLevel == 2 ) ? iRedEffect :
		                                                                     iBlueEffect;
		pPulse->pEffect = AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

		if ( !pPulse->pEffect )
			return;

		// The textures of the rings are clamped
		AParticleEffect*          pEffect    = pPulse->pEffect->GetInstance()->GetEffect();
		AParticleMaterialLibrary* pMaterials = AParticleSystem::GetSingleton()->GetMaterialLibrary();

		for ( TINT i = 0; i < pEffect->iNumEmitters; i++ )
		{
			ASysMaterial* pMaterial = pMaterials->pMaterials[ pEffect->pEmitters[ i ].pSettings->iMaterial ].pMaterial;
			pMaterial->GetSysTexture()->SetAddressing( ADDRESSINGMODE_CLAMP );
		}

		vecActive.PushBack( pPulse );
		vecFree.PopBack();
	}
	else
	{
		// NOTE: the oldest pulse is reused with the effect it has
		pPulse = vecActive[ 0 ];
		vecActive.Erase( decltype( vecActive )::Iterator( 0, &vecActive ) );
		vecActive.PushBack( pPulse );
	}

	pPulse->fScale = 0.001f;
	TMath::Clip( a_fDistance, 0.2f, DETECTOR_RANGE );
	pPulse->fTime       = a_fDistance * 0.0277f + 0.172f;
	pPulse->fScaleSpeed = 8.0f - a_fDistance * 0.5f;

	pPulse->mTransform = TMatrix44::IDENTITY;
	pPulse->mTransform.Rotate( -vecRotation.x, vecRotation.y, vecRotation.z );
	pPulse->mTransform.SetTranslation( vecGround );

	TMatrix44& rTransform = pPulse->pEffect->GetInstance()->GetTransform();
	rTransform            = pPulse->mTransform;
	rTransform.Scale( pPulse->fScale, pPulse->fScale, pPulse->fScale );
}
