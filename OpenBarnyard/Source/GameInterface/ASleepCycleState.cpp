#include "pch.h"
#include <Render/TCameraObject.h>
#include "ASleepCycleState.h"
#include "AGameStateController.h"
#include "AStartMenuState.h"
#include "ASimState.h"
#include "Animation/AAnimatableObject.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Cameras/ALerpCameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/ARegrowthManager.h"
#include "HUD/ASimHudGroup.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ASunFlareEffect.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnitPlayer.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "Physics/APhysicsBike.h"

#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 0043fc90, CreateObjectInPlace 0043fd20 (the constructor inlined)
TDEFINE_CLASS( ASleepCycleState );

TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( yawn );
TPSTRING8_DECLARE( player_save );
TPSTRING8_DECLARE( camera_save );

// Where the lerp camera looks at the bed from while the player sleeps
static const TVector4 s_vecBedCameraPosition( 19.54f, -10.38f, -35.06f, 1.0f );
static const TVector4 s_vecBedCameraTarget( 19.86f, -11.5f, -27.63f, 1.0f );

// $Barnyard: FUNCTION 0043fb80
ASleepCycleState::ASleepCycleState()
    : m_eState( STATE_START )
    , m_fFade( 0.0f )
    , m_fPhaseTime( 0.0f )
    , m_fPhaseDuration( 0.0f )
    , m_bCameraSaved( TFALSE )
{
}

// $Barnyard: FUNCTION 0043fbc0
// $Barnyard: FUNCTION 0043fdb0
ASleepCycleState::~ASleepCycleState()
{
}

// NOTE: shared with ASleepCycleChooseState and AShakeTreeMicroGame
// $Barnyard: FUNCTION 0043fc30
TBOOL ASleepCycleState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 0043fdd0
TBOOL ASleepCycleState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_Quit )
	{
		AGameStateController::GetSingleton()->PushState( new AStartMenuState() );
		return TTRUE;
	}

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

void ASleepCycleState::SetFadeAlpha( TFLOAT a_fAlpha )
{
	// NOTE: AFade::SetFloats builds the colour from floats (black with a_fAlpha)
	AGameStateController::OverlayData oData;
	oData.uiColorA = TUINT8( a_fAlpha * 255.0f );
	oData.uiColorR = 0;
	oData.uiColorG = 0;
	oData.uiColorB = 0;

	AGameStateController::GetSingleton()->SetOverlayParams( AGameState::OVERLAY_3, oData );
}

// $Barnyard: FUNCTION 00440580
TBOOL ASleepCycleState::OnUpdate( TFLOAT a_fDeltaTime )
{
	switch ( m_eState )
	{
		case STATE_START:
			m_fFade = 0.0f;
			SetFadeAlpha( 0.0f );
			AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );
			m_eState = STATE_FADE_OUT;
			return TTRUE;
		case STATE_FADE_OUT:
			m_fFade += FADE_SPEED * a_fDeltaTime;

			if ( m_fFade >= 1.0f )
			{
				m_fFade = 1.0f;

				// Look at the bed
				ACameraManager* pCameraManager = ACameraManager::GetSingleton();
				SaveCamera();
				pCameraManager->SetCameraHelper( ACameraManager::CAMERAHELPER_LERP );

				ALerpCameraHelper* pLerpHelper = TSTATICCAST( ALerpCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_LERP ) );
				pLerpHelper->SetPosition( s_vecBedCameraPosition, s_vecBedCameraTarget );
				pLerpHelper->SetDestination( s_vecBedCameraPosition, s_vecBedCameraTarget );

				RefillPlayer();
				AInventoryGroupComponent::PutAwayHandItem();
				FreezeClosestBike();

				m_eState = STATE_FADE_IN;
			}

			SetFadeAlpha( m_fFade );
			return TTRUE;
		case STATE_FADE_IN:
			m_fFade -= FADE_SPEED * a_fDeltaTime;

			if ( m_fFade <= 0.0f )
			{
				m_fPhaseTime = 0.0f;
				m_fFade      = 0.0f;
				AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_1 );

				AGameTimeManager* pTimeManager = AGameTimeManager::GetSingleton();
				TFLOAT            fTimeLeft    = pTimeManager->GetCurrentPhaseTimeLeft();
				m_fPhaseDuration               = PHASE_DURATION;

				if ( fTimeLeft < m_fPhaseDuration )
					m_fPhaseDuration = fTimeLeft;

				// The day phases are changed by this state now
				pTimeManager->SetUpdatePhaseOfDay( TFALSE );
				AGameTimeFXManager::GetSingleton()->SetRemainingBlendTime( m_fPhaseDuration );
				m_eState = STATE_SLEEPING;
			}

			SetFadeAlpha( m_fFade );
			return TTRUE;
		case STATE_SLEEPING:
		{
			TScheduler* pScheduler = g_oSystemManager.GetScheduler();
			pScheduler->SetDebugDeltaTimeMult( TTRUE, TIME_SCALE );
			pScheduler->SetDebugDeltaTime( TFALSE, 0.0f );
			pScheduler->SetDebugSlowTime( TFALSE );

			m_fPhaseTime += a_fDeltaTime;

			if ( m_fPhaseTime <= m_fPhaseDuration )
			{
				// Move the sun
				m_fSleptTime += a_fDeltaTime;
				TFLOAT fTime     = TMath::LERP( m_fStartTime, m_fEndTime, m_fSleptTime / m_fSleepDuration );
				TUINT  uiSeconds = TUINT( TUINT64( fmod( fTime, 1200.0 ) * AGameTimeManager::GAME_TIME_TO_REAL_TIME ) );

				ASunFlareEffect::GetSingleton()->SetTime( uiSeconds / 3600, uiSeconds % 3600 / 60, TINT( uiSeconds ) % 60 );
				return TTRUE;
			}

			AGameTimeManager*          pTimeManager = AGameTimeManager::GetSingleton();
			AGameTimeManager::DAYPHASE eNextPhase   = pTimeManager->GetCurrentDayPhase() + 1;

			if ( eNextPhase >= AGameTimeManager::DAYPHASE_NUMOF )
				eNextPhase = AGameTimeManager::DAYPHASE_DAWN;

			m_fPhaseDuration = PHASE_DURATION;

			if ( eNextPhase == AGameTimeManager::DAYPHASE_DAWN || eNextPhase == AGameTimeManager::DAYPHASE_NIGHT )
			{
				// Time to wake up
				m_fFade = 0.0f;
				SetFadeAlpha( 0.0f );
				AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );

				pScheduler->SetDebugDeltaTimeMult( TFALSE, 1.0f );
				pScheduler->SetDebugDeltaTime( TFALSE, 0.0f );
				pScheduler->SetDebugSlowTime( TFALSE );

				pTimeManager->SetUpdatePhaseOfDay( TTRUE );
				m_eState = STATE_WAKE_FADE;
				pTimeManager->ChangePhaseOfDay( eNextPhase, TTRUE, TTRUE );
				return TTRUE;
			}

			AGameTimeFXManager::GetSingleton()->SetNextBlendDuration( PHASE_DURATION );
			m_fPhaseTime = 0.0f;
			pTimeManager->ChangePhaseOfDay( eNextPhase, TTRUE, TTRUE );
			return TTRUE;
		}
		case STATE_WAKE_FADE:
			m_fFade += FADE_SPEED * a_fDeltaTime;

			if ( m_fFade >= 1.0f )
			{
				RestoreCamera();

				// Face the camera
				AUnit*   pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
				ASteer*  pSteer  = pPlayer->GetSteer();
				TVector4 vecDirection( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation() );
				vecDirection.Substract( *pSteer->GetPosition() );
				vecDirection.y = 0.0f;

				TFLOAT fMagnitudeSq = vecDirection.MagnitudeSq();

				if ( fMagnitudeSq > 0.001f )
				{
					vecDirection.Divide( TMath::Sqrt( fMagnitudeSq ) );
					pSteer->SetDesiredDirection( vecDirection );
					pSteer->SetDirection( vecDirection );
				}

				TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->PlayAnimation( TPS8( yawn ) );

				// Woke up in the morning
				if ( AGameTimeManager::GetSingleton()->GetCurrentDayPhase() <= AGameTimeManager::DAYPHASE_MORNING )
					ARegrowthManager::GetSingleton()->NewDay( TFALSE, TFALSE );

				m_fFade  = 1.0f;
				m_eState = STATE_WAKE_IN;
			}

			SetFadeAlpha( m_fFade );
			return TTRUE;
		case STATE_WAKE_IN:
			m_fFade -= FADE_SPEED * a_fDeltaTime;

			if ( m_fFade > 0.0f )
			{
				SetFadeAlpha( m_fFade );
				return TTRUE;
			}

			m_fFade = 0.0f;
			AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_1 );
			m_eState = STATE_YAWNING;
			return TTRUE;
		case STATE_YAWNING:
			if ( !TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )->GetAnimatableObject()->IsAnimPlaying( TPS8( yawn ), 0.0f ) )
				m_eState = STATE_FINISHED;

			return TTRUE;
		case STATE_FINISHED:
			m_eState = STATE_NONE;
			Remove();
			return TTRUE;
		default:
			return TTRUE;
	}
}

// $Barnyard: FUNCTION 0043fe70
void ASleepCycleState::OnInsertion()
{
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME );

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) ) )
		pSimState->GetSimHudGroup()->SetShowTimer( TFALSE );

	if ( APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) )
	{
		pControlComponent->SetContextEnabled( TFALSE );
		pControlComponent->SetOverrideText( TNULL );
	}

	m_bCameraSaved = TFALSE;

	AGameTimeManager* pTimeManager = AGameTimeManager::GetSingleton();

	// Game time (0-1200) of now
	TFLOAT fSecondsPerDay = TFLOAT( AGameTimeManager::SECONDS_PER_DAY );
	TFLOAT fSeconds       = AGameTimeManager::GAME_TIME_TO_REAL_TIME * pTimeManager->GetDayTime();

	if ( fSeconds > fSecondsPerDay )
		fSeconds -= fSecondsPerDay;

	m_fEndTime   = 0.0f;
	m_fStartTime = fSeconds * AGameTimeManager::REAL_TIME_TO_GAME_TIME;

	// Sleep until the night, or until the next dawn if it's already night
	AGameTimeManager::DAYPHASE eDayPhase    = pTimeManager->GetCurrentDayPhase();
	AGameTimeManager::DAYPHASE eTargetPhase = ( TUINT( eDayPhase ) >= AGameTimeManager::DAYPHASE_NIGHT ) ? AGameTimeManager::DAYPHASE_DAWN : AGameTimeManager::DAYPHASE_NIGHT;

	for ( TINT i = 0; i <= eTargetPhase; i++ )
		m_fEndTime += AGameTimeManager::DAYPHASE_DURATION[ i ];

	if ( m_fEndTime < m_fStartTime )
		m_fEndTime += 1200.0f;

	m_fSleptTime     = 0.0f;
	m_fSleepDuration = 0.0f;

	if ( eDayPhase > eTargetPhase )
		eTargetPhase += AGameTimeManager::DAYPHASE_NUMOF;

	for ( TINT i = eDayPhase; i < eTargetPhase; i++ )
		m_fSleepDuration += PHASE_DURATION;

	TFLOAT fTimeLeft = pTimeManager->GetCurrentPhaseTimeLeft();

	// NOTE: retail subtracts the time that is left in the current phase instead of the part of it
	// that is skipped (the first phase only lasts min( PHASE_DURATION, time left ))
	if ( fTimeLeft < PHASE_DURATION )
		m_fSleepDuration -= fTimeLeft;

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00440530
void ASleepCycleState::OnRemoval()
{
	RestoreCamera();
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) ) )
		pControlComponent->SetContextEnabled( TTRUE );

	ASunFlareEffect::GetSingleton()->SetUseTime( TFALSE );

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0043fff0
void ASleepCycleState::SaveCamera()
{
	const TMatrix44& rcMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();

	m_vecCameraPos      = rcMatrix.GetTranslation();
	m_vecCameraTarget   = rcMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	m_vecCameraTarget.w = 1.0f;
	m_vecCameraTarget.x += m_vecCameraPos.x;
	m_vecCameraTarget.y += m_vecCameraPos.y;
	m_vecCameraTarget.z += m_vecCameraPos.z;

	m_bCameraSaved = TTRUE;
}

// $Barnyard: FUNCTION 00440070
void ASleepCycleState::RestoreCamera()
{
	if ( !m_bCameraSaved )
		return;

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer() );
	pSteer->PlaceAtNamedGoal( TPS8( player_save ), TTRUE, TTRUE );
	pSteer->SetTilt( 0.0f, 0.0f, TTRUE );

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	AInteriorGoal*  pGoal          = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoalInArea( TPS8( camera_save ), -1 );

	if ( pGoal )
	{
		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );

		TVector4 vecPosition;
		pGoal->GetPosition( vecPosition );
		vecPosition.y -= pGTAHelper->GetHeightOffset();

		TVector4 vecTarget = *pSteer->GetPosition();
		vecTarget.y -= pGTAHelper->GetHeightOffset();

		ALerpCameraHelper* pLerpHelper = TSTATICCAST( ALerpCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_LERP ) );
		pLerpHelper->SetPosition( vecPosition, vecTarget );
		pLerpHelper->SetDestination( vecPosition, vecTarget );
		pGTAHelper->SetPosition( vecPosition, vecTarget, TTRUE );
	}

	m_bCameraSaved = TFALSE;
}

// $Barnyard: FUNCTION 00440190
void ASleepCycleState::RefillPlayer()
{
	APlayerSimProfile* pProfile    = APlayerProfileManager::GetSingleton()->GetMainProfile();
	TFLOAT             fMaxStamina = pProfile->GetMaxStamina();

	if ( fMaxStamina < 5.0f )
	{
		pProfile->SetMaxStamina( 5.0f );
		fMaxStamina = 5.0f;
	}

	pProfile->SetStamina( fMaxStamina );

	// Refill the ammo
	AProjectileLauncher* pLauncher = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )->GetActiveLauncher();

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check if there's an active launcher
	if ( pLauncher )
#endif
	{
		ALaunchAmmo* pAmmo = pLauncher->GetAmmo();
		pAmmo->SetAmmo( pAmmo->GetMaxAmmo() );
	}

	ASimHudGroup* pHudGroup = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) )->GetSimHudGroup();
	pHudGroup->Update( 0.0f );
	pHudGroup->GetMainElement()->RefreshStaminaBars();
}

// $Barnyard: FUNCTION 00440240
void ASleepCycleState::FreezeClosestBike()
{
	ABikeManager* pBikeManager = ABikeManager::GetSingleton();

	if ( pBikeManager->IsPlayerOnBike() )
		return;

	AUnit* pBike     = TNULL;
	TINT   iBikeType = -1;
	pBikeManager->FindClosestBike( *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition(), pBike, iBikeType );

	if ( pBike && pBike->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
	{
		APhysicsBike* pPhysics = TSTATICCAST( ABikeSteer, pBike->GetSteer() )->GetPhysics();
		pPhysics->SetFrozen( TTRUE );

		ARigidBody& rBody = pPhysics->GetBody();
		rBody.m_vVelocity = TVector4::VEC_ZERO;
		rBody.SetAngularVelocity( TVector4::VEC_ZERO );
		rBody.m_vTorque = TVector4::VEC_ZERO;
		rBody.m_vForce  = TVector4::VEC_ZERO;
	}
}
