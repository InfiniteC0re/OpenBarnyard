#include "pch.h"
#include "ABikeManager.h"
#include "MiniGame/ABikeRaceMicroGame.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AKeyFrameLibraryManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "FSM/AMethodPointer.h"
#include "Units/AUnit.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitType.h"
#include "Units/AUnitTransform.h"
#include "Units/AAnimatedUnit.h"
#include "Player/APlayerManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "Helpers/AWeatherManager.h"
#include "Helpers/AGameTimeManager.h"
#include "FX/AParticleEffectManager.h"
#include "Render/AShadowManager.h"
#include "Render/ARenderer.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Animation/AModel.h"
#include "Physics/ACollisionObjectUnit.h"
#include "Physics/AVehicleManager.h"
#include "Physics/APhysicsBike.h"
#include "Helpers/ABikeHelpers.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AInstanceLightingHelper.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "GameInterface/ASimState.h"
#include "GUI/AFadeManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "Player/APlayerProfileManager.h"
#include "Player/APlayerSet.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/music.h"
#include "Sound/AMultiplexStream.h"
#include "Units/AInteractionComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Animation/AFallingCowAnimController.h"
#include "Cameras/ABikePhysicsCameraHelper.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AAlienSteer.h"
#include "UnitMovement/Steering/AFallingCowSteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"

#include <Input/TInputInterface.h>
#include <Render/TRenderInterface.h>

#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( bike1 );
TPSTRING8_DECLARE( bike2 );
TPSTRING8_DECLARE( bike3 );
TPSTRING8_DECLARE( bike4 );
TPSTRING8_DECLARE( bike5 );
TPSTRING8_DECLARE( bike );
TPSTRING8_DECLARE( bikerack );
TPSTRING8_DECLARE( collision_i_bikerack );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( c_neck );
TPSTRING8_DECLARE( bk_gton_rack_right );
TPSTRING8_DECLARE( bk_gton_rack_left );
TPSTRING8_DECLARE( bk_gton_right );
TPSTRING8_DECLARE( bk_gton_left );
TPSTRING8_DECLARE( bk_ftdn );
TPSTRING8_DECLARE( bk_gtoff_right );
TPSTRING8_DECLARE( bk_gtoff_left );
TPSTRING8_DECLARE( bk_collision );
TPSTRING8_DECLARE( bk_crashoffbike_loop );
TPSTRING8_DECLARE( bike_origin );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( c_l_clavicle );
TPSTRING8_DECLARE( water_respawn_goal );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( Bike );

// NOTE: GetClass 0053fd60, CreateObject 00541c10, CreateObjectInPlace 00541c70
TDEFINE_CLASS_INIT( ABikeManager, 1, 0 );

// DAT_00783ca4
AFunctionTable ABikeManager::ms_oFunctionTable( &TGetClass( ABikeManager ), TNULL );

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 005401d0
static TObject* __stdcall ABikeManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return ABikeManager::GetSingleton();
}

// $Barnyard: FUNCTION 00541dc0
void ABikeManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ABikeManagerFunctionFallback ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ABikeManager>::Create( "StartupPlayerBike", &ABikeManager::StartupPlayerBike ) );
}

// $Barnyard: FUNCTION 0053fd80
void ABikeManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00540e70
ABikeManager::ABikeManager()
{
	for ( TINT i = 0; i < MAX_NUM_RACKS; i++ )
	{
		m_aRacks[ i ].pGoal = TNULL;
		m_aRacks[ i ].pRack = TNULL;
		m_aRacks[ i ].pBike = TNULL;
	}

	m_iNumBikes       = 1;
	m_uiBikeTypes     = 0;
	m_iUnk554         = 0;
	m_pFade           = TNULL;
	m_pMainHudElement = TNULL;
	m_pGlowObject     = TNULL;
	m_fSavedNear      = 1.0f;
	m_bPlayerOnBike   = TFALSE;
	m_iLoadedBikeType = -1;
	m_iMilkDripEffect = -1;
	m_bRestartMusic   = TFALSE;
	m_bMusicStopped   = TFALSE;
	m_uiBikeTypes |= GetBikeTypeFlag( 0 );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail leaves m_eState uninitialised (harmless, OnUpdate only reads it while m_bPlayerOnBike is set)
	m_eState = STATE_NONE;
#endif

	m_aBikeTypes[ 0 ] = TPS8( bike1 );
	m_aBikeTypes[ 1 ] = TPS8( bike2 );
	m_aBikeTypes[ 2 ] = TPS8( bike3 );
	m_aBikeTypes[ 3 ] = TPS8( bike4 );
	m_aBikeTypes[ 4 ] = TPS8( bike5 );
}

// $Barnyard: FUNCTION 00541cb0
// $Barnyard: FUNCTION 00541cd0
ABikeManager::~ABikeManager()
{
}

// $Barnyard: FUNCTION 0053fd70
TBOOL ABikeManager::OnCreate()
{
	m_bWalking = TFALSE;
	return TTRUE;
}

static TBOOL IsBikeRaceRunning()
{
	return AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ABikeRaceMicroGame ) );
}

// $Barnyard: FUNCTION 00542eb0
TBOOL ABikeManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Restart the music once the stream of the ride is closed
	if ( m_bRestartMusic == TTRUE && AMultiplexStream::GetSingleton()->GetState() == AMultiplexStream::STATE_CLOSED )
	{
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
		m_bRestartMusic = TFALSE;
	}

	if ( !m_bPlayerOnBike )
	{
		ReleaseUnits();
		return TFALSE;
	}

	// The headlight isn't shown in the cutscenes
	if ( m_pGlowObject )
		m_pGlowObject->m_bEnabled = !ASSeqState::IsPlaying();

	if ( AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ASSeqState ) ) )
		return TTRUE;

	AUnit* pRider = GetRider();
	TSTATICCAST( AControlComponent, pRider->FindComponentExact( &TGetClass( APlayerControlComponent ) ) )->SetEnabled( TFALSE );

	TBOOL bUnk = TTRUE;

	switch ( m_eState )
	{
		case STATE_GETON_RACK:
		{
			AAnimatableObject* pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, GetPlayerBike() )->GetAnimatableObject();
			ABikeSteer*        pSteer            = TSTATICCAST( ABikeSteer, GetPlayerBike()->GetSteer() );

			if ( !pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gton_rack_right ), 0.0f ) && !pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gton_rack_left ), 0.0f ) )
			{
				// Got on the bike, take it from the rack
				pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, GetPlayerBike() )->GetAnimatableObject();
				pPlayerBikeObject->KillAllAnimations();
				pSteer->SetPosition( m_vecRackPosition );
				pSteer->SetDirection( m_vecRackDirection );

				m_eState = STATE_RIDING;
				pSteer->Start();
				pSteer->GetPhysics()->SetFrozen( TTRUE );
				pPlayerBikeObject->PlayAnimation( TPS8( bk_ftdn ) );
				pPlayerBikeObject->Update( 0.0f );
				AttachRideCamera( 0.5f );

				// The units of the bike and of the rider aren't used during the ride
				GetBike()->GetSteer()->SetPosition( TVector4::VEC_ZERO );
				GetRider()->GetSteer()->SetPosition( TVector4::VEC_ZERO );
				break;
			}

			// Remember where the bike model is while getting on it
			AAnimatableObject* pBikeObject = pPlayerBikeObject->GetAttachedObjects().Begin().Get();
			AModelInstance*    pBikeModel  = pBikeObject->GetModelInstance();
			TINT               iBone       = pBikeModel->GetBoneIndex( TPS8( bike_origin ) );

			TMatrix44 mBone;
			pBikeObject->GetModelInstance()->GetBoneTransform( iBone, mBone );

			TVector4 vecUp = mBone.AsBasisVector4( BASISVECTOR_UP );

			m_vecRackPosition    = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
			m_vecRackDirection.x = -vecUp.x;
			m_vecRackDirection.y = -vecUp.y;
			m_vecRackDirection.z = -vecUp.z;
			m_vecRackDirection.w = 1.0f;

			return TTRUE;
		}
		case STATE_WALKTOBIKE:
		{
			APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, pRider->GetSteer() );

			if ( pSteer->IsInWater() )
			{
				m_eState = STATE_NONE;
				break;
			}

			// Walk to the bike
			TVector4 vecPosition;
			vecPosition.Set( *pSteer->GetPosition() );

			TVector4 vecDirection;
			vecDirection.Substract( m_vecGetOnPosition, vecPosition );
			vecDirection.y = 0.0f;
			vecDirection.Normalise();

			TVector4 vecVelocity;
			vecVelocity.Multiply( vecDirection, 3.5f );
			pSteer->SetDesiredVelocity( vecVelocity );

			TFLOAT fDistance = TVector4::DistanceXZ( m_vecGetOnPosition, vecPosition );
			TFLOAT fSpeed    = 3.0f;

			if ( m_bWalking == TTRUE )
			{
				fSpeed            = TVector4::DistanceXZ( m_vecLastPosition, *pSteer->GetPosition() ) / a_fDeltaTime;
				m_vecLastPosition = *pSteer->GetPosition();
			}

			m_bWalking = TTRUE;

			if ( fDistance >= 0.5f && m_fWalkTime >= 0.0f )
			{
				if ( m_fWalkTime >= 2.3f )
				{
					m_fWalkTime -= a_fDeltaTime;
					break;
				}

				// Stuck
				if ( fSpeed >= 2.0f )
				{
					m_fWalkTime -= a_fDeltaTime;
					break;
				}
			}

			// Reached the bike
			CreatePlayerBike();
			pSteer->SetDesiredVelocity( TVector4::VEC_ZERO );
			break;
		}
		case STATE_GETON:
		{
			AAnimatableObject* pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, GetPlayerBike() )->GetAnimatableObject();
			ABikeSteer*        pSteer            = TSTATICCAST( ABikeSteer, GetPlayerBike()->GetSteer() );

			if ( pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gton_right ), 0.0f ) ||
			     pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gton_left ), 0.0f ) ||
			     pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gton_rack_left ), 0.0f ) ||
			     pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gton_rack_right ), 0.0f ) )
				break;

			m_eState = STATE_RIDING;
			pSteer->Start();
			pSteer->GetPhysics()->SetFrozen( TTRUE );
			pPlayerBikeObject->PlayAnimation( TPS8( bk_ftdn ) );

			// The units of the bike and of the rider aren't used during the ride
			GetBike()->GetSteer()->SetPosition( TVector4::VEC_ZERO );
			GetRider()->GetSteer()->SetPosition( TVector4::VEC_ZERO );
			break;
		}
		case STATE_RIDING:
		{
			AAnimatableObject* pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, GetPlayerBike() )->GetAnimatableObject();
			ABikeSteer*        pSteer            = TSTATICCAST( ABikeSteer, GetPlayerBike()->GetSteer() );

			if ( m_pMainHudElement )
			{
				m_pMainHudElement->SetStamina( pSteer->GetStamina() );

				if ( APlayerProfileManager::GetSingleton()->GetMainProfile() )
					APlayerProfileManager::GetSingleton()->GetMainProfile()->SetStamina( 5.0f * pSteer->GetStamina() );
			}

			if ( pPlayerBikeObject->IsAnimPlaying( TPS8( bk_collision ), 0.0f ) )
			{
				m_eState = STATE_CRASH;
				TInputInterface::GetSingleton()->StopAllRepeats();
			}

			APhysicsBike* pPhysics = pSteer->GetPhysics();
			TBOOL         bInWater = pSteer->IsInWater();

			if ( pPhysics->m_bThrowRider )
			{
				if ( !bInWater )
				{
					// Thrown off the bike
					if ( pSteer->HasCrashAnim() )
					{
						m_eState = STATE_CRASH;

						AFallingCowAnimController* pController = TSTATICCAST( AFallingCowAnimController, TSTATICCAST( AAnimatedUnit, pRider )->GetAnimatableObject()->FindAnimController( &TGetClass( AFallingCowAnimController ) ) );
						pController->SetDirection( pSteer->GetPhysics()->m_eThrowDirection != APhysicsBike::THROWDIRECTION_FORWARD );
					}

					m_bWantsToGetOff = TFALSE;
					m_fGetOffTime    = 0.0f;
					break;
				}
			}
			else if ( !bInWater )
			{
				goto GetOff;
			}

			if ( !IsBikeRaceRunning() )
			{
				// Sank, fade out to respawn
				m_bWantsToGetOff = TFALSE;
				m_fGetOffTime    = 0.0f;
				TSTATICCAST( AVisibleUnit, GetPlayerBike() )->SetVisible( TFALSE );
				ACameraManager::GetSingleton()->DetachCameraHelpers();
				m_pFade  = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
				m_eState = STATE_SUNK_FADE;
				break;
			}

		GetOff:
			if ( m_bWantsToGetOff != TTRUE )
				break;

			if ( pPhysics->m_oGearBox.iGear == 1 && pPlayerBikeObject->IsAnimPlaying( TPS8( bk_ftdn ), 0.0f ) && pPhysics->m_bFrozen == TTRUE )
			{
				// Stopped, get off on the free side
				TBOOL bRight = pSteer->CanGetOff( TTRUE );
				TBOOL bCan   = ( bRight ) ? bRight == TTRUE : pSteer->CanGetOff( TFALSE ) == TTRUE;

				if ( !bCan )
				{
					ASoundManager::GetSingleton()->PlayCue( 276 );
				}
				else
				{
					pSteer->Stop( TTRUE );
					pSteer->SetSpeed( 0.0f );
					pPlayerBikeObject->PlayAnimation( bRight == TTRUE ? TPS8( bk_gtoff_right ) : TPS8( bk_gtoff_left ) );
					pPlayerBikeObject->Update( 0.0f );

					m_bWantsToGetOff = TFALSE;
					m_eState         = STATE_GETOFF;
					BlendCameraFOV( 1.0f );
				}
			}
			else if ( !pPhysics->IsWheelie() && !pPhysics->IsStoppie() )
			{
				// Brake to a stop first, give up after two seconds of standing
				if ( pPhysics->m_oGearBox.iGear != 1 || ( m_fGetOffTime += a_fDeltaTime ) <= 2.0f )
				{
					pSteer->SetStoppingToGetOff( TTRUE );
					break;
				}
			}

			m_bWantsToGetOff = TFALSE;
			m_fGetOffTime    = 0.0f;
			pSteer->SetStoppingToGetOff( TFALSE );
			break;
		}
		case STATE_GETOFF:
		{
			AAnimatableObject* pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, GetPlayerBike() )->GetAnimatableObject();

			if ( pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gtoff_right ), 0.0f ) || pPlayerBikeObject->IsAnimPlaying( TPS8( bk_gtoff_left ), 0.0f ) )
				break;

			DestroyPlayerBike( TFALSE );
			EndRide( TPS8( idle_lp ), TFALSE, TTRUE );
			DetachRideCamera( 0.5f, TFALSE );
			ReleaseUnits();
			m_eState = STATE_NONE;
			break;
		}
		case STATE_CRASH:
		{
			// The rider falls off
			ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, GetPlayerBike()->GetSteer() );

			TVector4 vecVelocity;
			vecVelocity.Set( pSteer->GetPhysics()->m_vThrowVelocity );
			TVector4 vecDirection;
			vecDirection.Set( pSteer->GetPhysics()->m_vThrowDirection );
			TVector4 vecPrevPosition;
			vecPrevPosition.Set( pSteer->GetLastPosition() );

			pRider->ChangeSteer( &TGetClass( AFallingCowSteer ) );
			pRider->SetMovement( TTRUE );

			AAnimatableObject* pRiderObject = TSTATICCAST( AAnimatedUnit, pRider )->GetAnimatableObject();
			pRiderObject->SetAnimControllersDisabled( TTRUE );
			pRiderObject->FindAnimController( &TGetClass( AFallingCowAnimController ) )->SetDisabled( TFALSE );

			DestroyPlayerBike( TTRUE );
			DetachRideCamera( 0.0f, TFALSE );
			ABikeHelpers::SetupFallingCow( GetRider(), vecVelocity, vecDirection, vecPrevPosition );

			// Stop the music of the ride
			// NOTE: the Wii (802a1e0c) also only checks Bike, so with the Wii's Bike02 the music keeps playing after a fall
			if ( AMusicManager::GetSingleton()->GetCurrentStream() == TPS8( Bike ) )
			{
				ASoundManager::GetSingleton()->PlayCue( 262 );
				AMusicManager::GetSingleton()->SetEnabled( TFALSE, TFALSE );
				m_bMusicStopped = TTRUE;
			}
			else
			{
				m_bMusicStopped = TFALSE;
			}

			m_eState = STATE_FALLING;
			break;
		}
		case STATE_FALLING:
		{
			AFallingCowAnimController* pController = TSTATICCAST( AFallingCowAnimController, TSTATICCAST( AAnimatedUnit, pRider )->GetAnimatableObject()->FindAnimController( &TGetClass( AFallingCowAnimController ) ) );
			AFallingCowSteer*          pSteer      = TSTATICCAST( AFallingCowSteer, GetRider()->GetSteer() );

			if ( pController->GetState() == AFallingCowAnimController::STATE_DONE )
			{
				// Got up
				if ( m_bMusicStopped == TTRUE )
				{
					m_bMusicStopped = TFALSE;
					m_bRestartMusic = TTRUE;
				}

				TBOOL bSunk = TSTATICCAST( ABikeSteer, GetBike()->GetSteer() )->IsSunk();

				if ( !IsBikeRaceRunning() )
				{
					EndRide( TPS8( idle_lp ), TFALSE, TFALSE );
					DetachRideCamera( 0.5f, TFALSE );

					// The bike sank, put it back to a rack
					if ( bSunk == TTRUE && m_bPlayerOnBike != TTRUE )
						ParkAllBikes();
				}

				ReleaseUnits();
				m_eState = STATE_NONE;
				break;
			}

			if ( pSteer->IsInWater() == TTRUE && !IsBikeRaceRunning() )
			{
				// Fell into the water, fade out to respawn
				if ( m_bMusicStopped == TTRUE )
				{
					m_bRestartMusic = TTRUE;
					m_bMusicStopped = TFALSE;
				}

				ACameraManager::GetSingleton()->DetachCameraHelpers();
				m_pFade  = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
				m_eState = STATE_FELL_FADE;
			}

			break;
		}
		case STATE_SUNK_FADE:
		case STATE_FELL_FADE:
		{
			TVector4 vecPosition;
			vecPosition.Set( TVector4::VEC_ZERO );

			TBOOL bSunk;

			if ( m_eState == STATE_SUNK_FADE )
			{
				ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, GetPlayerBike()->GetSteer() );
				bSunk              = pSteer->IsSunk();
				vecPosition        = *pSteer->GetPosition();
			}
			else
			{
				AFallingCowSteer* pSteer = TSTATICCAST( AFallingCowSteer, GetRider()->GetSteer() );
				bSunk                    = pSteer->IsSunk();
				vecPosition              = *pSteer->GetPosition();
			}

			if ( m_pFade && !m_pFade->IsStillFading( TTRUE ) )
			{
				AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
				m_pFade = TNULL;
			}

			if ( bSunk != TTRUE )
				break;

			// Respawn at the closest water respawn goal
			TVector4 vecRespawnPosition;
			TVector4 vecRespawnDirection;
			GetRespawnPosition( vecPosition, vecRespawnPosition, vecRespawnDirection );

			if ( m_eState == STATE_SUNK_FADE )
				DestroyPlayerBike( TFALSE );

			GetRider()->GetSteer()->SetPosition( vecRespawnPosition );
			GetRider()->GetSteer()->SetDirection( vecRespawnDirection );

			EndRide( TPS8( yuk ), TTRUE, TFALSE );
			DetachRideCamera( 0.0f, TTRUE );

			if ( m_bPlayerOnBike != TTRUE )
				ParkAllBikes();

			ReleaseUnits();
			m_eState = STATE_NONE;
			break;
		}
		case STATE_NONE:
		{
			if ( m_pRiderProxy && m_pRiderProxy->GetUnit() )
			{
				AControlComponent* pControlComponent = TSTATICCAST( AControlComponent, m_pRiderProxy->GetUnit()->FindComponentExact( &TGetClass( APlayerControlComponent ) ) );

				if ( pControlComponent )
					pControlComponent->SetEnabled( TTRUE );
			}

			ReleaseUnits();
			m_bPlayerOnBike = TFALSE;
			break;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00541070
AFunctionTable* ABikeManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00541280
void ABikeManager::StartupPlayerBike()
{
	if ( m_bPlayerOnBike == TTRUE )
		return;

	m_bPlayerOnBike = TTRUE;

	if ( AGameLoader::IsMainGameLoaded() )
	{
		ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

		if ( pSimState )
		{
			// Hides the target reticles of the milk squirt
			pSimState->GetMilkReticles().SetActive( TFALSE );
		}
	}

	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	AUnit* pBike     = TNULL;
	TINT   iBikeType = 0;
	FindClosestBike( *pPlayer->GetSteer()->GetPosition(), pBike, iBikeType );

	TBOOL bBikeInWater = TFALSE;

	if ( pBike->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
		bBikeInWater = TSTATICCAST( ABikeSteer, pBike->GetSteer() )->IsInWater();

	if ( bBikeInWater || TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->IsInWater() )
	{
		m_bPlayerOnBike = TFALSE;
		return;
	}

	if ( !IsBikeRaceRunning() && !ABikeHelpers::CanReachBike( pBike, pPlayer ) )
	{
		ASoundManager::GetSingleton()->PlayCue( 276 );
		m_eState        = STATE_NONE;
		m_bPlayerOnBike = TFALSE;
		return;
	}

	// Walk to the side of the bike the player is on
	TVector4 vecPlayerPos = *pPlayer->GetSteer()->GetPosition();
	TVector4 vecBikePos   = *pBike->GetSteer()->GetPosition();
	TVector4 vecBikeDir   = *pBike->GetSteer()->GetDirection();

	TVector4 vecSide;
	vecSide.CrossProduct( vecBikeDir, TVector4::VEC_NEGY );

	TVector4 vecToBike;
	vecToBike.Substract( vecBikePos, vecPlayerPos );
	vecToBike.Normalise();

	TVector4 vecOffset;
	vecOffset.Multiply( vecSide, 0.75f );

	if ( TVector4::DotProduct3( vecSide, vecToBike ) >= 0.0f )
		m_vecGetOnPosition.Substract( vecBikePos, vecOffset );
	else
		m_vecGetOnPosition.Add( vecBikePos, vecOffset );

	m_pRiderProxy     = pPlayer->GetSelfProxy();
	m_vecLastPosition = *pPlayer->GetSteer()->GetPosition();
	m_fWalkTime       = 3.0f;
	m_bWalking        = TFALSE;
	m_eState          = STATE_WALKTOBIKE;

	AVehicleManager::SetUseTriangleNormal( TTRUE );
	AVehicleManager::SetSlideOnObjects( TTRUE );
}

// $Barnyard: FUNCTION 0053fd90
TBOOL ABikeManager::IsBikeParked( AUnit* a_pBike ) const
{
	for ( TINT i = 0; i < m_iNumBikes; i++ )
	{
		if ( m_aBikes[ i ].pUnit == a_pBike )
			return m_aBikes[ i ].bParked;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0053fdd0
void ABikeManager::SetBikeParked( AUnit* a_pBike, TBOOL a_bParked )
{
	for ( TINT i = 0; i < m_iNumBikes; i++ )
	{
		if ( m_aBikes[ i ].pUnit == a_pBike )
		{
			m_aBikes[ i ].bParked = a_bParked;
			return;
		}
	}
}

// $Barnyard: FUNCTION 0053fe10
void ABikeManager::FindClosestBike( const TVector4& a_rcPosition, AUnit*& a_rpBike, TINT& a_riType ) const
{
	TFLOAT fClosestDistanceSq = 1000000.0f;

	for ( TINT i = 0; i < m_iNumBikes; i++ )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: the constructor sets m_iNumBikes to 1 without a bike, retail crashes if it's called before the bikes are spawned
		if ( !m_aBikes[ i ].pUnit )
			continue;
#endif

		const TVector4* pBikePosition = m_aBikes[ i ].pUnit->GetSteer()->GetPosition();

		TFLOAT fDeltaZ     = a_rcPosition.z - pBikePosition->z;
		TFLOAT fDeltaX     = a_rcPosition.x - pBikePosition->x;
		TFLOAT fDistanceSq = fDeltaZ * fDeltaZ + fDeltaX * fDeltaX;

		if ( fDistanceSq < fClosestDistanceSq )
		{
			fClosestDistanceSq = fDistanceSq;
			a_rpBike           = m_aBikes[ i ].pUnit;
			a_riType           = m_aBikes[ i ].iType;
		}
	}
}

// $Barnyard: FUNCTION 0053fea0
void ABikeManager::SetBikeTypeUnlocked( BIKEFLAGS a_uiFlag, TBOOL a_bUnlocked )
{
	if ( a_bUnlocked == TTRUE )
		m_uiBikeTypes |= a_uiFlag;
	else
		m_uiBikeTypes &= ~a_uiFlag;
}

// $Barnyard: FUNCTION 0053fee0
ABikeManager::BIKEFLAGS ABikeManager::GetBikeTypeFlag( TINT a_iType )
{
	switch ( a_iType )
	{
		case 0: return 2;
		case 1: return 4;
		case 2: return 8;
		case 3: return 16;
		default: return 32;
	}
}

// $Barnyard: FUNCTION 0053ff30
void ABikeManager::SortRacksByDistance( const TVector4& a_rcPosition, TINT* a_piOutRacks )
{
	TVector4 vecPosition = a_rcPosition;
	TFLOAT   afDistances[ MAX_NUM_BIKES ];

	for ( TINT i = 0; i < m_iNumRacks; i++ )
	{
		const TVector4* pRackPosition = m_aRacks[ i ].pRack->GetSteer()->GetPosition();

		TFLOAT fDeltaX = vecPosition.x - pRackPosition->x;
		TFLOAT fDeltaZ = vecPosition.z - pRackPosition->z;

		a_piOutRacks[ i ] = i;
		afDistances[ i ]  = fDeltaZ * fDeltaZ + fDeltaX * fDeltaX;
	}

	// The closest rack bubbles to the front...
	for ( TINT i = m_iNumRacks - 1; i > 0; i-- )
	{
		if ( afDistances[ i - 1 ] > afDistances[ i ] )
		{
			TFLOAT fDistance      = afDistances[ i - 1 ];
			TINT   iRack          = a_piOutRacks[ i - 1 ];
			a_piOutRacks[ i - 1 ] = a_piOutRacks[ i ];
			afDistances[ i - 1 ]  = afDistances[ i ];
			afDistances[ i ]      = fDistance;
			a_piOutRacks[ i ]     = iRack;
		}
	}

	// ...so it stops the insertion sort of the rest
	for ( TINT i = 2; i < m_iNumRacks; i++ )
	{
		TINT   iRack     = a_piOutRacks[ i ];
		TFLOAT fDistance = afDistances[ i ];
		TINT   j         = i;

		for ( ; fDistance < afDistances[ j - 1 ]; j-- )
		{
			a_piOutRacks[ j ] = a_piOutRacks[ j - 1 ];
			afDistances[ j ]  = afDistances[ j - 1 ];
		}

		afDistances[ j ]  = fDistance;
		a_piOutRacks[ j ] = iRack;
	}
}

// $Barnyard: FUNCTION 00540160
void ABikeManager::SetWantsToGetOff( TBOOL a_bGetOff )
{
	if ( m_bPlayerOnBike && TUINT( m_eState ) > STATE_GETON )
		m_bWantsToGetOff = a_bGetOff;
}

// $Barnyard: FUNCTION 00540190
void ABikeManager::Disable()
{
	m_uiBikeTypes = GetBikeTypeFlag( 0 );

	if ( m_pGlowObject )
	{
		AGlowViewport::GetSingleton()->RemoveGlowObject( m_pGlowObject );
		m_pGlowObject = TNULL;
	}
}

// $Barnyard: FUNCTION 005401e0
TINT ABikeManager::GetBestBikeType() const
{
	TINT iBest = -1;

	for ( TINT i = 0; i < NUM_BIKE_TYPES; i++ )
	{
		if ( ( m_uiBikeTypes & GetBikeTypeFlag( i ) ) && iBest < i )
			iBest = i;
	}

	return ( iBest == -1 ) ? 0 : iBest;
}

// $Barnyard: FUNCTION 00540280
void ABikeManager::LoadBikeAssets()
{
	m_iLoadedBikeType = GetBestBikeType();

	TTRB* pTRB = AAssetLoader::GetAssetTRB( AAssetType_AssetPack );
	AMaterialLibraryManager::GetSingleton()->LoadLibrary( m_aBikeTypes[ m_iLoadedBikeType ], pTRB, TTRUE );
	AKeyFrameLibraryManager::GetSingleton()->LoadLibrary( m_aBikeTypes[ m_iLoadedBikeType ], pTRB );
	AAnimatableObjectManager::GetSingleton()->LoadAnimObjType( m_aBikeTypes[ m_iLoadedBikeType ], TNULL, TFALSE );
}

// $Barnyard: FUNCTION 005402d0
void ABikeManager::DestroyBikeAssets()
{
	if ( m_iLoadedBikeType != -1 )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyType( m_aBikeTypes[ m_iLoadedBikeType ] );
		AKeyFrameLibraryManager::GetSingleton()->UnrefLibrary( m_aBikeTypes[ m_iLoadedBikeType ] );
		AMaterialLibraryManager::GetSingleton()->UnrefLibrary( m_aBikeTypes[ m_iLoadedBikeType ], TTRUE );
		m_iLoadedBikeType = -1;
	}
}

// $Barnyard: FUNCTION 00540340
void ABikeManager::CreateBikes()
{
	m_iNumBikes     = 1;
	m_fRespawnTimer = 0.0f;

	for ( TINT i = 0; i < m_iNumBikes; i++ )
	{
		TINT iType = GetBestBikeType();

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGX, 0, -1, -1, TTRUE );
		oTransform.m_iModelIndex = iType;

		m_aBikes[ i ].pUnit   = AUnitManager::GetSingleton()->SpawnUnit( TPS8( bike ), &oTransform );
		m_aBikes[ i ].bParked = TFALSE;
		m_aBikes[ i ].iType   = iType;
		m_aBikes[ i ].iRack   = -1;
	}
}

// $Barnyard: FUNCTION 005403d0
void ABikeManager::DestroyBikes()
{
	for ( TINT i = 0; i < m_iNumBikes; i++ )
		AUnitManager::GetSingleton()->DestroyUnit( m_aBikes[ i ].pUnit );

	m_iNumBikes = 0;
}

// NOTE: inlined into RespawnBikes and OnUpdate in retail
static void ParkBikeAtRack( ABikeManager::Bike& a_rBike, const ABikeManager::Rack& a_rcRack, TINT a_iRack )
{
	ASteer* pSteer = a_rBike.pUnit->GetSteer();

	if ( pSteer->GetClass()->IsA( &TGetClass( ABikeSteer ) ) )
	{
		a_rBike.pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
		pSteer = a_rBike.pUnit->GetSteer();
	}

	ASteer* pRackSteer = a_rcRack.pRack->GetSteer();
	pSteer->SetPosition( *pRackSteer->GetPosition() );
	pSteer->SetDirection( *pRackSteer->GetDirection() );

	a_rBike.bParked = TTRUE;
	a_rBike.iRack   = a_iRack;
}

// $Barnyard: FUNCTION 00540410
void ABikeManager::RespawnBikes( TFLOAT a_fDeltaTime, TBOOL a_bForce )
{
	if ( m_bPlayerOnBike == TTRUE )
		return;

	if ( m_fRespawnTimer >= 2.0f || a_bForce )
	{
		m_fRespawnTimer = 0.0f;

		TVector4 vecPlayerPos = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();

		TINT aiRacks[ MAX_NUM_RACKS ];
		TINT aiBikes[ MAX_NUM_BIKES ];
		TINT iNumBikes = 0;

		for ( TINT i = 0; i < m_iNumBikes; i++ )
		{
			TVector4 vecBikePos;

			if ( m_aBikes[ i ].bParked || a_bForce == TTRUE ||
			     ( vecBikePos = *m_aBikes[ i ].pUnit->GetSteer()->GetPosition(), TVector4::DistanceXZ( vecPlayerPos, vecBikePos ) > 85.0f ) ||
			     TMath::Abs( vecPlayerPos.y - vecBikePos.y ) > 85.0f )
			{
				aiBikes[ iNumBikes++ ] = i;
			}
		}

		SortRacksByDistance( vecPlayerPos, aiRacks );

		for ( TINT i = 0; i < m_iNumRacks && i < iNumBikes; i++ )
			ParkBikeAtRack( m_aBikes[ aiBikes[ i ] ], m_aRacks[ aiRacks[ i ] ], aiRacks[ i ] );
	}
	else
	{
		m_fRespawnTimer += a_fDeltaTime;
	}
}

// NOTE: inlined into OnUpdate twice in retail
void ABikeManager::ParkAllBikes()
{
	m_fRespawnTimer = 0.0f;

	TVector4 vecPlayerPos = *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition();

	TINT aiRacks[ MAX_NUM_RACKS ];
	TINT aiBikes[ MAX_NUM_BIKES ];
	TINT iNumBikes = 0;

	for ( TINT i = 0; i < m_iNumBikes; i++ )
		aiBikes[ iNumBikes++ ] = i;

	SortRacksByDistance( vecPlayerPos, aiRacks );

	for ( TINT i = 0; i < m_iNumRacks && i < iNumBikes; i++ )
		ParkBikeAtRack( m_aBikes[ aiBikes[ i ] ], m_aRacks[ aiRacks[ i ] ], aiRacks[ i ] );
}

// $Barnyard: FUNCTION 00540640
void ABikeManager::BlendCameraFOV( TFLOAT a_fDuration )
{
	// Blend the FOV of the ride camera to the one of the walking camera
	TFLOAT fFOV = ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 )->GetFOV();
	TSTATICCAST( ABikePhysicsCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_BIKE ) )->BlendFOV( fFOV, a_fDuration );
}

// $Barnyard: FUNCTION 00540680
void ABikeManager::ReleaseUnits()
{
	m_pRiderProxy      = TNULL;
	m_pBikeProxy       = TNULL;
	m_pPlayerBikeProxy = TNULL;
}

// $Barnyard: FUNCTION 00540700
TINT ABikeManager::GetRideMusic()
{
	TINT iWeather = AWeatherManager::GetSingleton()->GetCurrentWeather();

	if ( AGameTimeManager::GetSingleton()->IsItNight() )
	{
		if ( iWeather == AWeatherManager::WEATHERTYPE_HEAVYRAIN || iWeather == AWeatherManager::WEATHERTYPE_LIGHTNINGSTORM )
			return soundbank::MUSIC_BY_NIGHT_RAIN_HEAVY;

		return ( iWeather != AWeatherManager::WEATHERTYPE_LIGHTRAIN ) ? soundbank::MUSIC_BY_NIGHT : soundbank::MUSIC_BY_NIGHT_RAIN_LIGHT;
	}
	else
	{
		if ( iWeather == AWeatherManager::WEATHERTYPE_HEAVYRAIN || iWeather == AWeatherManager::WEATHERTYPE_LIGHTNINGSTORM )
			return soundbank::MUSIC_BY_BIKE_RAIN_HEAVY;

		return ( iWeather == AWeatherManager::WEATHERTYPE_LIGHTRAIN ) ? soundbank::MUSIC_BY_BIKE_RAIN_LIGHT : soundbank::MUSIC_BY_BIKE;
	}
}

// $Barnyard: FUNCTION 00540770
void ABikeManager::PlayMilkDrip( AUnit* a_pUnit )
{
	AModelInstance* pModelInstance = TSTATICCAST( AVisibleUnit, a_pUnit )->GetModelInstance();

	if ( pModelInstance )
	{
		TINT      iBone = pModelInstance->GetBoneID( TPS8( c_neck ) );
		TMatrix44 mBone;
		pModelInstance->GetBoneTransform( iBone, mBone );

		TVector4 vecPosition = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
		m_iMilkDripEffect    = AParticleEffectManager::FindEffect( "milkdrip" );

		AParticleEffectHandle* pHandle = AParticleEffectManager::GetSingleton()->PlayEffect( m_iMilkDripEffect, vecPosition, TVector4::VEC_NEGY );
		pHandle->AttachToBone( pModelInstance, iBone, TTRUE, TTRUE );
	}
}

// $Barnyard: FUNCTION 00540a10
void ABikeManager::SpawnBikesAtRacks( TINT a_iNumBikes )
{
	m_iNumBikes     = a_iNumBikes;
	m_fRespawnTimer = 0.0f;

	if ( m_iNumRacks < a_iNumBikes )
		m_iNumBikes = m_iNumRacks;

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	TINT aiRacks[ MAX_NUM_BIKES ];
	SortRacksByDistance( *APlayerManager::GetSingleton()->GetPlayer( 0 )->GetSteer()->GetPosition(), aiRacks );

	for ( TINT i = 0; i < m_iNumBikes; i++ )
	{
		TINT iType = GetBestBikeType();

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGX, 0, -1, -1, TTRUE );
		oTransform.m_iModelIndex = iType;

		TINT   iRack = aiRacks[ i ];
		AUnit* pBike = AUnitManager::GetSingleton()->SpawnUnitAtGoal( TPS8( bike ), &oTransform, pNetwork, m_aRacks[ iRack ].pGoal, TNULL );

		m_aBikes[ i ].iType   = iType;
		m_aBikes[ i ].bParked = TTRUE;
		m_aBikes[ i ].pUnit   = pBike;
		m_aBikes[ i ].iRack   = iRack;

		// NOTE: retail indexes the racks by the bike here, not by iRack
		m_aRacks[ i ].pBike = pBike;
	}
}

// $Barnyard: FUNCTION 00541080
void ABikeManager::CreateRacks()
{
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	m_iNumRacks                = pNetwork->CountGoalsContaining( "bikepos" );

	TCHAR szGoalName[ 64 ];
	T2String8::Copy( szGoalName, "bikepos" );
	TCHAR* pNumber = szGoalName + T2String8::Length( szGoalName );

	for ( TINT i = 1; i < m_iNumRacks + 1; i++ )
	{
		T2String8::Format( pNumber, "%02d", i );

		AInteriorGoal* pGoal = pNetwork->FindGoal( TPString8( szGoalName ) );

		TVector4 vecGoalPosition;
		pGoal->GetPosition( vecGoalPosition );

		AUnit* pRack = AUnitManager::GetSingleton()->SpawnUnitAtGoal( TPS8( bikerack ), TVector4::VEC_ZERO, pNetwork, pGoal, TNULL );

		TVector4 vecDirection = pGoal->m_vForward;

		if ( vecDirection.MagnitudeSq() < 0.01f )
			vecDirection = TVector4::VEC_POSX;

		pRack->GetSteer()->SetDirection( vecDirection );
		AShadowManager::GetSingleton()->RemoveShadow( pRack );

		m_aRacks[ i - 1 ].pRack = pRack;
		m_aRacks[ i - 1 ].pGoal = pGoal;

		// The collision of the rack follows its bone
		AVisibleUnit* pVisibleRack = TSTATICCAST( AVisibleUnit, pRack );
		TINT          iBone        = TSTATICCAST( AAnimatedUnit, pRack )->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( collision_i_bikerack ) );
		TSTATICCAST( ACollisionObjectUnit, pVisibleRack->GetCollisionObject() )->SetBone( iBone );
	}
}

// $Barnyard: FUNCTION 00541590
void ABikeManager::EndRide( TBOOL a_bRight )
{
	EndRide( TPS8( idle_lp ), TFALSE, a_bRight );
}

// $Barnyard: FUNCTION 005415b0
void ABikeManager::AttachRideCamera( TFLOAT a_fTransitionTime )
{
	m_fSavedNear = ARenderer::GetSingleton()->GetNear();
	ARenderer::GetSingleton()->SetNear( 0.25f );

	if ( m_pPlayerBikeProxy && m_pPlayerBikeProxy->GetUnit() )
	{
		ACameraManager* pCameraManager = ACameraManager::GetSingleton();
		pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_BIKE )->SetTarget( m_pPlayerBikeProxy );

		if ( a_fTransitionTime > 0.0f )
			pCameraManager->SetSmoothTransition( pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_BIKE ), a_fTransitionTime );
	}
}

// $Barnyard: FUNCTION 00541680
void ABikeManager::DetachRideCamera( TFLOAT a_fTransitionTime, TBOOL a_bBehindRider )
{
	if ( m_pRiderProxy && m_pRiderProxy->GetUnit() )
	{
		AUnit*          pRider         = m_pRiderProxy->GetUnit();
		ACameraManager* pCameraManager = ACameraManager::GetSingleton();

		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );
		pGTAHelper->SetTarget( m_pRiderProxy );

		TVector4 vecCameraPos = pCameraManager->GetCurrentCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		TVector4 vecTarget( 0.0f, -1.2f, 0.0f, 1.0f );
		pGTAHelper->SetPosition( vecCameraPos, vecTarget, TFALSE );

		if ( a_fTransitionTime <= 0.0f )
			pCameraManager->SetSplitscreenCameraHelper( 1 );
		else
			pCameraManager->SetSmoothTransition( pGTAHelper, a_fTransitionTime );

		if ( a_bBehindRider )
		{
			pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );

			ASteer*  pRiderSteer = pRider->GetSteer();
			TVector4 vecOffset;
			vecOffset.Multiply( *pRiderSteer->GetDirection(), 5.0f );
			TVector4 vecPosition;
			vecPosition.Add( *pRiderSteer->GetPosition(), vecOffset );
			vecPosition.y -= pGTAHelper->GetHeightOffset();

			TVector4 vecRiderPos = *pRider->GetSteer()->GetPosition();
			vecRiderPos.y -= pGTAHelper->GetHeightOffset();

			pGTAHelper->SetPosition( vecPosition, vecRiderPos, TTRUE );
		}
	}

	ARenderer::GetSingleton()->SetNear( m_fSavedNear );
}

// $Barnyard: FUNCTION 00540810
void ABikeManager::GetRespawnPosition( const TVector4& a_rcPosition, TVector4& a_rOutPosition, TVector4& a_rOutDirection )
{
	AInteriorGoal* pGoal = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindClosestGoalOrFirst( TPS8( water_respawn_goal ).GetString(), a_rcPosition );

	if ( pGoal )
	{
		pGoal->GetPosition( a_rOutPosition );
		a_rOutDirection = pGoal->m_vForward;
		ABikeHelpers::AlignToGround( a_rOutPosition, a_rOutDirection, 2.0f, 40.0f );
	}
	else
	{
		// Where the bike last stood on its wheels
		TRef<AUnitProxy> pProxy = m_pPlayerBikeProxy;

		if ( !pProxy || !pProxy->GetUnit() )
			pProxy = m_pBikeProxy;

		ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, pProxy->GetUnit()->GetSteer() );

		a_rOutPosition  = pSteer->GetLastGroundPosition();
		a_rOutDirection = TVector4::VEC_POSZ;
		ABikeHelpers::AlignToGround( a_rOutPosition, a_rOutDirection, 2.0f, 40.0f );
	}
}

// $Barnyard: FUNCTION 00540b30
void ABikeManager::EndRide( const TPString8& a_rcAnim, TBOOL a_bUnused, TBOOL a_bRight )
{
	if ( !m_bPlayerOnBike )
		return;

	AGlowViewport::GlowObject* pGlowObject = m_pGlowObject;
	m_bPlayerOnBike                        = TFALSE;
	ABikeHelpers::RemoveGlowObject( pGlowObject );
	m_pGlowObject = TNULL;

	AUnit* pRider = GetRider();

	TVector4 vecPosition  = *pRider->GetSteer()->GetPosition();
	TVector4 vecDirection = *pRider->GetSteer()->GetDirection();
	vecDirection.y        = 0.0f;
	vecDirection.Normalise();

	// The rider walks again
	if ( pRider->GetSteer()->GetClass() != &TGetClass( APlayerSteer ) && pRider->GetSteer()->GetClass() != &TGetClass( AAlienSteer ) )
		pRider->ChangeSteer( &TGetClass( AAlienSteer ) );

	pRider->SetMovement( TTRUE );
	pRider->SetAIEnabled( TTRUE );

	ABikeHelpers::PushOutOfCollision( vecPosition, 0.55f );

	if ( a_bRight && m_pBikeProxy && m_pBikeProxy->GetUnit() )
	{
		// Stand on the ground next to the bike
		TVector4 vecSide;
		vecSide.CrossProduct( TVector4::VEC_NEGY, vecDirection );

		TVector4 vecToBike;
		vecToBike.Substract( *GetBike()->GetSteer()->GetPosition(), vecPosition );
		vecToBike.y = 0.0f;

		ABikeHelpers::SNAPFLAGS eFlags = ( TVector4::DotProduct3( vecSide, vecToBike ) >= 0.0f ) ?
		    ( ABikeHelpers::SNAPFLAGS_CENTRE | ABikeHelpers::SNAPFLAGS_RIGHT | ABikeHelpers::SNAPFLAGS_BACKWARD | ABikeHelpers::SNAPFLAGS_FORWARD ) :
		    ( ABikeHelpers::SNAPFLAGS_CENTRE | ABikeHelpers::SNAPFLAGS_LEFT | ABikeHelpers::SNAPFLAGS_BACKWARD | ABikeHelpers::SNAPFLAGS_FORWARD );

		ABikeHelpers::SnapToGroundAround( vecPosition, 0.4f, vecDirection, eFlags );
	}
	else
	{
		ABikeHelpers::SnapToGroundAround( vecPosition, 0.4f, TVector4::VEC_POSZ, ABikeHelpers::SNAPFLAGS_CENTRE | ABikeHelpers::SNAPFLAGS_RIGHT | ABikeHelpers::SNAPFLAGS_LEFT | ABikeHelpers::SNAPFLAGS_BACKWARD | ABikeHelpers::SNAPFLAGS_FORWARD );
	}

	ASteer* pSteer = pRider->GetSteer();
	pSteer->SetPosition( vecPosition );
	pSteer->SetDirection( vecDirection );
	pSteer->SetSpeed( 0.0f );
	pSteer->SetDesiredSpeed( 0.0f );
	pSteer->GetKinematics()->SetMeasuredSpeed( TFALSE );
	pSteer->SetDesiredDirection( vecDirection );

	// NOTE: retail writes the position into +0x58c of the steer whatever its class is (APlayerSteer::m_vLastPosition)
	TSTATICCAST( APlayerSteer, pSteer )->SetLastPosition( vecPosition );

	TSTATICCAST( AControlComponent, pRider->FindComponentExact( &TGetClass( APlayerControlComponent ) ) )->SetEnabled( TTRUE );

	AAnimatableObject* pRiderObject = TSTATICCAST( AAnimatedUnit, pRider )->GetAnimatableObject();
	pRiderObject->SetAnimControllersDisabled( TFALSE );
	pRiderObject->FindAnimController( &TGetClass( AFallingCowAnimController ) )->SetDisabled( TTRUE );
	pRiderObject->PlayAnimation( a_rcAnim );
	pRiderObject->Update( 0.0f );

	// Give the rider back what it held
	if ( m_pHeldUnit )
	{
		TSTATICCAST( AAnimatedUnit, pRider )->AttachUnit( m_pHeldUnit, TPS8( c_l_clavicle ) );
		m_pHeldUnit = TNULL;
	}

	if ( pRider->FindComponent( &TGetClass( AInteractionComponent ) ) )
		TSTATICCAST( AInteractionComponent, pRider->FindComponent( &TGetClass( AInteractionComponent ) ) )->LeaveSequence( TFALSE );

	AUnit* pBike = GetBike();

	if ( pBike && pBike->FindComponent( &TGetClass( AInteractionComponent ) ) )
		TSTATICCAST( AInteractionComponent, pBike->FindComponent( &TGetClass( AInteractionComponent ) ) )->LeaveSequence( TFALSE );

	if ( a_rcAnim == TPS8( yuk ) )
		PlayMilkDrip( pRider );

	AVehicleManager::SetUseTriangleNormal( TFALSE );
	AVehicleManager::SetSlideOnObjects( TFALSE );
}

// $Barnyard: FUNCTION 00541850
void ABikeManager::DestroyPlayerBike( TBOOL a_bKeepBike )
{
	if ( !m_pPlayerBikeProxy || !m_pPlayerBikeProxy->GetUnit() )
		return;

	AUnit*        pPlayerBike = GetPlayerBike();
	ABikeSteer*   pSteer      = TSTATICCAST( ABikeSteer, pPlayerBike->GetSteer() );
	APhysicsBike* pPhysics    = pSteer->GetPhysics();

	TVector4    vecPosition      = pPhysics->GetBody().GetPosition();
	TQuaternion qOrientation     = pPhysics->GetBody().m_qOrientation;
	TVector4    vecPrevPosition  = pPhysics->GetBody().m_oPrevTransform.GetTranslation();
	TVector4    vecLastGroundPos = pSteer->GetLastGroundPosition();

	if ( m_pHeldUnit )
		TSTATICCAST( AAnimatedUnit, pPlayerBike )->DetachUnit( m_pHeldUnit, TTRUE, TTRUE );

	ABikeHelpers::RemoveGlowObject( m_pGlowObject );
	m_pGlowObject = TNULL;

	ABikeHelpers::MoveHeadAttachments( GetPlayerBike(), GetRider() );
	ABikeHelpers::PlaceBikeAndRider( m_pPlayerBikeProxy, m_pRiderProxy, m_pBikeProxy, !a_bKeepBike );

	if ( a_bKeepBike == TTRUE )
	{
		// The bike keeps moving
		TVector4 vecVelocity        = pPhysics->m_vThrowVelocity;
		TVector4 vecAngularVelocity = pPhysics->GetBody().m_vAngularVelocity;

		GetBike()->ChangeSteer( &TGetClass( ABikeSteer ) );

		ABikeSteer*   pBikeSteer   = TSTATICCAST( ABikeSteer, GetBike()->GetSteer() );
		APhysicsBike* pBikePhysics = pBikeSteer->GetPhysics();

		pBikePhysics->Reset( vecPosition, qOrientation );
		pBikePhysics->GetBody().m_vVelocity = vecVelocity;
		pBikePhysics->GetBody().SetAngularVelocity( vecAngularVelocity );
		pBikeSteer->Park( TTRUE );
		pBikePhysics->GetBody().SetPreviousPosition( vecPrevPosition );
		pBikeSteer->GetLastGroundPosition() = vecLastGroundPos;
	}
	else
	{
		GetBike()->ChangeSteer( &TGetClass( AStationarySteer ) );
	}

	TSTATICCAST( AAnimatedUnit, GetBike() )->GetAnimatableObject()->Update( 0.0f );

	AUnitManager::GetSingleton()->DestroyUnit( GetPlayerBike() );
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	m_pPlayerBikeProxy = TNULL;

	// The rider is the player again
	APlayerSet* pPlayerSet                       = APlayerManager::GetSingleton()->GetCurrentPlayerSet();
	pPlayerSet->GetPlayerSlot( 0 )->m_pUnitProxy = TNULL;
	pPlayerSet->SetPlayerUnit( GetRider(), 0 );

	ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

	if ( pMiniMap )
		pMiniMap->SetPlayer( m_pRiderProxy );

	AInstanceLightingHelper::GetSingleton()->Update();
}

// $Barnyard: FUNCTION 00541df0
void ABikeManager::CreatePlayerBike()
{
	m_bWantsToGetOff = TFALSE;
	m_fGetOffTime    = 0.0f;

	// The player stops being controlled
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	pPlayer->SetMovement( TFALSE );
	pPlayer->SetAIEnabled( TFALSE );
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->FindAnimController( &TGetClass( AFallingCowAnimController ) )->SetDisabled( TTRUE );
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->SetSkeletonUpdating( TFALSE, TTRUE );
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->KillAllAnimations();

	m_pRiderProxy = pPlayer->GetSelfProxy();

	// Take what the player holds
	m_pHeldUnit = TSTATICCAST( AAnimatedUnit, pPlayer )->GetAttachedUnit( TPS8( c_l_clavicle ), -1 );

	if ( m_pHeldUnit )
		TSTATICCAST( AAnimatedUnit, pPlayer )->DetachUnit( m_pHeldUnit, TTRUE, TTRUE );

	APlayerSet* pPlayerSet                       = APlayerManager::GetSingleton()->GetCurrentPlayerSet();
	pPlayerSet->GetPlayerSlot( 0 )->m_pUnitProxy = TNULL;

	// The cow riding the bike becomes the player
	ASteer*        pPlayerSteer = pPlayer->GetSteer();
	AUnitTransform oTransform( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection(), 0, 0, -1, TTRUE );

	AUnit* pBike     = TNULL;
	TINT   iBikeType = 0;
	FindClosestBike( *pPlayer->GetSteer()->GetPosition(), pBike, iBikeType );

	AUnit* pPlayerBike;

	if ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_FEMALE )
		pPlayerBike = AUnitManager::GetSingleton()->CreateUnit( AUnitManager::GetSingleton()->FindUnit( "american_f_bike" ), &oTransform );
	else
		pPlayerBike = AUnitManager::GetSingleton()->CreateUnit( AUnitManager::GetSingleton()->FindUnit( "american_m_bike" ), &oTransform );

	pPlayerSet->SetPlayerUnit( pPlayerBike, 0 );

	if ( TSTATICCAST( AVisibleUnit, pPlayerBike )->GetCollisionObject() )
		TSTATICCAST( AVisibleUnit, pPlayerBike )->GetCollisionObject()->SetType( ACollisionObject::TYPE_PLAYER );

	if ( m_pHeldUnit )
		TSTATICCAST( AAnimatedUnit, pPlayerBike )->AttachUnit( m_pHeldUnit, TPS8( c_l_clavicle ) );

	TSTATICCAST( AAnimatedUnit, pPlayerBike )->GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

	ABikeHelpers::MoveHeadAttachments( pPlayer, pPlayerBike );
	ABikeHelpers::AttachBikeModel( pPlayerBike, iBikeType );

	ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, pPlayerBike->GetSteer() );
	pSteer->SetBikeType( iBikeType );

	if ( m_pGlowObject )
		ABikeHelpers::RemoveGlowObject( m_pGlowObject );

	m_pGlowObject      = ABikeHelpers::CreateHeadlightGlow( pPlayerBike );
	m_pPlayerBikeProxy = pPlayerBike->GetSelfProxy();

	// Where the bike is
	TVector4    vecBikePosition  = *pBike->GetSteer()->GetPosition();
	TVector4    vecBikeDirection = *pBike->GetSteer()->GetDirection();
	TVector4    vecPrevPosition  = vecBikePosition;
	TQuaternion qPrevOrientation = TQuaternion::IDENTITY;
	TVector4    vecFrontWheel    = TVector4::VEC_ZERO;
	TVector4    vecRearWheel     = TVector4::VEC_ZERO;
	TVector4    vecLastGroundPos = TVector4::VEC_ZERO;
	TBOOL       bHadPhysics      = TFALSE;

	if ( pBike->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
	{
		// The bike is moving (it was left after a crash), take over its physics
		ABikeSteer*   pBikeSteer = TSTATICCAST( ABikeSteer, pBike->GetSteer() );
		APhysicsBike* pPhysics   = pBikeSteer->GetPhysics();

		TVector4 vecDown;
		vecDown.Multiply( TVector4::VEC_NEGY, pPhysics->m_aWheels[ 0 ].m_oParams.fRadius );

		TVector4 vecOffset;
		vecOffset.x = vecDown.x + pPhysics->GetBody().m_vCentreOfMassLocal.x;
		vecOffset.y = vecDown.y + pPhysics->GetBody().m_vCentreOfMassLocal.y;
		vecOffset.z = vecDown.z + pPhysics->GetBody().m_vCentreOfMassLocal.z;
		vecOffset.w = pPhysics->GetBody().m_vCentreOfMassLocal.w;

		TMatrix44::TransformVector( vecBikePosition, pPhysics->GetBody().GetTransform(), vecOffset );
		qPrevOrientation = pPhysics->GetBody().m_qOrientation;
		vecLastGroundPos = pBikeSteer->GetLastGroundPosition();
		bHadPhysics      = TTRUE;

		TVector4 vecRotated;
		TMatrix44::RotateVector( vecRotated, pPhysics->GetBody().GetTransform(), vecOffset );

		vecFrontWheel.x = vecRotated.x + pPhysics->m_aWheels[ 0 ].m_vPosition.x;
		vecFrontWheel.y = vecRotated.y + pPhysics->m_aWheels[ 0 ].m_vPosition.y;
		vecFrontWheel.z = vecRotated.z + pPhysics->m_aWheels[ 0 ].m_vPosition.z;
		vecFrontWheel.w = pPhysics->m_aWheels[ 0 ].m_vPosition.w;

		vecRearWheel.x = vecRotated.x + pPhysics->m_aWheels[ 1 ].m_vPosition.x;
		vecRearWheel.y = vecRotated.y + pPhysics->m_aWheels[ 1 ].m_vPosition.y;
		vecRearWheel.z = vecRotated.z + pPhysics->m_aWheels[ 1 ].m_vPosition.z;
		vecRearWheel.w = pPhysics->m_aWheels[ 1 ].m_vPosition.w;
	}

	// The bike unit isn't used during the ride
	pBike->ChangeSteer( &TGetClass( AStationarySteer ) );
	m_pBikeProxy = pBike->GetSelfProxy();
	TSTATICCAST( AAnimatedUnit, pBike )->GetAnimatableObject()->SetSkeletonUpdating( TFALSE, TTRUE );

	if ( TSTATICCAST( AVisibleUnit, pBike )->GetCollisionObject() )
		TSTATICCAST( AVisibleUnit, pBike )->GetCollisionObject()->SetType( ACollisionObject::TYPE_PLAYER );

	// Which side of the bike the player is on
	TVector4 vecPlayerModelPos;
	TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->GetModelInstance()->GetPosition( vecPlayerModelPos );

	AModelInstance* pBikeModel = TSTATICCAST( AAnimatedUnit, pBike )->GetAnimatableObject()->GetModelInstance();

	TVector4 vecBikeModelPos;
	pBikeModel->GetPosition( vecBikeModelPos );

	TMatrix44 mBike;
	pBikeModel->GetLocalTransform( mBike );

	TVector4 vecRow1 = mBike.AsBasisVector4( BASISVECTOR_UP );
	mBike.AsBasisVector4( BASISVECTOR_RIGHT ).Multiply( mBike.AsBasisVector4( BASISVECTOR_RIGHT ), -1.0f );
	mBike.AsBasisVector4( BASISVECTOR_UP ).Multiply( mBike.AsBasisVector4( BASISVECTOR_FORWARD ), -1.0f );
	mBike.AsBasisVector4( BASISVECTOR_FORWARD ).Multiply( vecRow1, -1.0f );

	TVector4 vecToPlayer;
	vecToPlayer.x = vecPlayerModelPos.x - vecBikeModelPos.x;
	vecToPlayer.y = vecPlayerModelPos.y - vecBikeModelPos.y;
	vecToPlayer.z = vecPlayerModelPos.z - vecBikeModelPos.z;
	vecToPlayer.w = vecPlayerModelPos.w;

	TBOOL bLeft = TFALSE;

	const TVector4& rcAxis = ( TMath::Abs( TVector4::DotProduct3( TVector4::VEC_POSY, mBike.AsBasisVector4( BASISVECTOR_FORWARD ) ) ) >= 0.95f ) ? mBike.AsBasisVector4( BASISVECTOR_UP ) : mBike.AsBasisVector4( BASISVECTOR_FORWARD );

	TVector4 vecSide;
	vecSide.CrossProduct( TVector4::VEC_POSY, rcAxis );

	if ( TVector4::DotProduct3( vecSide, vecToPlayer ) < 0.0f )
		bLeft = TTRUE;

	// Get on
	TBOOL bParked = TFALSE;

	for ( TINT i = 0; i < m_iNumBikes; i++ )
	{
		if ( pBike == m_aBikes[ i ].pUnit )
		{
			bParked = m_aBikes[ i ].bParked;
			break;
		}
	}

	if ( bParked )
	{
		m_eState = STATE_GETON_RACK;
		TSTATICCAST( AAnimatedUnit, pPlayerBike )->GetAnimatableObject()->PlayAnimation( bLeft ? TPS8( bk_gton_rack_left ) : TPS8( bk_gton_rack_right ) );
		SetBikeParked( pBike, TFALSE );
	}
	else
	{
		TBOOL bBikeRace = IsBikeRaceRunning();
		m_eState        = STATE_GETON;

		if ( bBikeRace )
		{
			AttachRideCamera( 0.5f );
			TSTATICCAST( AAnimatedUnit, pPlayerBike )->GetAnimatableObject()->PlayAnimation( TPS8( bk_ftdn ) );
		}
		else
		{
			AttachRideCamera( 0.5f );

			if ( !IsBikeRaceRunning() )
				TSTATICCAST( AAnimatedUnit, pPlayerBike )->GetAnimatableObject()->PlayAnimation( bLeft ? TPS8( bk_gton_left ) : TPS8( bk_gton_right ) );
		}
	}

	// Put the bike on the ground
	if ( bHadPhysics )
	{
		TBOOL bFront = ABikeHelpers::SnapToGround( vecFrontWheel );
		TBOOL bRear  = ABikeHelpers::SnapToGround( vecRearWheel );

		if ( bFront && bRear )
		{
			TVector4 vecSum;
			vecSum.Add( vecFrontWheel, vecRearWheel );
			vecBikePosition.Multiply( vecSum, 0.5f );

			vecBikeDirection.Substract( vecFrontWheel, vecRearWheel );

			if ( vecBikeDirection.MagnitudeSq() < 0.09f )
			{
				TVector4 vecDirection;
				vecDirection.Multiply( mBike.AsBasisVector4( BASISVECTOR_UP ), -1.0f );

				TVector4 vecPosition = vecBikePosition;

				if ( ABikeHelpers::AlignToGround( vecPosition, vecDirection, 2.0f, 40.0f ) )
					vecBikeDirection = vecDirection;
			}

			vecBikeDirection.w = 1.0f;
			vecBikeDirection.Normalise();
		}
		else
		{
			vecBikePosition = *pBike->GetSteer()->GetPosition();
		}
	}
	else
	{
		TBOOL bOnGround;

		if ( !bParked )
		{
			TVector4 vecOffset;
			vecOffset.Multiply( vecBikeDirection, 0.12f + 0.58f );
			TVector4 vecFront;
			vecFront.Add( vecBikePosition, vecOffset );

			vecOffset.Multiply( vecBikeDirection, 0.58f - 0.12f );
			TVector4 vecRear;
			vecRear.Substract( vecBikePosition, vecOffset );

			TBOOL bFront = ABikeHelpers::SnapToGround( vecFront );
			TBOOL bRear  = ABikeHelpers::SnapToGround( vecRear );

			bOnGround = bFront && bRear;

			if ( bOnGround )
			{
				TVector4 vecSum;
				vecSum.Add( vecFront, vecRear );
				vecBikePosition.Multiply( vecSum, 0.5f );

				vecBikeDirection.Substract( vecFront, vecRear );
				vecBikeDirection.w = 1.0f;
				vecBikeDirection.Normalise();
			}
		}
		else
		{
			TVector4 vecOffset;
			vecOffset.Multiply( vecBikeDirection, 0.12f );

			vecBikePosition.x = vecOffset.x + vecBikePosition.x;
			vecBikePosition.y = vecOffset.y + vecBikePosition.y;
			vecBikePosition.z = vecOffset.z + vecBikePosition.z;

			bOnGround = ABikeHelpers::AlignToGround( vecBikePosition, vecBikeDirection, 2.0f, 40.0f );
		}

		if ( !bOnGround )
			vecBikePosition = *pBike->GetSteer()->GetPosition();

		TVector4 vecUp;
		vecUp.Multiply( TVector4::VEC_NEGY, 0.75f );

		pSteer->GetLastGroundPosition().x = vecUp.x + vecBikePosition.x;
		pSteer->GetLastGroundPosition().y = vecUp.y + vecBikePosition.y;
		pSteer->GetLastGroundPosition().z = vecUp.z + vecBikePosition.z;
		pSteer->GetLastGroundPosition().w = vecBikePosition.w;
	}

	pSteer->SetPosition( vecBikePosition );
	pSteer->SetDirection( vecBikeDirection );
	pSteer->Stop( TTRUE );

	if ( bHadPhysics )
	{
		pSteer->GetLastGroundPosition() = vecLastGroundPos;
		pSteer->GetPhysics()->GetBody().SetPreviousOrientation( qPrevOrientation );
		pSteer->GetPhysics()->GetBody().SetPreviousPosition( vecPrevPosition );
	}

	if ( TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject()->HasAnimation( TPS8( bk_crashoffbike_loop ) ) )
	{
		pSteer->SetHasCrashAnim( TTRUE );
		pSteer->GetPhysics()->m_bFlipRecovery = TFALSE;
	}
	else
	{
		pSteer->SetHasCrashAnim( TFALSE );
		pSteer->GetPhysics()->m_bFlipRecovery = TTRUE;
	}

	AAnimatableObject* pPlayerBikeObject = TSTATICCAST( AAnimatedUnit, pPlayerBike )->GetAnimatableObject();
	pPlayerBikeObject->Update( 0.0f );
	pPlayerBikeObject->UpdateAttachedObjectTransforms();

	m_pMainHudElement = AHudManager::GetSingleton()->FindElement<ASimPlayerMainHudElement>();

	if ( APlayerProfileManager::GetSingleton()->GetMainProfile() )
		pSteer->SetStamina( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetStamina() / 5.0f );

	ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

	if ( pMiniMap )
		pMiniMap->SetPlayer( m_pPlayerBikeProxy );

	AInstanceLightingHelper::GetSingleton()->Update();
}
