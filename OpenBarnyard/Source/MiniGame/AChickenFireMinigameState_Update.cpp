#include "pch.h"
#include "AChickenFireMinigameState.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ASSeqCameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleSystem.h"
#include "FX/AParticleEffectManager.h"
#include "FX/ASplashManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/launch.h"
#include "SoundBank/music.h"
#include "Units/AAnimatedUnit.h"
#include "ARandom.h"
#include <Math/TMathInline.h>
#include <Math/TQuaternion.h>
#include <Toshi/TStringManager.h>
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionModelInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( DialogInputMap );
TPSTRING8_DECLARE( americanm_pullback );
TPSTRING8_DECLARE( americanm_release );
TPSTRING8_DECLARE( flightDive );
TPSTRING8_DECLARE( flightTumble );
TPSTRING8_DECLARE( impact );
TPSTRING8_DECLARE( impactwobble );
TPSTRING8_DECLARE( launch );
TPSTRING8_DECLARE( launchPullBack );
TPSTRING8_DECLARE( postImpactFail );
TPSTRING8_DECLARE( prelaunch );
TPSTRING8_DECLARE( score_bad );
TPSTRING8_DECLARE( score_good );
TPSTRING8_DECLARE( slide );
TPSTRING8_DECLARE( tumble_end );
TPSTRING8_DECLARE( tumble_endhit );

// Interval of the power dial text updates (0x00773884)
static constexpr TFLOAT TEXT_UPDATE_INTERVAL = 0.2f;

// Duration of the end camera (0x007738ac)
static constexpr TFLOAT END_CAMERA_DURATION = 5.0f;

// Speed limits of Peck while diving (0x0077388c, 0x00773890, 0x00773894)
static constexpr TFLOAT MAX_FLIGHT_POWER = 3.0f;
static constexpr TFLOAT MIN_FLIGHT_SPEED = 10.0f;
static constexpr TFLOAT MAX_FLIGHT_SPEED = 60.0f;

// $Barnyard: FUNCTION 00480360
TBOOL AChickenFireMinigameState::OnUpdate( TFLOAT a_fDeltaTime )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif

	m_oTextBox.Hide();
	m_fStateTime += a_fDeltaTime;

	UpdateRings( a_fDeltaTime );

	if ( m_iVariant == 4 )
		UpdateBarnbucks( a_fDeltaTime );

	switch ( m_eState )
	{
		case STATE_WAIT_FOR_BASH:
		{
			AttachPeckToPlayer();
			PlayPeckAnimation( TPS8( prelaunch ) );

			PlayerInput& rInput = GetCurrentInput();

			if ( rInput.bBash == TTRUE )
			{
				m_eState = STATE_BASHING;
				ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_STRETCH, m_vPeckPos );
				m_pPlayerModel->PlayAnimation( TPS8( americanm_pullback ) );
				PlayPeckAnimation( TPS8( launchPullBack ) );

				m_fPower += m_fBashInc;
				rInput.bCmd57 = TFALSE;
				m_fTimer      = 4.0f;

				// NOTE: retail formats into a global buffer (0x007ce238)
				TWCHAR wszCountdown[ 16 ];
				TStringManager::String16Format( wszCountdown, TARRAYSIZE( wszCountdown ), L"%d", TMath::FloorToInt( 5.0f ) );
				m_oBashHud.SetPopText( wszCountdown );

				m_fStateTime = 0.0f;
			}

			break;
		}
		case STATE_BASHING:
			UpdateBashing( a_fDeltaTime );
			break;
		case STATE_OVERPOWER:
		case STATE_UNDERPOWER:
		{
			TVector4 vPosition;
			GetLocatorPos( vPosition, "player_start" );

			GetPeckUnit()->SetVisible( TFALSE );
			m_pPlayerModel->SetSkeletonUpdating( TFALSE, TTRUE );

			ASSeqState* pSeqState;

			if ( m_eState == STATE_OVERPOWER )
				pSeqState = new ASSeqState( "cfp_overpwr_failcase", "cfp_overpwr_failcase", "cfpcamoverpwr", vPosition, 1, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE );
			else
				pSeqState = new ASSeqState( "cfp_underpwr_failcase", "cfp_underpwr_failcase", "cfpcamundrpwr", vPosition, 1, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE );

			AGameStateController::GetSingleton()->PushState( pSeqState );

			// NOTE: ASSeqState::OnInsertion has made the sequence camera helper current
			TSTATICCAST( ASSeqCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() )->SetUseTarget( TTRUE );

			m_eState = STATE_SEQUENCE;

			// NOTE: retail returns here, skipping the common tail and AMiniGameState::OnUpdate
			return TTRUE;
		}
		case STATE_RELEASE:
		{
			AttachPeckToPlayer();

			if ( !m_pPlayerModel->IsAnimPlaying( TPS8( americanm_release ), 0.0f ) )
				m_pPlayerModel->PlayAnimation( TPS8( americanm_release ) );

			PlayPeckAnimation( TPS8( launch ) );
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_RELEASE, m_vPeckPos );

			m_eState    = STATE_RELEASING;
			m_bReleased = TFALSE;
			m_oReleaseListener.Connect( m_pReleaseEmitter->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnReleaseBreakpoint ), 0 );

			AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect2, m_vPeckPos, TVector4::VEC_POSZ );

			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			TVector4 vToPeck;
			vToPeck.Substract( m_vPeckPos, pCamera->GetMatrix().GetTranslation() );
			vToPeck.Normalise();

			TurnCamera( pCamera, vToPeck, a_fDeltaTime * 5.0f );
			break;
		}
		case STATE_RELEASING:
		{
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			TVector4 vToPeck;
			vToPeck.Substract( m_vPeckPos, pCamera->GetMatrix().GetTranslation() );
			vToPeck.Normalise();

			TurnCamera( pCamera, vToPeck, a_fDeltaTime * 5.0f );
			AttachPeckToPlayer();

			// NOTE: retail gets the animatable object of Peck here without using it
			GetPeckObject();

			if ( m_bReleased != TTRUE )
				break;

			m_bReleased = TFALSE;
			m_oReleaseListener.Disconnect();

			m_eState   = STATE_TUMBLE;
			m_vPeckDir = TVector4::VEC_NEGZ;
			m_fYawRate = 0.0f;

			PlayerInput& rInput = GetCurrentInput();
			rInput.fAimX        = TVector2::VEC_ZERO.x;
			rInput.fAimY        = TVector2::VEC_ZERO.y;

			PlayPeckAnimation( TPS8( flightTumble ) );

			m_fPower = 1.25f;
			m_oBashHud.SetForceHide( TTRUE );
			m_fTimer = 200.0f;

			if ( m_iCue_C3C == -1 )
				m_iCue_C3C = ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_FLY, 0.5f, TFALSE, 0.0f, -1 );

			break;
		}
		case STATE_UNK_7:
			AttachPeckToPlayer();

			if ( m_fStateTime > 0.5f )
			{
				m_eState = STATE_TUMBLE;

				if ( m_iCue_C3C == -1 )
					m_iCue_C3C = ASoundManager::GetSingleton()->PlayCueEx( soundbank::LAUNCH_FLY, 0.5f, TFALSE, 0.0f, -1 );
			}

			break;
		case STATE_TUMBLE:
			UpdateTumble( a_fDeltaTime );
			break;
		case STATE_FLYING:
			UpdateFlying( a_fDeltaTime );
			break;
		case STATE_CRASH:
			PlayPeckAnimation( TPS8( impact ) );

			m_fPower = 0.0f;
			m_eState = STATE_CRASHED;
			m_pFreddy->PlayAnimation( TPS8( score_bad ) );

			if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
				m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_FAIL, m_vPeckPos );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, TVector4::VEC_POSZ );

			m_fShakeRadius = 0.2f;
			break;
		case STATE_CRASHED:
			UpdateCrashed( a_fDeltaTime );
			break;
		case STATE_END_CAMERA:
			UpdateEndCamera( a_fDeltaTime );
			break;
		case STATE_END:
			AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );

			m_eState       = STATE_RESULTS;
			m_bOrbitCamera = TFALSE;

			GetHUDParams().SetFlags( 0 );
			m_oPortrait.Hide();
			m_oFeathers.SetVisible( TFALSE );

			ShowScorecard( m_iLandingScore, TVector4::VEC_ZERO );

			if ( m_iVariant == 4 )
				m_oPickUpHudElement.SetForceHide( TTRUE );

			break;
		case STATE_LAUNCH:
			if ( m_fPower > m_fUnderPowerBash )
			{
				if ( m_fPower > m_fOverPowerBash )
				{
					m_eState = STATE_OVERPOWER;
					AMusicManager::GetSingleton()->SetForcedSound( soundbank::MUSIC_OVERPOWER );
				}
				else
				{
					m_eState = STATE_RELEASE;
					AMusicManager::GetSingleton()->SetForcedSound( soundbank::MUSIC_LAUNCH );
				}
			}
			else
			{
				m_eState = STATE_UNDERPOWER;
				AMusicManager::GetSingleton()->SetForcedSound( soundbank::MUSIC_UNDERPOWER );
			}

			AMusicManager::GetSingleton()->ForceUpdate();
			break;
		case STATE_DIVE:
			UpdateDive( a_fDeltaTime );
			break;
		case STATE_SPLASHED:
			HideHud();
			m_fTimer += a_fDeltaTime;

			if ( ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
			{
				if ( m_bOrbitCamera == TTRUE )
					OrbitCamera( a_fDeltaTime );
			}
			else
			{
				StopSounds( TTRUE );
				SetInputMap( TPS8( DialogInputMap ) );
				AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );

				m_eState = STATE_RESULTS;
				StopFlightFeathers();
				m_bOrbitCamera = TFALSE;

				if ( m_iVariant == 4 )
					m_oPickUpHudElement.SetForceHide( TTRUE );
			}

			break;
		case STATE_FALLING:
			m_fTimer += a_fDeltaTime;

			if ( m_fTimer > 3.0f )
			{
				if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C60 ) )
				{
					StopSounds( TTRUE );
					m_fTimer = 0.0f;
					m_fPower = 0.5f;
					AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );

					if ( m_iVariant == 4 )
						m_oPickUpHudElement.SetForceHide( TTRUE );

					m_eState = STATE_RESULTS;
				}
			}
			else
			{
				TVector4 vDiveDir;
				GetDiveDirection( vDiveDir );

				TurnDirection( m_vPeckDir, vDiveDir, a_fDeltaTime * 0.25f );
				UpdateFlight( a_fDeltaTime );
			}

			break;
		case STATE_RESULTS:
			if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
			{
				if ( m_bOrbitCamera == TTRUE )
					OrbitCamera( a_fDeltaTime );

				ShowScorecard( m_iLandingScore, TVector4::VEC_ZERO );
			}
			else
			{
				AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );
				AttachPeckToPlayer();

				TINT iCurrentPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerIterator();

				// Next player's turn or the end of the game
				if ( m_iNumPlayers2 <= 1 || iCurrentPlayer >= m_iNumPlayers2 - 1 )
					ResetTurn( TFALSE );
				else
					ResetTurn( TTRUE );
			}

			break;
		case STATE_DESCEND:
			UpdateDescend( a_fDeltaTime );
			break;
		case STATE_WAIT_FOR_HUD:
			if ( m_oStartHudElement.IsForceHidden() )
				m_eState = STATE_WAIT_FOR_BASH;

			AttachPeckToPlayer();
			PlayPeckAnimation( TPS8( prelaunch ) );
			break;
		case STATE_ROLLING:
			UpdateRolling( a_fDeltaTime );
			break;
		case STATE_STOPPED:
			HideHud();

			if ( m_iVariant == 4 )
				m_oPickUpHudElement.SetForceHide( TTRUE );

			m_fTimer += a_fDeltaTime;

			if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
			{
				StopSounds( TTRUE );
				SetInputMap( TPS8( DialogInputMap ) );
				AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );

				m_eState = STATE_RESULTS;
				StopFlightFeathers();
				m_bOrbitCamera = TFALSE;
			}

			break;
	}

	m_oBashHud.m_fValue = m_fPower;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		m_aInputs[ i ].bBash = TFALSE;
		m_aInputs[ i ].fAimX = TVector2::VEC_ZERO.x;
		m_aInputs[ i ].fAimY = TVector2::VEC_ZERO.y;
	}

	AInputHandler* pInputHandler = AInputHandler::GetSingleton();
	pInputHandler->SetRumblePosition( pInputHandler->GetMainController(), m_vPeckPos );

	return AMiniGameState::OnUpdate( a_fDeltaTime );
}

// STATE_BASHING
void AChickenFireMinigameState::UpdateBashing( TFLOAT a_fDeltaTime )
{
	PlayerInput& rInput = GetCurrentInput();

	// NOTE: OnUpdate has already added the delta time, so it is added twice here
	if ( rInput.bBash != TFALSE )
		m_fStateTime = 0.0f;
	else
		m_fStateTime += a_fDeltaTime;

	AAnimatableObject* pPeckObject = GetPeckObject();

	// Countdown
	m_fTimer -= a_fDeltaTime;

	if ( m_iCountdown != TMath::FloorToInt( m_fTimer ) )
	{
		m_iCountdown = TMath::FloorToInt( m_fTimer );
		m_iCue_C44   = ASoundManager::GetSingleton()->PlayCue( ( m_iCountdown == -1 ) ? soundbank::LAUNCH_LAUNCH_BEEP02 : soundbank::LAUNCH_LAUNCH_BEEP01 );
	}

	if ( m_fTimer <= 0.0f )
	{
		m_eState = STATE_LAUNCH;

		if ( m_iCue_C4C != -1 )
		{
			ASoundManager::GetSingleton()->StopCue( m_iCue_C4C );
			m_iCue_C4C = -1;
		}

		return;
	}

	// NOTE: retail formats into a global buffer (0x007ce238)
	TWCHAR wszCountdown[ 16 ];
	TStringManager::String16Format( wszCountdown, TARRAYSIZE( wszCountdown ), L"%d", TMath::FloorToInt( m_fTimer + 1.0f ) );
	m_oBashHud.SetPopText( wszCountdown );

	AttachPeckToPlayer();

	// Bashing adds power, it drains over time
	if ( rInput.bBash == TTRUE )
	{
		m_fPower += m_fBashInc;
		rInput.bCmd57 = TFALSE;

		if ( m_fPower > 0.5f && !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C4C ) )
			m_iCue_C4C = ASoundManager::GetSingleton()->PlayCue( soundbank::LAUNCH_COW_GRUNT );
	}

	m_fPower -= a_fDeltaTime * m_fBashRed;
	m_fPower = TMath::Min( m_fPower, 1.0f );
	m_fPower = TMath::Max( m_fPower, 0.0f );

	UpdatePullbackAnimation( m_pPlayerModel, TPS8( americanm_pullback ), m_fPlayerPullbackDuration, m_fPlayerPullbackTime, a_fDeltaTime );
	UpdatePullbackAnimation( pPeckObject, TPS8( launchPullBack ), m_fPeckPullbackDuration, m_fPeckPullbackTime, a_fDeltaTime );

	const TWCHAR* wszText = TNULL;

	if ( m_fPower >= m_fOverPowerBash )
		wszText = ALocaleManager::Interface()->GetString( LOCALE_POWER_HIGH );
	else if ( m_fPower <= m_fUnderPowerBash )
		wszText = ALocaleManager::Interface()->GetString( LOCALE_POWER_LOW );
	else if ( m_fPower >= m_fUnderPowerBash && m_fPower < m_fOverPowerBash )
		wszText = ALocaleManager::Interface()->GetString( LOCALE_POWER_GOOD );

	m_fTextTimer += a_fDeltaTime;

	if ( m_fTextTimer >= TEXT_UPDATE_INTERVAL )
	{
		// NOTE: retail also calls two DOES_NOTHING (00628f40) methods of the bash hud here with 0 and -1
		m_oBashHud.SetText( wszText );
		m_fTextTimer = 0.0f;
	}
}

// Plays the pull back animation forwards while bashing and backwards when the player stops (inlined in OnUpdate)
void AChickenFireMinigameState::UpdatePullbackAnimation( AAnimatableObject* a_pObject, const TPString8& a_rcName, TFLOAT& a_rfDuration, TFLOAT& a_rfTime, TFLOAT a_fDeltaTime )
{
	if ( a_pObject->IsAnimPlaying( a_rcName, 0.0f ) != TTRUE )
		return;

	if ( a_rfDuration == -1.0f )
		a_rfDuration = a_pObject->GetAnimationDuration( a_rcName );

	a_rfTime = a_pObject->GetAnimationTime( a_rcName );

	if ( m_fStateTime == 0.0f )
		a_rfTime += a_fDeltaTime;
	else if ( m_fStateTime >= 0.35f )
		a_rfTime -= a_fDeltaTime + a_fDeltaTime;

	TMath::Clip( a_rfTime, 0.0f, a_rfDuration - 0.001f );
	a_pObject->SetAnimationTime( a_rcName, a_rfTime );
}

// STATE_TUMBLE
void AChickenFireMinigameState::UpdateTumble( TFLOAT a_fDeltaTime )
{
	UpdateFlight( a_fDeltaTime );
	m_fTimer -= a_fDeltaTime;

	if ( CheckFlight( a_fDeltaTime ) == TTRUE )
		return;

	ACamera* pCamera    = ACameraManager::GetSingleton()->GetCurrentCamera();
	TVector4 vCameraPos = pCamera->GetMatrix().GetTranslation();

	TurnCamera( pCamera, m_vPeckDir, a_fDeltaTime * 5.0f );

	// Follow Peck at a distance of 3
	TVector4 vToPeck;
	vToPeck.Substract( m_vPeckPos, vCameraPos );

	TFLOAT fDistance = vToPeck.Magnitude();
	vToPeck.Normalise();

	if ( fDistance > 3.0f )
	{
		TVector4 vUp;
		vUp.Multiply( TVector4::VEC_NEGY, 1.5f );
		vUp.Multiply( 0.1f );

		TVector4 vOffset;
		vOffset.Multiply( vToPeck, fDistance - 3.0f );
		vOffset.Add( vOffset, vUp );

		vCameraPos.Add( vOffset );
	}

	pCamera->GetMatrix().GetTranslation() = vCameraPos;
	pCamera->SetFOV( 1.3089969f );

	if ( GetPeckObject()->IsAnimationFinished( TPS8( flightTumble ) ) || m_fTimer <= 0.0f )
	{
		m_eState   = STATE_FLYING;
		m_fTimer   = 0.0f;
		m_vUnk_BEC = TVector4::VEC_ZERO;

		if ( m_iVariant == 4 )
			SetCharityActorsSkeletonUpdating( TFALSE );
	}

	TFLOAT fUnused;
	CheckPeckCollision( a_fDeltaTime, &fUnused );
}

// STATE_FLYING
void AChickenFireMinigameState::UpdateFlying( TFLOAT a_fDeltaTime )
{
	m_fFlightTime += a_fDeltaTime;

	// Boost
	if ( m_iNumBoosts > 0 && GetCurrentInput().bBash == TTRUE )
	{
		m_bBoosted = TTRUE;
		m_iNumBoosts--;
		m_iNumBoosts = TMath::Max( m_iNumBoosts, 0 );
		m_oFeathers.SetNumFeathers( m_iNumBoosts );

		if ( m_pFlightFeathersL == TNULL )
		{
			m_pFlightFeathersL = AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherFlightEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( m_pFlightFeathersL )
				m_pFlightFeathersL->AttachToBone( m_pPeckModelInstance, m_iPeckLeftForearmBone, TTRUE, TFALSE );
		}

		if ( m_pFlightFeathersR == TNULL )
		{
			m_pFlightFeathersR = AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherFlightEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( m_pFlightFeathersR )
				m_pFlightFeathersR->AttachToBone( m_pPeckModelInstance, m_iPeckRightForearmBone, TTRUE, TFALSE );
		}

		if ( ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C5C ) == TTRUE )
			ASoundManager::GetSingleton()->StopCue( m_iCue_C5C );

		m_iCue_C5C = ASoundManager::GetSingleton()->PlayCueAtPositionEx( soundbank::LAUNCH_BOOST, m_vPeckPos, 0.5f, TFALSE, 0.0f, -1 );
	}

	// The feathers fly out behind Peck
	TVector4 vBackwards;
	vBackwards.Negate3( m_vPeckDir );

	if ( m_pFlightFeathersL )
		m_pFlightFeathersL->GetInstance()->GetTransform().LookAtDirection( vBackwards, TVector4::VEC_NEGY );

	if ( m_pFlightFeathersR )
		m_pFlightFeathersR->GetInstance()->GetTransform().LookAtDirection( vBackwards, TVector4::VEC_NEGY );

	if ( CheckFlight( a_fDeltaTime ) == TTRUE )
		return;

	// NOTE: retail creates a TPString8 "board" here and never uses it

	TFLOAT fHitDistance = -1.0f;
	TINT   iResult      = CheckPeckCollision( a_fDeltaTime, &fHitDistance );

	if ( iResult == HIT_BOARD )
	{
		OnLandedOnBoard( fHitDistance );
	}
	else if ( iResult == HIT_BARN )
	{
		OnLandedOnBarn( fHitDistance );
	}
	else if ( iResult >= HIT_SUCCESS && iResult <= HIT_SUCCESS_LAST )
	{
		OnLandedInRing( iResult );
	}
	else if ( iResult == HIT_NONE )
	{
		if ( m_vTargetPoint.z - 1.0f >= m_vPeckPos.z )
			EndTurn();

		UpdateFlight( a_fDeltaTime );
		UpdateFlightCamera( a_fDeltaTime );

		if ( ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C60 ) == TTRUE )
			ASoundManager::GetSingleton()->SetCuePosition( m_iCue_C60, m_vPeckPos, 0.0f );

		// Peck shouts now and then
		m_fTimer += a_fDeltaTime;

		if ( m_fTimer >= 4.0f )
		{
			m_fTimer = 0.0f;

			if ( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 1.0f ) >= 0.9f )
				m_iCue_C60 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_INFLIGHT1, m_vPeckPos );
		}
	}
}

// STATE_CRASHED
void AChickenFireMinigameState::UpdateCrashed( TFLOAT a_fDeltaTime )
{
	m_fEndTimer += a_fDeltaTime;

	TFLOAT fProgress = a_fDeltaTime * 0.5f;
	m_fShakeRadius   = TMath::Max( m_fShakeRadius - fProgress, 0.0f );

	if ( m_fShakeRadius <= 0.0f )
	{
		// Slide away from the target
		TMatrix44 matTarget;
		ATerrainInterface::GetSingleton()->GetLocatorTransform( "target_point00", matTarget );

		PlayPeckAnimation( TPS8( slide ) );

		TVector4 vUp( -matTarget.m_f21, -matTarget.m_f22, -matTarget.m_f23, matTarget.m_f24 );
		TVector4 vSlideDir( -matTarget.m_f31, -matTarget.m_f32, -matTarget.m_f33, matTarget.m_f34 );

		TFLOAT fDistance = 1.0f;
		ATerrainInterface::GetSingleton()->CollideRay( m_vPeckPos, vSlideDir, fDistance, 0, TTRUE );

		if ( fDistance == 1.0f )
		{
			// 0x00782860
			static TFLOAT s_fSlideSpeed = 0.0f;
			s_fSlideSpeed               = TMath::LERP( s_fSlideSpeed, 0.5f, fProgress );

			TVector4 vMove;
			vMove.Multiply( vSlideDir, a_fDeltaTime );
			vMove.Multiply( s_fSlideSpeed );
			m_vPeckPos.Add( vMove );

			vUp.w = 1.0f;
			SetPeckTransform( m_vPeckPos, vUp );
		}
	}
	else
	{
		// Camera shake
		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		TVector4 vCameraPos = pCamera->GetMatrix().GetTranslation();
		AddRandomOffset( vCameraPos, m_fShakeRadius );
		pCamera->GetMatrix().GetTranslation() = vCameraPos;
		pCamera->LookAtPoint( m_vPeckPos );
	}

	if ( END_CAMERA_DURATION * 0.5f <= m_fEndTimer )
	{
		m_fEndTimer = 0.0f;
		m_eState    = STATE_END_CAMERA;

		m_pFreddy->PlayAnimation( ( m_iLandingScore < 7 ) ? TPS8( score_bad ) : TPS8( score_good ) );
		PlayPeckAnimation( TPS8( postImpactFail ) );

		TVector4    vStaggerPos;
		TQuaternion quatStaggerRot;
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "peck_stagger", vStaggerPos, quatStaggerRot );
		SetPeckTransform( vStaggerPos, TVector4::VEC_POSZ );

		ASoundManager::GetSingleton()->PlayCue( soundbank::LAUNCH_CLAP );

		AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();
		pFXManager->SetForcedFX( pFXManager->FindFX( "PeckRange" ) );
	}
}

// STATE_END_CAMERA
void AChickenFireMinigameState::UpdateEndCamera( TFLOAT a_fDeltaTime )
{
	HideHud();
	m_fEndTimer += a_fDeltaTime;

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->SetFOV( 1.0471976f );

	// NOTE: the hardcoded camera is replaced by the locators right away
	TVector4 vCameraPos( 4.2739711f, -11.800001f, -193.08322f, 1.0f );
	TVector4 vCameraDir( -0.420977f, 0.248748f, -0.87229699f, 1.0f );
	vCameraDir.Normalise();

	pCamera->GetMatrix().GetTranslation() = vCameraPos;
	pCamera->LookAtDirection( vCameraDir );

	vCameraPos             = ATerrainInterface::GetSingleton()->GetLocatorPos( "end_cam_pos" );
	TVector4 vCameraLookAt = ATerrainInterface::GetSingleton()->GetLocatorPos( "end_cam_lookat" );

	pCamera->GetMatrix().GetTranslation() = vCameraPos;
	pCamera->LookAtPoint( vCameraLookAt );

	ShowScorecard( m_iLandingScore, TVector4::VEC_ZERO );

	if ( m_bEndCameraStarted == TFALSE )
		m_bEndCameraStarted = TTRUE;

	if ( GetCurrentInput().bBash == TTRUE || m_fEndTimer >= END_CAMERA_DURATION )
	{
		m_fEndTimer = 0.0f;
		m_eState    = STATE_END;
	}
}

// STATE_DIVE
void AChickenFireMinigameState::UpdateDive( TFLOAT a_fDeltaTime )
{
	m_fFlightTime += a_fDeltaTime;

	TVector4 vDiveDir;
	GetDiveDirection( vDiveDir );
	TurnDirection( m_vPeckDir, vDiveDir, a_fDeltaTime + a_fDeltaTime );

	PlayPeckAnimation( TPS8( flightDive ) );
	MoveForward( a_fDeltaTime );

	if ( TVector4::DotProduct3( m_vPeckDir, TVector4::VEC_POSY ) < 0.75f || m_fPower < 0.3f )
	{
		TFLOAT fHitDistance = -1.0f;
		TINT   iResult      = CheckPeckCollision( a_fDeltaTime, &fHitDistance );

		if ( iResult == HIT_BOARD )
		{
			OnLandedOnBoard( fHitDistance );
		}
		else if ( iResult >= HIT_SUCCESS && iResult <= HIT_SUCCESS_LAST )
		{
			OnLandedInRing( iResult );
		}
		else if ( iResult == HIT_NONE && m_vTargetPoint.z - 1.0f >= m_vPeckPos.z )
		{
			EndTurn();
		}
	}
	else
	{
		m_eState     = STATE_FLYING;
		m_fBankAngle = 0.0f;
		m_fPitchRate = 0.0f;
		m_fYawRate   = 0.0f;
	}
}

// STATE_DESCEND
void AChickenFireMinigameState::UpdateDescend( TFLOAT a_fDeltaTime )
{
	TVector4 vDiveDir;
	GetDiveDirection( vDiveDir );
	TurnDirection( m_vPeckDir, vDiveDir, a_fDeltaTime + a_fDeltaTime );

	MoveForward( a_fDeltaTime );

	// Look for what is ahead of Peck
	TFLOAT   fMaxDistance = 2.0f;
	TVector4 vRayDir      = m_vPeckDir;
	vRayDir.Normalise();

	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideRay( m_vPeckPrevPos, vRayDir, fMaxDistance, &oHitList, 0, TTRUE );

	TFLOAT fHitDistance = 50.0f;

	if ( oHitList.iNumHits <= 0 )
		return;

	TINT iClosest = FindClosestHit( oHitList, fHitDistance );

	if ( iClosest <= -1 )
		return;

	ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosest ];
	TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );
	TModelCollisionGroup&           rGroup    = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) );

	if ( rGroup.eCollGroup == TCollisionCommon::TOSHICGROUP_WATER2 )
	{
		LandInWater( fHitDistance, TTRUE );
		return;
	}

	if ( rGroup.strName.Find( "coll", 0 ) < 0 )
		return;

	// Peck hit an obstacle
	m_eState = STATE_SPLASHED;
	PlayPeckAnimation( TPS8( impactwobble ) );
	StopFlightFeathers();

	m_oTextBox.Hide();
	StopSounds( TFALSE );

	if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) )
		m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_PECK_IMPACT, m_vPeckPos );

	// Average the normals of the triangles around the hit point that face Peck
	TVector4 vSphere;
	TVector4 vMove;
	vMove.Multiply( m_vPeckDir, fHitDistance );
	vSphere.Add( m_vPeckPrevPos, vMove );
	vSphere.w = 0.5f;

	ACollisionModelRayHitList oSphereHits;
	ATerrainInterface::GetSingleton()->CollideSphere( vSphere, &oSphereHits, -1, 0 );

	TVector4 vNormal = TVector4::VEC_ZERO;

	for ( TINT i = 0; i < oSphereHits.iNumHits; i++ )
	{
		TVector4 vHitNormal = oSphereHits.aHits[ i ].GetNormal();

		if ( TVector4::DotProduct3( vHitNormal, m_vPeckDir ) <= 0.0f )
		{
			vNormal.x = vHitNormal.x + vNormal.x;
			vNormal.y = vHitNormal.y + vNormal.y;
			vNormal.z = vHitNormal.z + vNormal.z;
		}
	}

	if ( TVector4::DotProduct3( vNormal, TVector4::VEC_NEGY ) >= 0.9f )
		vNormal.AsVector3().RotateX( TMath::DegToRad( 1.0f ) );

	vNormal.Normalise();

	AParticleEffectManager::GetSingleton()->PlayEffect( m_iFeatherExplodeEffect, m_vPeckPos, vNormal );
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, m_vPeckPos, vNormal );

	vNormal.Negate3();

	// Bounce back off the obstacle
	TVector4 vBack;
	vBack.Multiply( vNormal, 0.2f );

	vMove.Multiply( m_vPeckDir, fHitDistance );
	vMove.Multiply( 0.75f );

	TVector4 vNewPos;
	vNewPos.Add( m_vPeckPos, vMove );
	m_vPeckPos.Substract( vNewPos, vBack );

	SetPeckTransform( m_vPeckPos, vNormal );

	m_fTimer = 0.0f;
	Rumble();
}

// STATE_ROLLING
void AChickenFireMinigameState::UpdateRolling( TFLOAT a_fDeltaTime )
{
	m_fTimer += a_fDeltaTime;

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	TVector4 vNormal;
	TFLOAT   fStepHeight;
	TFLOAT   fPenetration;
	TFLOAT   fGroundDistance = 5.0f;

	if ( m_fTimer > 2.0f ||
	     !pTerrain->CollideSweptSphere( m_vPeckPos, 0.5f, TVector4::VEC_POSY, fGroundDistance, vNormal, 0.0f, fStepHeight, fPenetration, 0 ) ||
	     TMath::Abs( m_vPeckPos.x ) > 40.0f )
	{
		PlayPeckAnimation( TPS8( tumble_end ) );
		m_eState = STATE_STOPPED;
		m_fTimer = 0.0f;
		return;
	}

	// Rolled into the water?
	ACollisionModelRayHitList oHitList;
	TFLOAT                    fMaxDistance = 2.0f;
	pTerrain->CollideRay( m_vPeckPos, TVector4::VEC_POSY, fMaxDistance, &oHitList, 0, TTRUE );

	TFLOAT fHitDistance = 50.0f;

	if ( oHitList.iNumHits > 0 )
	{
		TINT iClosest = FindClosestHit( oHitList, fHitDistance );

		if ( iClosest > -1 )
		{
			ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosest ];
			TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );

			if ( pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup == TCollisionCommon::TOSHICGROUP_WATER2 )
			{
				LandInWater( fHitDistance, TFALSE );
				return;
			}
		}
	}

	// Gravity along the slope
	TVector4 vGravity;
	vGravity.Multiply( TVector4::VEC_POSY, 9.8f );

	TVector4 vGravityOnNormal = vGravity;
	vGravityOnNormal.ProjectOnto( vNormal );
	vGravity.Substract( vGravityOnNormal );

	TVector4 vRollDir;
	vRollDir.Multiply( m_vPeckDir, 5.0f );

	TVector4 vAccel;
	vAccel.Multiply( vGravity, a_fDeltaTime );
	vRollDir.Add( vAccel );
	vRollDir.Normalise();

	TFLOAT fProgress = a_fDeltaTime * 5.0f;
	TFLOAT fDistance = fProgress;

	if ( pTerrain->CollideSweptSphere( m_vPeckPos, 0.5f, vRollDir, fDistance, vNormal, 0.0f, fStepHeight, fPenetration, 0 ) == TTRUE )
	{
		if ( TMath::Abs( TVector4::DotProduct3( vNormal, TVector4::VEC_NEGY ) ) < 0.1f )
		{
			// Hit a wall
			PlayPeckAnimation( TPS8( tumble_endhit ) );

			m_eState = STATE_STOPPED;
			m_fTimer = 0.0f;

			TVector4 vMove;
			vMove.Multiply( vRollDir, fDistance );
			vMove.Multiply( 0.9f );
			m_vPeckPos.Add( vMove );

			SnapToGround();
			SetPeckTransform( m_vPeckPos, m_vPeckDir );
			FollowWithCamera( fProgress );
			return;
		}

		// Slide along the surface, give up after 5 tries
		TINT  iNumTries = 0;
		TBOOL bHit;

		while ( TTRUE )
		{
			TVector4 vDirOnNormal = m_vPeckDir;
			vDirOnNormal.ProjectOnto( vNormal );
			m_vPeckDir.Substract( vDirOnNormal );
			m_vPeckDir.Normalise();

			vRollDir.Multiply( m_vPeckDir, 5.0f );
			vAccel.Multiply( vGravity, a_fDeltaTime );
			vRollDir.Add( vAccel );
			vRollDir.Normalise();

			fDistance = fProgress;
			bHit      = pTerrain->CollideSweptSphere( m_vPeckPos, 0.5f, vRollDir, fDistance, vNormal, 0.0f, fStepHeight, fPenetration, 0 );

			if ( ++iNumTries == 5 )
				break;

			if ( bHit != TTRUE )
				break;
		}

		if ( iNumTries == 5 )
		{
			// Stuck
			PlayPeckAnim( PECKANIM_TUMBLE_ENDHIT );

			m_eState = STATE_STOPPED;
			m_fTimer = 0.0f;

			TVector4 vMove;
			vMove.Multiply( vRollDir, fDistance );
			vMove.Multiply( 0.9f );
			m_vPeckPos.Add( vMove );

			SnapToGround();
			SetPeckTransform( m_vPeckPos, m_vPeckDir );
			FollowWithCamera( fProgress );

			// NOTE: retail falls through and moves Peck once more
		}
	}

	TVector4 vMove;
	vMove.Multiply( vRollDir, 5.0f );
	vMove.Multiply( a_fDeltaTime );
	m_vPeckPos.Add( vMove );

	SnapToGround();

	m_vPeckDir = vRollDir;
	SetPeckTransform( m_vPeckPos, m_vPeckDir );
	FollowWithCamera( fProgress );
}

//-----------------------------------------------------------------------------
// Helpers (inlined or repeated in OnUpdate)
//-----------------------------------------------------------------------------

AAnimatedUnit* AChickenFireMinigameState::GetPeckUnit() const
{
	return m_oPeckProxy ? TSTATICCAST( AAnimatedUnit, m_oPeckProxy->GetUnit() ) : TNULL;
}

AAnimatableObject* AChickenFireMinigameState::GetPeckObject() const
{
	return GetPeckUnit()->GetAnimatableObject();
}

void AChickenFireMinigameState::PlayPeckAnimation( const TPString8& a_rcName )
{
	AAnimatableObject* pPeckObject = GetPeckObject();

	if ( !pPeckObject->IsAnimPlaying( a_rcName, 0.0f ) )
		pPeckObject->PlayAnimation( a_rcName );
}

// Turns a_rDirection towards a_rcTarget (slerp of the look at rotations)
void AChickenFireMinigameState::TurnDirection( TVector4& a_rDirection, const TVector4& a_rcTarget, TFLOAT a_fProgress )
{
	TMatrix44 matFrom;
	TMatrix44 matTo;
	matFrom.LookAtDirection( a_rDirection, TVector4::VEC_NEGY );
	matTo.LookAtDirection( a_rcTarget, TVector4::VEC_NEGY );

	TQuaternion quatFrom;
	TQuaternion quatTo;
	TMatrix44::MatToQuat( quatFrom, matFrom );
	TMatrix44::MatToQuat( quatTo, matTo );

	TQuaternion quatRotation;
	TQuaternion::Slerp( quatRotation, quatFrom, quatTo, a_fProgress );

	RotateVector( a_rDirection, quatRotation, TVector4::VEC_POSZ );
	a_rDirection.Normalise();
}

// Turns the camera towards a_rcTarget
void AChickenFireMinigameState::TurnCamera( ACamera* a_pCamera, const TVector4& a_rcTarget, TFLOAT a_fProgress )
{
	TVector4 vDirection;
	a_pCamera->GetDirection( vDirection );
	vDirection.w = 1.0f;

	TurnDirection( vDirection, a_rcTarget, a_fProgress );

	vDirection.w = 1.0f;
	a_pCamera->LookAtDirection( vDirection );
}

// VEC_POSY rotated by -1 degree around the right axis of Peck
void AChickenFireMinigameState::GetDiveDirection( TVector4& a_rOutDirection )
{
	TMatrix44 matPeck;
	GetPeckObject()->GetModelInstance()->GetLocalTransform( matPeck );

	TVector4 vAxis( matPeck.m_f11, matPeck.m_f12, matPeck.m_f13, matPeck.m_f14 );

	// NOTE: retail also makes a normalised copy of the axis without its y but never uses it
	TVector4 vFlatAxis = vAxis;
	vFlatAxis.y        = 0.0f;
	vFlatAxis.Normalise();

	a_rOutDirection = TVector4::VEC_POSY;

	TQuaternion quatRotation = TQuaternion::IDENTITY;
	quatRotation.RotateAroundAxis( vAxis.AsVector3(), -TMath::DegToRad( 1.0f ) );
	RotateVector( a_rOutDirection, quatRotation, a_rOutDirection );
}

// Accelerates Peck and moves it along m_vPeckDir
void AChickenFireMinigameState::MoveForward( TFLOAT a_fDeltaTime )
{
	m_fPower = m_fPeckBoostAccel * m_fPeckBoostAccel * a_fDeltaTime * 0.5f + m_fPower;
	TMath::Clip( m_fPower, ms_fMinFlightSpeed, MAX_FLIGHT_POWER );

	TFLOAT fSpeed = m_fPower * 30.0f;
	TMath::Clip( fSpeed, MIN_FLIGHT_SPEED, MAX_FLIGHT_SPEED );

	m_fFlightStep  = fSpeed * a_fDeltaTime;
	m_vPeckPrevPos = m_vPeckPos;

	TVector4 vMove;
	vMove.Multiply( m_vPeckDir, m_fFlightStep );
	m_vPeckPos.Add( vMove );

	SetPeckFlightTransform( m_vPeckPos, m_vPeckDir );
	UpdateFlightCamera( a_fDeltaTime );
}

void AChickenFireMinigameState::StopFlightFeathers()
{
	if ( m_pFlightFeathersL )
	{
		m_pFlightFeathersL->GetInstance()->Stop();
		m_pFlightFeathersL = TNULL;
	}

	if ( m_pFlightFeathersR )
	{
		m_pFlightFeathersR->GetInstance()->Stop();
		m_pFlightFeathersR = TNULL;
	}
}

void AChickenFireMinigameState::HideHud()
{
	GetHUDParams().SetFlags( 0 );
	m_oPortrait.Hide();
	m_oFeathers.SetVisible( TFALSE );
}

// Rotates the camera around Peck
void AChickenFireMinigameState::OrbitCamera( TFLOAT a_fDeltaTime )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->LookAtPoint( m_vPeckPos );

	TVector4 vOffset;
	vOffset.Substract( pCamera->GetMatrix().GetTranslation(), m_vPeckPos );
	vOffset.AsVector3().RotateY( a_fDeltaTime * 0.1f );

	TVector4 vCameraPos;
	vCameraPos.Add( m_vPeckPos, vOffset );
	pCamera->GetMatrix().GetTranslation() = vCameraPos;
}

// Moves the camera behind and above Peck
void AChickenFireMinigameState::FollowWithCamera( TFLOAT a_fProgress )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TVector4 vDir = m_vPeckDir;
	vDir.y        = 0.0f;
	vDir.Normalise();

	TVector4 vUp;
	vUp.Multiply( TVector4::VEC_NEGY, 3.0f );

	TVector4 vBehind;
	vBehind.Multiply( vDir, 3.0f );

	TVector4 vTarget;
	vTarget.Substract( m_vPeckPos, vBehind );
	vTarget.Add( vTarget, vUp );

	TVector4 vCameraPos = pCamera->GetMatrix().GetTranslation();
	vCameraPos.Lerp4( vCameraPos, vTarget, a_fProgress );

	pCamera->GetMatrix().GetTranslation() = vCameraPos;
	pCamera->LookAtPoint( m_vPeckPos );
}

// Puts Peck on the ground below it
void AChickenFireMinigameState::SnapToGround()
{
	TVector4 vNormal;
	TFLOAT   fStepHeight;
	TFLOAT   fPenetration;
	TFLOAT   fDistance = 10.0f;

	if ( ATerrainInterface::GetSingleton()->CollideSweptSphere( m_vPeckPos, 0.5f, TVector4::VEC_POSY, fDistance, vNormal, 0.0f, fStepHeight, fPenetration, 0 ) )
		m_vPeckPos.y = fDistance + m_vPeckPos.y;
}

// Peck falls into the water
void AChickenFireMinigameState::LandInWater( TFLOAT a_fHitDistance, TBOOL a_bHideText )
{
	GetPeckUnit()->SetVisible( TFALSE );

	m_eState = STATE_SPLASHED;
	m_fTimer = 0.0f;
	StopFlightFeathers();

	if ( a_bHideText )
		m_oTextBox.Hide();

	StopSounds( TFALSE );

	m_iCue_C58 = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::LAUNCH_SPLASH, m_vPeckPos );

	TVector4 vMove;
	vMove.Multiply( m_vPeckDir, a_fHitDistance );
	vMove.Multiply( 0.95f );

	TVector4 vSplashPos;
	vSplashPos.Add( m_vPeckPrevPos, vMove );

	AParticleEffectManager* pEffectManager = AParticleEffectManager::GetSingleton();
	pEffectManager->PlayEffect( m_iWaterSplashAEffect, vSplashPos, TVector4::VEC_NEGY );
	pEffectManager->PlayEffect( m_iWaterSplashBEffect, vSplashPos, TVector4::VEC_NEGY );
	pEffectManager->PlayEffect( m_iWaterRippleEffect, vSplashPos, TVector4::VEC_NEGY );

	ASplashManager::GetSingleton()->Spawn( m_vPeckPos, TVector4::VEC_NEGY, 1.0f, ASplashManager::SPLASHTYPE_WATER );

	Rumble();
}

void AChickenFireMinigameState::Rumble()
{
	AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f }, 1 };
	AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
}

TINT AChickenFireMinigameState::FindClosestHit( const ACollisionModelRayHitList& a_rcHitList, TFLOAT& a_rfDistance )
{
	TINT iClosest = -1;

	for ( TINT i = 0; i < a_rcHitList.iNumHits; i++ )
	{
		if ( a_rcHitList.aHits[ i ].fDistance < a_rfDistance )
		{
			a_rfDistance = a_rcHitList.aHits[ i ].fDistance;
			iClosest     = i;
		}
	}

	return iClosest;
}
