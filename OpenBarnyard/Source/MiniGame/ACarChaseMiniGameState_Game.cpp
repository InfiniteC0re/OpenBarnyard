#include "pch.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "ACarChaseMiniGameState.h"
#include "ARandom.h"
#include "Helpers/ASpline.h"
#include "Helpers/APlayerProgress.h"
#include "Terrain/ATerrainInterface.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ABikePhysicsCameraHelper.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/carchase.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"
#include "Render/AShadowManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "Units/AUnitCarChaseCar.h"
#include "UnitMovement/Steering/ARacingCarSteer.h"
#include "UnitMovement/Steering/AKidBikeSteer.h"
#include "Physics/APhysicsCar.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING
using namespace soundbank;

// Speed limit table of the track data ("Main" symbol of the track data TRB)
struct ACarChaseTrackData
{
	struct SpeedPoint
	{
		TFLOAT fProgress;
		TFLOAT fSpeed;
	};

	TINT        iNumSpeedPoints;
	SpeedPoint* pSpeedPoints;
};

// Eases the pedals of a_pCar towards accelerating (a_bAccelerate) or braking.
// NOTE: inlined in UpdateRacers and UpdateSplineFollowers.
static void AdjustPedals( APhysicsCar* a_pCar, TBOOL a_bAccelerate, TFLOAT a_fDeltaTime )
{
	TFLOAT fThrottle = a_pCar->m_fThrottle;
	TFLOAT fBrake    = a_pCar->m_fBrake;

	if ( a_bAccelerate )
	{
		if ( fBrake <= 0.0f )
		{
			if ( fThrottle < 1.0f )
			{
				fThrottle = a_fDeltaTime + a_fDeltaTime + fThrottle;
				TMath::Clip( fThrottle, 0.0f, 1.0f );
			}
		}
		else
		{
			fBrake = fBrake - ( a_fDeltaTime + a_fDeltaTime );
			TMath::Clip( fBrake, 0.0f, 1.0f );
		}
	}
	else
	{
		if ( fThrottle <= 0.0f )
		{
			if ( fBrake < 1.0f )
			{
				fBrake = a_fDeltaTime + a_fDeltaTime + fBrake;
				TMath::Clip( fBrake, 0.0f, 1.0f );
			}
		}
		else
		{
			fThrottle = fThrottle - ( a_fDeltaTime + a_fDeltaTime );
			TMath::Clip( fThrottle, 0.0f, 1.0f );
		}
	}

	a_pCar->SetBrake( fBrake );
	a_pCar->SetThrottle( fThrottle );
}

// Stops the car of a_pSteer and puts it in neutral when it's in reverse.
// NOTE: inlined in UpdateGame.
static void StopCar( ARacingCarSteer* a_pSteer )
{
	APhysicsCar* pCar = a_pSteer->GetCar();
	pCar->SetThrottle( 0.0f );
	pCar->SetBrake( 1.0f );

	if ( a_pSteer->GetCar()->m_oGearBox.iGear == 0 )
	{
		a_pSteer->SetVelocity( TVector4::VEC_ZERO );

		APhysicsGearBox& rGearBox = a_pSteer->GetCar()->m_oGearBox;
		rGearBox.iPrevGear        = rGearBox.iGear;
		rGearBox.iGear            = 1;
	}
}

// XZ distance squared between two points (TVector4::DistanceSqXZ)
static TFLOAT DistanceSqXZ( const TVector4& a_rcVec1, const TVector4& a_rcVec2 )
{
	TFLOAT fZ = a_rcVec2.z - a_rcVec1.z;
	TFLOAT fX = a_rcVec2.x - a_rcVec1.x;
	return fZ * fZ + fX * fX;
}

TBOOL ACarChaseMiniGameState::GetTrackSpeedLimit( TFLOAT a_fProgress, TFLOAT& a_rfOutSpeed ) const
{
	const ACarChaseTrackData* pData = TSTATICCAST( const ACarChaseTrackData, m_pTrackData );

	TINT i = 0;

	if ( a_fProgress > pData->pSpeedPoints[ 0 ].fProgress )
	{
		do
		{
			if ( i >= pData->iNumSpeedPoints - 1 )
				break;

			i++;
		} while ( a_fProgress > pData->pSpeedPoints[ i ].fProgress );
	}

	if ( i <= 0 )
		return TFALSE;

	const ACarChaseTrackData::SpeedPoint& rPrev = pData->pSpeedPoints[ i - 1 ];
	const ACarChaseTrackData::SpeedPoint& rNext = pData->pSpeedPoints[ i ];

	TFLOAT fRatio = ( a_fProgress - rPrev.fProgress ) / ( rNext.fProgress - rPrev.fProgress );
	TMath::Clip( fRatio, 0.0f, 1.0f );

	a_rfOutSpeed = TMath::LERP( rPrev.fSpeed, rNext.fSpeed, fRatio );
	return TTRUE;
}

// $Barnyard: FUNCTION 004f3270
void ACarChaseMiniGameState::UpdateGame( TFLOAT a_fDeltaTime )
{
	// The spinning star of the "collect the stars" track
	if ( m_eTrack == TRACK_STAR )
	{
		TMatrix44 matTransform;
		m_pCarStar->GetModelInstance()->GetLocalTransform( matTransform );
		matTransform.RotateZ( a_fDeltaTime * 5.0f );
		m_pCarStar->GetModelInstance()->SetTransform( matTransform );
	}

	switch ( m_eState )
	{
		case STATE_INIT:
			m_eState = STATE_START;
			break;
		case STATE_START_MESSAGE:
			if ( m_iUnkc98 > 0 )
			{
				static TFLOAT s_fTextDelay = 0.0f;
				s_fTextDelay += a_fDeltaTime;

				if ( s_fTextDelay > 1.0f )
				{
					ALocale::FormatTags( m_wszText, TARRAYSIZE( m_wszText ), 665, PPropertyValue( m_iUnkc98 ) );

					m_oFadingText.m_fUnk4 = 1.0f;
					m_oFadingText.m_fUnk3 = 1.0f;
					// NOTE: the original stores the integer 2 in this float
					*TREINTERPRETCAST( TINT*, &m_oFadingText.m_fUnk1 ) = 2;
					m_oFadingText.m_fUnk2                              = 1.2f;
					m_oFadingText.m_fFadeTime                          = 4.0f;

					// DOES_NOTHING 0054df00 is called on m_oFadingText with &m_aiMapRect and TVector2( 0.5f, 0.5f )
					m_oFadingText.SetText( m_wszText, 0x7755ff55 );

					s_fTextDelay = 0.0f;
					m_eState     = STATE_START;
				}
			}
			else
			{
				m_eState = STATE_START;
			}
			break;
		case STATE_START:
			m_eState = STATE_RACING;
			break;
		case STATE_RESTART:
		{
			static TFLOAT s_fRestartDelay = 0.0f;
			s_fRestartDelay += a_fDeltaTime;

			if ( s_fRestartDelay > 3.0f )
			{
				s_fRestartDelay = 0.0f;
				SetupLevel();
			}
			break;
		}
		case STATE_RACING:
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			DevAutoPlay( a_fDeltaTime );
#endif

			if ( m_eTrack == TRACK_COINS )
			{
				UpdateCoins( a_fDeltaTime );
			}
			else
			{
				if ( m_eTrack != TRACK_FOLLOW_KIDS && m_apCars[ CAR_PLAYER ]->m_iState != AUnitCarChaseCar::STATE_FINISHED )
					UpdateRacers( a_fDeltaTime );

				UpdateSplineFollowers( a_fDeltaTime );
			}

			if ( m_eTrack == TRACK_FOLLOW_KIDS )
				UpdateFollowKids( a_fDeltaTime );
			else
				UpdateRace( a_fDeltaTime );

			UpdateHUD( a_fDeltaTime );

			// Pull the camera closer for a moment when the player changes up a gear
			ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

			if ( pCameraHelper && pCameraHelper->IsA( &TGetClass( ABikePhysicsCameraHelper ) ) )
			{
				ABikePhysicsCameraHelper* pBikeHelper = TSTATICCAST( ABikePhysicsCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() );
				m_apCameraHelpers[ 0 ]                = pBikeHelper;

				TVector4 vOffset = pBikeHelper->GetCameraOffset();
				TINT     iGear   = m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox.iGear;

				if ( m_apCars[ CAR_PLAYER ]->m_iDriveState == AUnitCarChaseCar::DRIVESTATE_ACCELERATE && iGear > m_iLastGear && iGear > 2 )
					m_fGearShiftCameraTime = 0.25f;

				if ( m_fGearShiftCameraTime <= 0.0f )
				{
					m_fGearShiftCameraTime = 0.0f;
					vOffset.Lerp4( vOffset, TVector4( 0.0f, -3.0f, -8.0f, 1.0f ), a_fDeltaTime );
				}
				else
				{
					vOffset.Lerp4( vOffset, TVector4( 0.0f, -3.0f, -6.0f, 1.0f ), a_fDeltaTime );
					m_fGearShiftCameraTime -= a_fDeltaTime;
				}

				pBikeHelper->SetCameraOffset( vOffset );
				m_iLastGear = iGear;
			}

			// Voices of the other drivers
			TVector4       vCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
			ASoundManager* pSoundMngr = ASoundManager::GetSingleton();
			TRandom&       rRandom    = ARandom::GetSingleton()->m_oRandom;

			if ( m_eTrack == TRACK_STAR )
			{
				// A racer is close
				TBOOL bClose = TFALSE;

				for ( TINT i = 0; i < NUM_RACERS; i++ )
				{
					if ( m_apCars[ CAR_RACER1 + i ]->IsVisible() &&
					     DistanceSqXZ( *m_apSteers[ CAR_PLAYER ]->GetPosition(), *m_apSteers[ CAR_RACER1 + i ]->GetPosition() ) <= 2500.0f )
					{
						bClose = TTRUE;
						break;
					}
				}

				if ( bClose == TTRUE && !pSoundMngr->IsCueUsed( m_iCue4384 ) )
				{
					m_fDriverVoiceTimer -= a_fDeltaTime;

					if ( m_fDriverVoiceTimer <= 0.0f && rRandom.GetFloat() >= 0.8f )
					{
						m_iCue4384          = pSoundMngr->PlayCueAtPositionEx( ASoundId( rRandom.GetInt( 2 ) + CARCHASE_EDDY_CHASE ), vCameraPos, 1.0f, TFALSE, 0.0f, -1 );
						m_fDriverVoiceTimer = 15.0f;
					}
				}
				else
				{
					pSoundMngr->SetCuePosition( m_iCue4384, vCameraPos, 0.0f );
				}
			}
			else if ( m_eTrack == TRACK_FOLLOW_KIDS )
			{
				TFLOAT fDistanceSq = DistanceSqXZ( *m_apSteers[ CAR_PLAYER ]->GetPosition(), *m_apKidBikeSteers[ 1 ]->GetPosition() );

				if ( fDistanceSq >= 5625.0f || pSoundMngr->IsCueUsed( m_iCue4384 ) )
				{
					pSoundMngr->SetCuePosition( m_iCue4384, vCameraPos, 0.0f );
				}
				else
				{
					m_fDriverVoiceTimer -= a_fDeltaTime;

					if ( m_fDriverVoiceTimer <= 0.0f && rRandom.GetFloat() >= 0.8f )
					{
						m_iCue4384          = pSoundMngr->PlayCueAtPositionEx( ASoundId( rRandom.GetInt( 2 ) + CARCHASE_BUD_LAUGH ), vCameraPos, 1.0f, TFALSE, 0.0f, -1 );
						m_fDriverVoiceTimer = 15.0f;
					}
				}

				if ( fDistanceSq >= 625.0f || pSoundMngr->IsCueUsed( m_iCue438c ) )
				{
					pSoundMngr->SetCuePosition( m_iCue438c, vCameraPos, 0.0f );
				}
				else
				{
					m_fBikeVoiceTimer -= a_fDeltaTime;

					if ( m_fBikeVoiceTimer <= 0.0f && rRandom.GetFloat() >= 0.8f )
					{
						m_iCue438c        = pSoundMngr->PlayCueAtPositionEx( CARCHASE_BS_BIKE, vCameraPos, 1.0f, TFALSE, 0.0f, -1 );
						m_fBikeVoiceTimer = 10.0f;
					}
				}
			}
			else if ( m_eTrack < TRACK_FOLLOW_KIDS )
			{
				if ( DistanceSqXZ( *m_apSteers[ CAR_PLAYER ]->GetPosition(), *m_apSteers[ CAR_FOLLOWER ]->GetPosition() ) > 100.0f ||
				     pSoundMngr->IsCueUsed( m_iCue4384 ) )
				{
					pSoundMngr->SetCuePosition( m_iCue4384, vCameraPos, 0.0f );
				}
				else
				{
					m_fDriverVoiceTimer -= a_fDeltaTime;

					if ( m_fDriverVoiceTimer <= 0.0f && rRandom.GetFloat() >= 0.8f )
					{
						m_iCue4384          = pSoundMngr->PlayCueAtPositionEx( ASoundId( rRandom.GetInt( 2 ) + CARCHASE_BUD_TAUNT ), vCameraPos, 1.0f, TFALSE, 0.0f, -1 );
						m_fDriverVoiceTimer = 15.0f;
					}
				}
			}
			break;
		}
		case STATE_LOST:
		{
			// Lost: stop every car
			APhysicsCar* pPlayerCar = m_apSteers[ CAR_PLAYER ]->GetCar();
			pPlayerCar->SetThrottle( 0.0f );
			pPlayerCar->SetBrake( 1.0f );
			m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox.bAllowReverse = TFALSE;

			if ( m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox.iGear == 0 )
			{
				m_apSteers[ CAR_PLAYER ]->SetVelocity( TVector4::VEC_ZERO );

				APhysicsGearBox& rGearBox = m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox;
				rGearBox.iPrevGear        = rGearBox.iGear;
				rGearBox.iGear            = 1;
			}

			for ( TINT i = CAR_RACER2; i >= CAR_RACER1; i-- )
			{
				if ( m_apCars[ i ] )
				{
					StopCar( m_apSteers[ i ] );
					m_apCars[ i ]->StopSounds();
				}
			}

			if ( m_apCars[ CAR_FOLLOWER ] )
			{
				StopCar( m_apSteers[ CAR_FOLLOWER ] );
				m_apCars[ CAR_FOLLOWER ]->StopSounds();
			}

			m_bInputEnabled = TFALSE;

			if ( m_iNumPlayers == 1 )
			{
				TINT iLocaleId;

				switch ( m_eSubState )
				{
					case SUBSTATE_NONE:
					case 1:
						iLocaleId = 671;
						break;
					case SUBSTATE_CAUGHT:
						iLocaleId = 673;
						break;
					case SUBSTATE_SPOTTED:
					case 5:
						iLocaleId = 672;
						break;
					case SUBSTATE_KIDS_LOST:
						iLocaleId = 674;
						break;
					default:
						iLocaleId = 661;
						break;
				}

				m_oFadingText.SetLocaleText( iLocaleId, -1 );
			}

			m_eState = STATE_OUTRO;
			SetHUDVisible( TFALSE );
			m_oIconElement.Hide();
			ASoundManager::GetSingleton()->StopCue( m_iCue3824 );
			m_iCue3824 = -1;
			break;
		}
		case STATE_WON:
		{
			// Finished: stop every car
			APhysicsCar* pPlayerCar = m_apSteers[ CAR_PLAYER ]->GetCar();
			pPlayerCar->SetThrottle( 0.0f );
			pPlayerCar->SetBrake( 1.0f );

			if ( m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox.iGear == 0 )
			{
				m_apSteers[ CAR_PLAYER ]->SetVelocity( TVector4::VEC_ZERO );

				APhysicsGearBox& rGearBox = m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox;
				rGearBox.iPrevGear        = rGearBox.iGear;
				rGearBox.iGear            = 1;
			}

			m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox.bAllowReverse = TFALSE;

			for ( TINT i = CAR_RACER2; i >= CAR_RACER1; i-- )
			{
				if ( m_apCars[ i ] )
				{
					StopCar( m_apSteers[ i ] );
					m_apCars[ i ]->StopSounds();
				}
			}

			if ( m_apCars[ CAR_FOLLOWER ] )
			{
				StopCar( m_apSteers[ CAR_FOLLOWER ] );
				m_apCars[ CAR_FOLLOWER ]->StopSounds();
			}

			m_bInputEnabled = TFALSE;

			if ( m_eTrack == TRACK_FOLLOW_KIDS || m_eTrack == TRACK_STAR )
			{
				m_oFadingText.SetLocaleText( 325, -1 );
			}
			else if ( m_eTrack == TRACK_COINS )
			{
				m_oFadingText.SetLocaleText( 242, -1 );
			}
			else if ( m_iNumPlayers == 1 )
			{
				// "1st", "2nd", "3rd", ...
				TINT iLocaleId;

				switch ( m_apCars[ CAR_PLAYER ]->GetFinishPosition() )
				{
					case 1:
						iLocaleId = 657;
						break;
					case 2:
						iLocaleId = 658;
						break;
					case 3:
						iLocaleId = 659;
						break;
					default:
						iLocaleId = 660;
						break;
				}

				TStringManager::String16CopySafe( m_wszText, ALocaleManager::Interface()->GetString( iLocaleId ), TARRAYSIZE( m_wszText ) );

				m_oFadingText.m_fUnk4 = 1.0f;
				m_oFadingText.m_fUnk3 = 1.0f;
				// NOTE: the original stores the integer 0 in this float
				*TREINTERPRETCAST( TINT*, &m_oFadingText.m_fUnk1 ) = 0;
				m_oFadingText.m_fUnk2                              = 1.2f;
				m_oFadingText.m_fFadeTime                          = 3.0f;

				// DOES_NOTHING 0054df00 is called on m_oFadingText with &m_aiMapRect and TVector2( 0.5f, 0.5f )
				m_oFadingText.SetText( m_wszText, 0xff00ff00 );
			}

			m_eState = STATE_OUTRO;
			SetHUDVisible( TFALSE );
			m_oIconElement.Hide();
			ASoundManager::GetSingleton()->StopCue( m_iCue3824 );
			m_iCue3824 = -1;
			break;
		}
		case STATE_OUTRO:
		{
			m_oFadingText.Update( a_fDeltaTime );
			m_bInputEnabled = TFALSE;
			m_fOutroTime += a_fDeltaTime;

			if ( m_fOutroTime >= 5.0f )
			{
				m_oIconElement.Hide();
				SetHUDVisible( TFALSE );
				m_eState   = STATE_GAME_OVER;
				TINT iZero = 0;
				ShowGameOver( 0, &iZero );
			}

			// The car has stopped, the camera circles around it
			if ( m_apSteers[ CAR_PLAYER ]->GetSpeed() <= 5.0f )
			{
				m_apCars[ CAR_PLAYER ]->StopSounds();

				ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

				if ( ACameraManager::GetSingleton()->GetCurrentCameraHelper() )
				{
					TFLOAT   fFOV      = pCamera->GetFOV();
					TVector4 vPosition = pCamera->GetMatrix().GetTranslation();
					ACameraManager::GetSingleton()->DetachCameraHelpers();
					pCamera->GetMatrix().GetTranslation() = vPosition;
					pCamera->SetFOV( fFOV );
				}

				TVector4 vTarget = *m_apSteers[ CAR_PLAYER ]->GetPosition();
				vTarget.y        = vTarget.y - 1.0f;

				TVector4 vOffset = pCamera->GetMatrix().GetTranslation() - vTarget;
				TFLOAT   fDist   = vOffset.NormaliseMag();
				fDist            = TMath::LERP( fDist, 10.0f, a_fDeltaTime * 5.0f );

				// Don't go through the terrain
				TFLOAT fMaxDist = 100.0f;

				if ( !ATerrainInterface::GetSingleton()->CollideRay( vTarget, vOffset, fMaxDist, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) )
					fMaxDist = 100.0f;

				vOffset.AsVector3().RotateY( a_fDeltaTime );

				TVector4 vScaled;
				vScaled.Multiply( vOffset, TMath::Min( fDist, fMaxDist ) );

				TVector4 vPosition                    = vTarget + vScaled;
				pCamera->GetMatrix().GetTranslation() = vPosition;
				pCamera->LookAtPoint( vTarget );
			}

			// Stop the kid bikes
			if ( m_eTrack == TRACK_FOLLOW_KIDS && m_apKidBikeSteers[ 0 ] )
			{
				TFLOAT fSpeed = m_apKidBikeSteers[ 0 ]->GetSpeed() - ( a_fDeltaTime + a_fDeltaTime );

				if ( fSpeed < 0.0f )
					fSpeed = 0.0f;

				m_apKidBikeSteers[ 0 ]->SetSpeed( fSpeed );
			}
			break;
		}
		case STATE_RESULTS:
			if ( !FindChildState( &TGetClass( AAnticGameOverState ) ) )
				SetupLevel();
			break;
	}

	// Keep the cue on the player car
	if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iCue3824 ) )
		m_iCue3824 = -1;

	if ( m_iCue3824 != -1 )
		ASoundManager::GetSingleton()->SetCuePosition( m_iCue3824, *m_apSteers[ CAR_PLAYER ]->GetPosition(), 0.0f );
}

// $Barnyard: FUNCTION 004f0320
void ACarChaseMiniGameState::UpdateRacers( TFLOAT a_fDeltaTime )
{
	TINT iNumIdle    = 0;
	TINT iNumChasing = 0;

	TVector4 vPlayerClosest;
	TVector4 vPlayerDirection;
	TFLOAT   fPlayerProgress;
	m_oRaceTrack.oSpline.GetClosestPositionAndDirection( *m_apSteers[ CAR_PLAYER ]->GetPosition(), vPlayerClosest, vPlayerDirection, fPlayerProgress, 300.0f );
	m_oRaceTrack.oSpline.GetLength();

	for ( TINT i = NUM_RACERS - 1; i >= 0; i-- )
	{
		AUnitCarChaseCar* pCar   = m_apCars[ CAR_RACER1 + i ];
		ARacingCarSteer*  pSteer = m_apSteers[ CAR_RACER1 + i ];
		Racer&            rRacer = m_aRacers[ i ];

		if ( !pCar->IsVisible() )
		{
			iNumIdle++;
			continue;
		}

		TVector4 vClosest;
		TVector4 vClosestDirection;
		TFLOAT   fProgress;
		m_oRaceTrack.oSpline.GetClosestPositionAndDirection( *pSteer->GetPosition(), vClosest, vClosestDirection, fProgress, 300.0f );

		TFLOAT fOffTrackSq = DistanceSqXZ( *pSteer->GetPosition(), vClosest );

		// Keep to 90% of the speed limit of the track while chasing
		if ( m_pTrackData && rRacer.iMode == Racer::MODE_CHASE )
		{
			TFLOAT fSpeedLimit;

			if ( GetTrackSpeedLimit( fProgress, fSpeedLimit ) )
				AdjustPedals( pSteer->GetCar(), fSpeedLimit * 0.9f >= pSteer->GetSpeed(), a_fDeltaTime );
		}

		// Stuck off the track behind the player: put it back on the track
		if ( fProgress < fPlayerProgress && fOffTrackSq >= 25.0f && pSteer->GetSpeed() < 2.0f && DistanceSqXZ( vPlayerClosest, vClosest ) >= 2500.0f )
			pSteer->Reset( vClosest, vClosestDirection );

		TINT iPedal;
		TINT iSteer;

		// NOTE: the original leaves the steering uninitialised when the racer doesn't drive
		TFLOAT fSteering = 0.0f;

		switch ( rRacer.Update( pSteer, m_iNumPlayers, &m_apSteers[ CAR_PLAYER ], &m_apCars[ CAR_PLAYER ], &iPedal, &iSteer, &fSteering, a_fDeltaTime ) )
		{
			case Racer::MODE_IDLE:
				iNumIdle++;

				// The player has passed the second racer: start chasing
				if ( i == 1 )
				{
					TFLOAT fPlayerT = m_oRaceTrack.oSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );
					TFLOAT fRacerT  = m_oRaceTrack.oSpline.FindClosestProgress( *m_apSteers[ CAR_RACER2 ]->GetPosition(), 300.0f );

					if ( fRacerT < fPlayerT )
					{
						m_aRacers[ 1 ].SetMode( Racer::MODE_CHASE, 0 );
						m_apCars[ CAR_RACER2 ]->SetSirenOn( TTRUE );
						m_apSteers[ CAR_RACER2 ]->GetCar()->SetThrottle( 1.0f );
						m_apSteers[ CAR_RACER2 ]->GetCar()->SetBrake( 0.0f );
					}
				}
				break;
			case Racer::MODE_CHASE:
			case Racer::MODE_FOLLOW:
				iNumChasing++;
				break;
			case Racer::MODE_CAUGHT:
				// Caught the player: lose a life
				m_oIconElement.Hide();
				pCar->SetSirenOn( TFALSE );
				rRacer.SetMode( Racer::MODE_FINISHED, 0 );

				if ( m_iLives <= 1 )
				{
					m_eSubState = SUBSTATE_CAUGHT;
					m_eState    = STATE_LOST;
				}
				else
				{
					m_oFadingText.SetLocaleText( 672, -1 );
					m_iLives--;
					m_oLifeHud.AddLives( -1 );
				}
				return;
			case Racer::MODE_LOST:
				m_oIconElement.Hide();
				pCar->SetVisible( TFALSE );
				m_aMiniMaps[ 0 ].SetIconVisible( pCar->m_iUnk168, TFALSE );
				pCar->SetSirenOn( TFALSE );
				break;
			case Racer::MODE_FINISHED:
				iNumIdle++;

				// Hide it once it's far away
				if ( DistanceSqXZ( *m_apSteers[ CAR_PLAYER ]->GetPosition(), *pSteer->GetPosition() ) >= 10000.0f )
				{
					pCar->SetVisible( TFALSE );
					m_aMiniMaps[ 0 ].SetIconVisible( pCar->m_iUnk168, TFALSE );
				}
				break;
		}

		iSteer                  = STEERCOMMAND_ANALOG;
		iPedal                  = -1;
		TFLOAT fClampedSteering = fSteering;
		TMath::Clip( fClampedSteering, -1.0f, 1.0f );
		DriveCar( pSteer, iPedal, iSteer, fClampedSteering, fSteering, 0 );
	}

	// No racer around: bring them back every 10 seconds in the middle part of the track
	TFLOAT fPlayerT = m_oRaceTrack.oSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );

	if ( fPlayerT >= 0.1f && fPlayerT <= 0.9f )
	{
		if ( iNumIdle == NUM_RACERS )
		{
			m_fRespawnTimer -= a_fDeltaTime;

			if ( m_fRespawnTimer <= 0.0f )
			{
				m_fRespawnTimer = 10.0f;
				RespawnRacer1();
				RespawnRacer2();
			}
		}
		else
		{
			m_fRespawnTimer = 10.0f;
		}
	}

	static TINT   s_iChaseCue = -1;
	static TFLOAT s_afChaseTimer[ NUM_RACERS ];

	if ( iNumChasing != 0 )
	{
		// The cop chase music while a racer follows the player
		for ( TINT i = 0; i < NUM_RACERS; i++ )
		{
			if ( m_aRacers[ i ].iMode == Racer::MODE_FOLLOW )
			{
				m_oIconElement.Show();
				s_afChaseTimer[ i ] -= a_fDeltaTime;

				if ( s_afChaseTimer[ i ] <= 0.0f )
				{
					if ( !ASoundManager::GetSingleton()->IsCueUsed( s_iChaseCue ) )
						s_iChaseCue = ASoundManager::GetSingleton()->PlayCue( CARCHASE_COP_CHASE );

					s_afChaseTimer[ i ] = 10.0f;
				}
			}
		}
	}
	else
	{
		m_oIconElement.Hide();
	}

	if ( s_iChaseCue != -1 && ASoundManager::GetSingleton()->IsCueUsed( s_iChaseCue ) == TTRUE )
	{
		TVector4 vCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
		ASoundManager::GetSingleton()->SetCuePosition( s_iChaseCue, vCameraPos, 0.0f );
	}
}

// $Barnyard: FUNCTION 004ed3c0
// $BarnyardWiiJP: FUNCTION 802399a0
void ACarChaseMiniGameState::UpdateSplineFollowers( TFLOAT a_fDeltaTime )
{
	ASpline& rSpline = m_oRaceTrack.oSpline;

	TVector4 vPlayerClosest;
	TVector4 vPlayerDirection;
	TFLOAT   fPlayerProgress;
	rSpline.GetClosestPositionAndDirection( *m_apSteers[ CAR_PLAYER ]->GetPosition(), vPlayerClosest, vPlayerDirection, fPlayerProgress, 300.0f );

	TFLOAT fPlayerDistance = rSpline.GetLength() * fPlayerProgress;

#ifdef BARNYARD_LATEST
	// The player's truck is put back on the track at the closest point when it lies on its roof (APhysicsCar::FlipUpright).
	// It makes the chase easier (made for the Wii controls), so here it's only done on Easy.
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
	{
		APhysicsCar* pPlayerPhysics = m_apSteers[ CAR_PLAYER ]->GetCar();
		pPlayerPhysics->SetRecoveryPoint( vPlayerClosest );
		pPlayerPhysics->SetRecoveryDirection( vPlayerDirection );
	}
#endif // BARNYARD_LATEST

	// NOTE: m_iNumSplineFollowers is the number of spline followers
	for ( TINT i = m_iNumSplineFollowers - 1; i >= 0; i-- )
	{
		AUnitCarChaseCar* pCar   = m_apCars[ CAR_FOLLOWER + i ];
		ARacingCarSteer*  pSteer = m_apSteers[ CAR_FOLLOWER + i ];

		if ( pCar->m_iState == AUnitCarChaseCar::STATE_FINISHED )
			continue;

		TVector4 vClosest;
		TVector4 vClosestDirection;
		TFLOAT   fProgress;
		rSpline.GetClosestPositionAndDirection( *pSteer->GetPosition(), vClosest, vClosestDirection, fProgress, 300.0f );

		TFLOAT       fOffTrackSq = DistanceSqXZ( *pSteer->GetPosition(), vClosest );
		TFLOAT       fToPlayerSq = DistanceSqXZ( vPlayerClosest, vClosest );
		APhysicsCar* pPhysics    = pSteer->GetCar();

		if ( m_pTrackData )
		{
			TFLOAT fSpeedLimit;

			if ( GetTrackSpeedLimit( fProgress, fSpeedLimit ) )
			{
				if ( fProgress <= fPlayerProgress )
				{
					// Behind the player: full speed
					m_fSpeedScale = TMath::LERP( m_fSpeedScale, 1.0f, a_fDeltaTime * 10.0f );
				}
				else
				{
					// Ahead of the player: slow down to half speed when the player is 100 behind
					TFLOAT fRatio = fToPlayerSq / 10000.0f;
					TMath::Clip( fRatio, 0.0f, 1.0f );
					fRatio = 1.0f - fRatio;
					TMath::Clip( fRatio, 0.5f, 1.0f );
					m_fSpeedScale = TMath::LERP( m_fSpeedScale, fRatio, a_fDeltaTime + a_fDeltaTime );
				}

#ifdef BARNYARD_LATEST
				// NOTE: the latest version (Wii 802399a0) drives the cars below the speed limit, at 0.82 of it on Easy
				// and 0.87 on Normal (806b4db0). Here Normal keeps the full speed limit of PC retail.
				const TFLOAT fDifficultyScale = APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 0.82f : 1.0f;
				AdjustPedals( pPhysics, fDifficultyScale * ( fSpeedLimit * m_fSpeedScale ) >= pSteer->GetSpeed(), a_fDeltaTime );
#else
				AdjustPedals( pPhysics, fSpeedLimit * m_fSpeedScale >= pSteer->GetSpeed(), a_fDeltaTime );
#endif
			}
		}

		// Stuck off the track behind the player: put it back on the track
		if ( fProgress < fPlayerProgress && fOffTrackSq >= 25.0f && pSteer->GetSpeed() < 2.0f && fToPlayerSq >= 10000.0f )
			pSteer->Reset( vClosest, vClosestDirection );

		TINT   iPedal;
		TINT   iSteer;
		TFLOAT fSteering;
		m_aSplineFollowers[ i ].Update( m_apSteers[ CAR_PLAYER ], &iPedal, &iSteer, &fSteering, fPlayerDistance, a_fDeltaTime );

		iSteer                  = STEERCOMMAND_ANALOG;
		iPedal                  = -1;
		TFLOAT fClampedSteering = fSteering;
		TMath::Clip( fClampedSteering, -1.0f, 1.0f );

		if ( pCar->m_iState == AUnitCarChaseCar::STATE_RACING )
			DriveCar( pSteer, iPedal, iSteer, fClampedSteering, fSteering, 0 );
	}
}

// $Barnyard: FUNCTION 004ed880
void ACarChaseMiniGameState::UpdateRace( TFLOAT a_fDeltaTime )
{
	ASpline& rSpline = m_oRaceTrack.oSpline;
	m_fTime += a_fDeltaTime;

	if ( m_eTrack != TRACK_COINS )
	{
		for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
		{
			if ( m_apCars[ i ]->m_iState == AUnitCarChaseCar::STATE_FINISHED )
				continue;

			// NOTE: always measured for the first player
			TFLOAT fProgress = rSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 1000.0f );

			TVector4 vTrackPos;
			TVector4 vTrackDir;
			rSpline.GetPosition( fProgress, vTrackPos );
			rSpline.GetDirection( fProgress, vTrackDir );

			m_afFinishTimes[ i ] = m_fTime;

			if ( DistanceSqXZ( vTrackPos, m_vFinishPosition ) > 1.0f )
			{
				// Driving the wrong way
				const TVector4* pDirection = m_apSteers[ CAR_PLAYER ]->GetDirection();

				if ( vTrackDir.z * pDirection->z + vTrackDir.x * pDirection->x < 0.0f && m_apSteers[ CAR_PLAYER ]->GetCar()->m_oGearBox.iGear != 0 )
					m_oFadingText.SetLocaleText( 1044, -1 );
			}
			else
			{
				// Reached the end of the track
				m_iNumPlayersFinished++;
				m_iFinishPlace++;
				m_bInputEnabled = TFALSE;
				m_apCars[ i ]->SetFinishPosition( m_iFinishPlace );
				m_apCars[ i ]->m_iState = AUnitCarChaseCar::STATE_FINISHED;

				m_apSteers[ i ]->GetCar()->m_oGearBox.bAllowReverse = TFALSE;
				m_apSteers[ i ]->GetCar()->SetThrottle( 0.0f );
				m_apSteers[ i ]->GetCar()->SetBrake( 1.0f );
				m_apSteers[ i ]->SetSteering( ARacingCarSteer::STEER_ABSOLUTE, 0.0f, 0.0f );

				if ( m_iNumFinishedCars == 0 )
					m_apCars[ i ]->m_bUnk18C = TTRUE;

				m_iNumFinishedCars++;
			}
		}

		for ( TINT i = m_iNumSplineFollowers - 1; i >= 0; i-- )
		{
			AUnitCarChaseCar* pCar   = m_apCars[ CAR_FOLLOWER + i ];
			ARacingCarSteer*  pSteer = m_apSteers[ CAR_FOLLOWER + i ];

			if ( pCar->m_iState == AUnitCarChaseCar::STATE_FINISHED )
				continue;

			TFLOAT fProgress = rSpline.FindClosestProgress( *pSteer->GetPosition(), 1000.0f );

			TVector4 vTrackPos;
			rSpline.GetPosition( fProgress, vTrackPos );

			m_afFinishTimes[ 1 + i ] = m_fTime;

			if ( DistanceSqXZ( vTrackPos, m_vFinishPosition ) <= 1.0f )
			{
				// Reached the end of the track
				m_iFinishPlace++;
				pCar->SetFinishPosition( m_iFinishPlace );
				pCar->m_iState = AUnitCarChaseCar::STATE_FINISHED;

				pSteer->GetCar()->SetThrottle( 0.0f );
				pSteer->GetCar()->SetBrake( 1.0f );
				pSteer->SetSteering( ARacingCarSteer::STEER_ABSOLUTE, 0.0f, 0.0f );

				if ( m_iNumFinishedCars == 0 )
					pCar->m_bUnk18C = TTRUE;

				m_iNumFinishedCars++;
			}
		}
	}

	if ( m_iNumPlayersFinished < m_iNumPlayers )
	{
		if ( m_fTime >= m_fTimeLimit )
		{
			// Out of time
			if ( m_eTrack == TRACK_COINS )
			{
				m_eState                          = STATE_WON;
				m_apCars[ CAR_PLAYER ]->m_bUnk18C = TTRUE;
			}
			else
			{
				m_eSubState = SUBSTATE_OUT_OF_TIME;
				m_eState    = STATE_LOST;
			}
		}
	}
	else
	{
		m_eState = STATE_WON;
	}
}

// $Barnyard: FUNCTION 004ef250
void ACarChaseMiniGameState::UpdateFollowKids( TFLOAT a_fDeltaTime )
{
	TFLOAT fDistanceSq = DistanceSqXZ( *m_apKidBikeSteers[ 1 ]->GetPosition(), *m_apSteers[ CAR_PLAYER ]->GetPosition() );

	if ( fDistanceSq >= 625.0f )
	{
		if ( fDistanceSq <= 625.0f || fDistanceSq >= 5625.0f )
		{
			// Too far away
			m_oFadingText.SetLocaleText( 676, -1 );
			m_fDistanceWarningTime += a_fDeltaTime;

			if ( m_fDistanceWarningTime >= 5.0f )
			{
				m_apCars[ CAR_PLAYER ]->m_iState = AUnitCarChaseCar::STATE_FAILED;
				m_afFinishTimes[ 0 ]             = 0.0f;
				m_eSubState                      = SUBSTATE_KIDS_LOST;
				m_eState                         = STATE_LOST;
				return;
			}
		}
		else
		{
			m_fDistanceWarningTime = 0.0f;
		}
	}
	else
	{
		// Too close
		m_oFadingText.SetLocaleText( 675, -1 );
		m_fDistanceWarningTime += a_fDeltaTime;

		TFLOAT fPlayerProgress = m_oRaceTrack.oSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );

		if ( m_fDistanceWarningTime >= 5.0f || m_apKidBikeSteers[ 0 ]->GetProgress() <= fPlayerProgress || fDistanceSq <= 100.0f )
		{
			// Spotted
			ASoundManager::GetSingleton()->StopCue( m_iCue438c );
			m_iCue438c = ASoundManager::GetSingleton()->PlayCueAtPositionEx( CARCHASE_BS_COWS, ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), 1.0f, TFALSE, 0.0f, -1 );

			m_apCars[ CAR_PLAYER ]->m_iState = AUnitCarChaseCar::STATE_FAILED;
			m_afFinishTimes[ 0 ]             = 0.0f;
			m_eSubState                      = SUBSTATE_SPOTTED;
			m_eState                         = STATE_LOST;
			return;
		}
	}

	if ( m_apKidBikeSteers[ 0 ]->GetProgress() >= 1.0f )
	{
		// The kids got home
		m_apCars[ CAR_PLAYER ]->m_iState  = AUnitCarChaseCar::STATE_FINISHED;
		m_apCars[ CAR_PLAYER ]->m_bUnk18C = TTRUE;

		for ( TINT i = 0; i < NUM_KID_BIKES; i++ )
			m_apKidBikeSteers[ i ]->SetSpeed( 0.0f );

		m_eState = STATE_WON;
	}
}

// $Barnyard: FUNCTION 004edbd0
void ACarChaseMiniGameState::UpdateHUD( TFLOAT a_fDeltaTime )
{
	ASpline& rSpline = m_oRaceTrack.oSpline;

	if ( m_eTrack == TRACK_FOLLOW_KIDS )
	{
		// Distance to the kids along the track
		TFLOAT   fProgress = -1.0f;
		TVector4 vClosest;
		TVector4 vDirection;
		rSpline.GetClosestPositionAndDirection( *m_apSteers[ CAR_PLAYER ]->GetPosition(), vClosest, vDirection, fProgress, 300.0f );

		TVector4 vAhead;
		rSpline.GetPositionAndDirection( 30.0f / rSpline.GetLength() + fProgress, vAhead, vDirection );

		TFLOAT fLength         = rSpline.GetLength();
		TFLOAT fPlayerProgress = rSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );
		TFLOAT fKidDistance    = m_apKidBikeSteers[ 1 ]->GetProgress() * fLength - fPlayerProgress * fLength;
		m_fKidDistance         = TMath::LERP( m_fKidDistance, fKidDistance, a_fDeltaTime * 5.0f );
	}

	m_oFadingText.Update( a_fDeltaTime );

	// km/h or mph
	TFLOAT fSpeed;

	if ( ALocaleManager::Interface()->GetLanguage() != ALocaleManager::Lang_English )
		fSpeed = m_apSteers[ CAR_PLAYER ]->GetSpeed() * 3.6f;
	else
		fSpeed = m_apSteers[ CAR_PLAYER ]->GetSpeed() * 2.2f;

	m_oSpeedometer.SetSpeed( fSpeed );
	m_oSpeedometer.Update( a_fDeltaTime );

	for ( TINT i = m_iNumPlayers - 1; i >= 0; i-- )
	{
		AGUI2MiniMap&    rMiniMap = m_aMiniMaps[ i ];
		ARacingCarSteer* pSteer   = m_apSteers[ i ];

		if ( m_eTrack == TRACK_FOLLOW_KIDS )
			m_oTimer.SetTime( 0.0f );
		else
			m_oTimer.SetTime( m_fTimeLimit - m_fTime );

		rMiniMap.SetCentre( *pSteer->GetPosition(), *pSteer->GetDirection() );

		// Icon 0: the finish line or the kids
		if ( m_eTrack == TRACK_FOLLOW_KIDS )
		{
			rMiniMap.SetIconTransform( 0, *m_apKidBikeSteers[ 0 ]->GetPosition(), *m_apKidBikeSteers[ 0 ]->GetDirection() );
		}
		else if ( m_eTrack != TRACK_COINS )
		{
			TVector4 vToFinish = m_vFinishPosition - *pSteer->GetPosition();
			vToFinish.w        = m_vFinishPosition.w;
			vToFinish.Normalise();
			rMiniMap.SetIconTransform( 0, m_vFinishPosition, vToFinish );
		}

		for ( TINT k = CAR_RACER2; k >= CAR_RACER1; k-- )
		{
			if ( m_apCars[ k ] && m_apCars[ k ]->m_iUnk168 != -1 )
				rMiniMap.SetIconTransform( m_apCars[ k ]->m_iUnk168, *m_apSteers[ k ]->GetPosition(), *m_apSteers[ k ]->GetDirection() );
		}

		for ( TINT k = m_iNumSplineFollowers - 1; k >= 0; k-- )
		{
			AUnitCarChaseCar* pCar = m_apCars[ CAR_FOLLOWER + k ];

			if ( pCar && pCar->m_iUnk168 != -1 )
				rMiniMap.SetIconTransform( pCar->m_iUnk168, *m_apSteers[ CAR_FOLLOWER + k ]->GetPosition(), *m_apSteers[ CAR_FOLLOWER + k ]->GetDirection() );
		}

		// The direction arrow points at the kids or along the track
		TVector4 vArrowTarget;

		if ( m_eTrack == TRACK_FOLLOW_KIDS )
		{
			vArrowTarget = *m_apKidBikeSteers[ 1 ]->GetPosition();
		}
		else
		{
			TFLOAT fProgress = rSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f ) + 0.1f;
			TMath::Clip( fProgress, 0.0f, 1.0f );
			rSpline.GetPosition( fProgress, vArrowTarget );
		}

		m_oDirectionArrow.PointAt( *m_apSteers[ CAR_PLAYER ]->GetPosition(), vArrowTarget );
	}

	// The light of the cop chase icon pulses
	if ( m_oIconElement.IsVisible() )
	{
		TFLOAT fAngle = TMath::TWO_PI * g_oSystemManager.GetScheduler()->GetTotalTime();
		m_oIconRectangle.GetTransform().SetIdentity();

		TFLOAT fScale = TMath::Sin( fAngle ) * 0.2f + 1.0f;
		m_oIconRectangle.GetTransform().Scale( fScale, fScale );
	}

	// Race position
	if ( !m_oRacePlacingHud.IsForceHidden() )
	{
		TFLOAT fPlayerProgress = rSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );
		TINT   iPlacing        = 0;

		// NOTE: always measures the first follower
		for ( TINT k = m_iNumSplineFollowers; k != 0; k-- )
		{
			if ( rSpline.FindClosestProgress( *m_apSteers[ CAR_FOLLOWER ]->GetPosition(), 300.0f ) > fPlayerProgress && ++iPlacing > 4 )
				iPlacing = 4;
		}

		m_oRacePlacingHud.SetPlacing( iPlacing );
	}
}

// $Barnyard: FUNCTION 004f14c0
// $BarnyardWiiJP: FUNCTION 802395e0
void ACarChaseMiniGameState::UpdateCoins( TFLOAT a_fDeltaTime )
{
	TVector4 vPlayerPos = *m_apSteers[ CAR_PLAYER ]->GetPosition();

	for ( TINT i = 0; i < NUM_COINS; i++ )
	{
		Coin& rCoin = m_aCoins[ i ];

		if ( !rCoin.pObject )
			continue;

		TMatrix44 matTransform;
		rCoin.pObject->GetModelInstance()->GetLocalTransform( matTransform );

		TFLOAT fDistanceSq = DistanceSqXZ( vPlayerPos, matTransform.GetTranslation() );

		if ( !rCoin.bCollected )
		{
#ifdef BARNYARD_LATEST
			// The JP Wii build picks the coins up within 6 (PC: 3). Made for the Wii controls, so only on Easy here.
			if ( fDistanceSq < ( APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 36.0f : 9.0f ) )
#else
			if ( fDistanceSq < 9.0f )
#endif
			{
				// Collected
				m_iNumCoinsCollected++;
				StartCollectEffect( rCoin.pObject, 1, 30.0f );
				rCoin.bCollected = TTRUE;
				m_aMiniMaps[ 0 ].SetIconVisible( rCoin.iMapIcon, TFALSE );
				AShadowManager::GetSingleton()->SetShadowEnabled( rCoin.pObject->GetModelInstance(), TFALSE );
			}
#ifdef BARNYARD_LATEST
			else if ( fDistanceSq > 90000.0f )
#else
			else if ( fDistanceSq > 250000.0f )
#endif
			{
				// Only show the coins closer than 500 (JP Wii: 300)
				m_aMiniMaps[ 0 ].SetIconVisible( rCoin.iMapIcon, TFALSE );
				AShadowManager::GetSingleton()->SetShadowEnabled( rCoin.pObject->GetModelInstance(), TFALSE );
			}
			else
			{
				m_aMiniMaps[ 0 ].SetIconVisible( rCoin.iMapIcon, TTRUE );
				AShadowManager::GetSingleton()->SetShadowEnabled( rCoin.pObject->GetModelInstance(), TTRUE );
			}
		}

		// Spin
		AModelInstance* pModelInstance = rCoin.pObject->GetModelInstance();

		if ( pModelInstance && pModelInstance->IsUpdatingSkeleton() )
		{
			matTransform.RotateZ( a_fDeltaTime + a_fDeltaTime );
			rCoin.pObject->GetModelInstance()->SetTransform( matTransform );
		}
	}

	// NOTE: the JP Wii build also calls m_aMiniMaps[ 0 ].Tick( a_fDeltaTime ) here (Wii GUI glue, not recovered)
}

#ifdef BARNYARD_COMMUNITY_PATCH

//-----------------------------------------------------------------------------
// -devautoplay: the player car drives along the track (steers at a point
// ahead of it like the spline followers do) at the speed limit of the track.
//-----------------------------------------------------------------------------
void ACarChaseMiniGameState::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	static TINT s_iEnabled = -1;

	if ( s_iEnabled < 0 )
		s_iEnabled = T2String8::FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;

	if ( !s_iEnabled || !m_oRaceTrack.oSpline.IsLoaded() )
		return;

	ARacingCarSteer* pSteer  = m_apSteers[ CAR_PLAYER ];
	ASpline&         rSpline = m_oRaceTrack.oSpline;

	TVector4 vPosition  = *pSteer->GetPosition();
	TVector4 vDirection = *pSteer->GetDirection();
	vDirection.y        = 0.0f;
	vDirection.Normalise();

	TVector4 vClosest;
	TVector4 vClosestDirection;
	TFLOAT   fProgress;
	rSpline.GetClosestPositionAndDirection( vPosition, vClosest, vClosestDirection, fProgress, 300.0f );

	TFLOAT fLookAhead = 30.0f * ( pSteer->GetSpeed() / 60.0f ) + 10.0f;

	TVector4 vAhead;
	TVector4 vAheadDirection;
	rSpline.GetPositionAndDirection( fLookAhead / rSpline.GetLength() + fProgress, vAhead, vAheadDirection );

	TVector4 vWanted = vAhead - vPosition;
	vWanted.y        = 0.0f;
	vWanted.Normalise();

	TFLOAT fSteering = vWanted.GetAngleBetweenVector( vDirection );

	if ( vWanted.z * vDirection.x - vDirection.z * vWanted.x > 0.0f )
		fSteering = -fSteering;

	fSteering /= TMath::DegToRad( 10.0f );
	TMath::Clip( fSteering, -1.0f, 1.0f );

	// Slow down before the turns of the track and above its speed limit
	TFLOAT fSpeedLimit = 1000.0f;

	if ( m_pTrackData )
		GetTrackSpeedLimit( fProgress, fSpeedLimit );

	TFLOAT fMaxTurn = 0.0f;

	for ( TINT i = 1; i <= 4; i++ )
	{
		TVector4 vTurnPos;
		TVector4 vTurnDir;
		rSpline.GetPositionAndDirection( fProgress + ( i * 20.0f ) / rSpline.GetLength(), vTurnPos, vTurnDir );
		vTurnDir.y = 0.0f;
		vTurnDir.Normalise();

		TVector4 vTrackDir = vClosestDirection;
		vTrackDir.y        = 0.0f;
		vTrackDir.Normalise();

		fMaxTurn = TMath::Max( fMaxTurn, vTrackDir.GetAngleBetweenVector( vTurnDir ) );
	}

	TFLOAT fTurnRatio = fMaxTurn / TMath::DegToRad( 90.0f );
	TMath::Clip( fTurnRatio, 0.0f, 1.0f );
	fSpeedLimit = TMath::Min( fSpeedLimit, TMath::LERP( 70.0f, 22.0f, fTurnRatio ) );

	TBOOL bBrake = pSteer->GetSpeed() > fSpeedLimit || ( TMath::Abs( fSteering ) >= 1.0f && pSteer->GetSpeed() > 20.0f );

	pSteer->SetPedal( bBrake ? ARacingCarSteer::PEDAL_BRAKE : ARacingCarSteer::PEDAL_THROTTLE, TFALSE );

	if ( !bBrake )
		pSteer->GetCar()->SetBrake( 0.0f );
	else
		pSteer->GetCar()->SetThrottle( 0.0f );

	pSteer->SetSteering( ARacingCarSteer::STEER_ABSOLUTE, fSteering, 0.0f );

	static TFLOAT s_fLogTimer = 0.0f;
	s_fLogTimer += a_fDeltaTime;

	if ( s_fLogTimer >= 5.0f )
	{
		s_fLogTimer = 0.0f;
		TINFO( "[devautoplay] joyride: progress %.3f speed %.1f time %.1f/%.1f lives %d state %d\n", fProgress, pSteer->GetSpeed(), m_fTime, m_fTimeLimit, m_iLives, m_eState );
	}
}

#endif // BARNYARD_COMMUNITY_PATCH
