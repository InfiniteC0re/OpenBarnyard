#include "pch.h"
#include <Render/TCameraObject.h>
#include "ABikeRaceMicroGame.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "FX/AAniEffectManager.h"
#include "FX/ASwirlTrackHudAniEffect.h"
#include "FX/AFadeAniEffect.h"
#include "GUI/AFadeManager.h"
#include "GUI/AFade.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/APlayerProgress.h"
#include "HUD/AHudManager.h"
#include "Render/ARenderer.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ABikeControlComponent.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/Steering/AFallingCowSteer.h"
#include "Physics/APhysicsBike.h"

#include <Math/TMath.h>
#include <Math/TQuaternion.h>
#include <Render/TRenderContext.h>
#include <Toshi/TScheduler.h>
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( fall );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( win );

// The taunts of the rivals when they overtake the player (0x00722fa0)
static const ASoundId s_aTauntCues[ ABikeRaceMicroGame::MAX_RIVALS ] = { 1953, 1955, 1954 };

// Rotation speed of the pick ups (0x007838f4, set on the first use)
static TFLOAT s_fPickupPhase = TMath::PI * ( 1.0f / 6.0f );

// Throttle of the AI (ABikeSteer::SetSeekSpeed)
static constexpr TFLOAT THROTTLE_COAST   = 0.0f;
static constexpr TFLOAT THROTTLE_BOOST   = 2.0f;
static constexpr TFLOAT THROTTLE_REVERSE = -1.0f;

static TINT IntAbs( TINT a_iValue )
{
	return ( a_iValue < 0 ) ? -a_iValue : a_iValue;
}

static TBOOL IsModelVisible( AAnimatableObject* a_pObject )
{
	AModelInstance* pModelInstance = a_pObject->GetModelInstance();
	// NOTE: retail tests the skeleton-updating bit (0x1), not the visible bit
	return pModelInstance && pModelInstance->IsUpdatingSkeleton();
}

// $Barnyard: FUNCTION 005286a0
void ABikeRaceMicroGame::UpdateRubberBand( TINT a_iBiker, TINT a_iOther, TINT a_iOffset )
{
	Biker&       rBiker = m_aBikers[ a_iBiker ];
	const Biker& rOther = m_aBikers[ a_iOther ];
	Rival&       rRival = m_aRivals[ a_iBiker - 1 ];

	// How many waypoints the biker is behind the other one (negative: ahead)
	TINT iBehind = a_iOffset + rOther.iWaypoint + m_iNumWaypoints * ( rOther.iLap - rBiker.iLap ) - rBiker.iWaypoint;

	AAnimatedUnit* pUnit  = TSTATICCAST( AAnimatedUnit, GetBikerUnit( a_iBiker ) );
	ABikeSteer*    pSteer = TSTATICCAST( ABikeSteer, pUnit->GetSteer() );

	if ( iBehind > 5 && !pUnit->IsInCameraFrustum() )
	{
		// Far behind and not seen, move the bike forwards
		TBOOL bNextLap  = TFALSE;
		TINT  iWaypoint = rBiker.iWaypoint + iBehind - 5;

		if ( iWaypoint >= m_iNumWaypoints )
		{
			iWaypoint -= m_iNumWaypoints;
			bNextLap = TTRUE;
		}

		const Biker& rPlayer         = m_aBikers[ 0 ];
		TBOOL        bAwayFromPlayer = IntAbs( iWaypoint - rPlayer.iWaypoint ) % m_iNumWaypoints >= 2;
		TINT         iLapStart       = m_iNumWaypoints * rBiker.iLap;
		TINT         iPlayerWaypoint = rPlayer.iWaypoint + m_iNumWaypoints * rPlayer.iLap;

		// Don't jump over the player
		if ( ( iLapStart + rBiker.iWaypoint > iPlayerWaypoint || iWaypoint + iLapStart < iPlayerWaypoint ) && bAwayFromPlayer )
		{
			AModelInstance* pModelInstance = pUnit->GetModelInstance();
			TModelLOD&      rLOD           = pModelInstance->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 );

			TVector4 vecPosition;
			TVector4 vecDirection;
			GetRespawnPoint( TNULL, vecPosition, vecDirection, iWaypoint );

			// Would the bike be seen there?
			// NOTE: retail copies the matrix of the transform object directly
			TMatrix44 matTransform                                 = TREINTERPRETCAST( const TMatrix44&, ATerrainInterface::GetSingleton()->GetLocatorList()->GetWorldTransform() );
			matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecPosition;

			TVector4 vecSphere;
			vecSphere.w = rLOD.BoundingSphere.GetRadius() * 1.5f;
			TMatrix44::TransformVector( vecSphere.AsVector3(), matTransform, rLOD.BoundingSphere.GetOrigin() );

			if ( !TRenderContext::CullSphereToFrustumSimple( vecSphere, ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext()->GetWorldPlanes(), 6 ) )
			{
				if ( bNextLap )
					rBiker.iLap++;

				rBiker.iWaypoint = iWaypoint;
				rBiker.iSegment  = iWaypoint / SEGMENT_SIZE + 1;

				pSteer->SetPosition( vecPosition );
				pSteer->SetDirection( vecDirection );

				rBiker.iWaypoint++;
				rRival.eState = RIVALSTATE_RACING;
				pSteer->SetSeekTarget( GetSeekTarget( a_iBiker ) );
				rRival.bAhead = TFALSE;
			}
		}
	}

	if ( rRival.eState == RIVALSTATE_REVERSING || rRival.eState == RIVALSTATE_RECOVERING || rRival.eState == RIVALSTATE_STUCK )
		return;

	const APhysicsBikeParams& rParams = pSteer->GetPhysics()->m_oParams;

	if ( rRival.eState == RIVALSTATE_SLOWED )
	{
		// Keep coasting until slow enough
		if ( iBehind >= -5 )
		{
			if ( rParams.fMaxSpeed * 0.6f <= pSteer->GetSpeed() )
			{
				pSteer->SetSeekSpeed( THROTTLE_COAST );
				return;
			}
		}
		else if ( rParams.fMaxSpeed * 0.4f <= pSteer->GetSpeed() )
		{
			pSteer->SetSeekSpeed( THROTTLE_COAST );
			return;
		}

		rRival.eState = RIVALSTATE_RACING;
		return;
	}

	if ( ( iBehind < -5 && rParams.fMaxSpeed * 0.6f < pSteer->GetSpeed() ) ||
	     ( iBehind < -2 && rParams.fMaxSpeed * 0.8f < pSteer->GetSpeed() ) )
	{
		// Too far ahead, slow down
		pSteer->SetSeekSpeed( THROTTLE_COAST );
		rRival.eState = RIVALSTATE_SLOWED;
	}
	else if ( iBehind > 1 && pSteer->GetStamina() > 0.0f )
	{
		// Behind, boost
		if ( iBehind > 3 )
			pSteer->SetStamina( 0.1f );

		pSteer->SetSeekSpeed( THROTTLE_BOOST );
	}
}

// $Barnyard: FUNCTION 00528a80
void ABikeRaceMicroGame::UpdatePlayerFall( TFLOAT a_fDeltaTime )
{
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetBikerUnit( 0 ) );
	ASteer*        pSteer  = pPlayer->GetSteer();

	if ( !pSteer->IsA( &TGetClass( AFallingCowSteer ) ) )
		return;

	AFallingCowSteer* pFallSteer = TSTATICCAST( AFallingCowSteer, pSteer );

	if ( !m_bFallStarted )
	{
		for ( TINT i = 0; i < m_iNumSpectators; i++ )
		{
			if ( IsModelVisible( m_apSpectators[ i ] ) )
				m_apSpectators[ i ]->PlayAnimation( TPS8( fall ) );
		}

		m_bFallStarted = TTRUE;
	}

	if ( pFallSteer->IsFinished() )
	{
		if ( IsGameOver() )
		{
			// Thrown off after the finish line, end the race sooner
			if ( m_fGameOverTime > 0.0f && m_fGameOverTime < 2.5f )
				m_fGameOverTime = 2.5f;
		}
		else if ( !m_pFade )
		{
			m_pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
		}
	}
	else if ( pFallSteer->IsInWater() )
	{
		ACameraManager::GetSingleton()->DetachCameraHelpers();
		pPlayer->SetVisible( TFALSE );

		if ( !m_pFade )
			m_pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
	}

	if ( m_pFade && !m_pFade->IsStillFading( TTRUE ) )
	{
		// The screen is black, put the player back on the bike
		pPlayer->SetVisible( TTRUE );

		TVector4 vecPosition;
		TVector4 vecDirection;
		GetRespawnPoint( pPlayer, vecPosition, vecDirection, -1 );

		AUnit* pBike     = TNULL;
		TINT   iBikeType = 0;

		ABikeManager* pBikeManager = ABikeManager::GetSingleton();
		pBikeManager->FindClosestBike( vecPosition, pBike, iBikeType );

		if ( pBike )
		{
			pBike->GetSteer()->SetPosition( vecPosition );
			pBike->GetSteer()->SetDirection( vecDirection );
		}

		pBikeManager->EndRide( TFALSE );
		pBikeManager->StartupPlayerBike();
		ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
		pBikeManager->CreatePlayerBike();

		ABikeSteer* pBikeSteer = GetBikerSteer( 0 );
		pBikeSteer->Reset( vecPosition, vecDirection );
		pBikeSteer->Start();
		pBikeSteer->GetPhysics()->SetFrozen( TTRUE );

		AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
		m_pFade = TNULL;

		ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ResetPosition( TTRUE );
		m_bFallStarted = TFALSE;
	}
}

// $Barnyard: FUNCTION 0052bcb0
void ABikeRaceMicroGame::UpdateSunkBike( AUnit* a_pUnit, TBOOL a_bPlayer )
{
	if ( !a_pUnit->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
		return;

	ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, a_pUnit->GetSteer() );

	if ( !pSteer || !pSteer->IsInWater() )
		return;

	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, a_pUnit );
	pUnit->SetVisible( TFALSE );

	if ( a_bPlayer == TTRUE )
	{
		ACameraManager::GetSingleton()->DetachCameraHelpers();

		if ( !m_pFade )
			m_pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
	}

	if ( !pSteer->IsSunk() )
		return;

	// Put the bike back on the course
	TVector4 vecPosition;
	TVector4 vecDirection;
	GetRespawnPoint( a_pUnit, vecPosition, vecDirection, -1 );
	pSteer->Reset( vecPosition, vecDirection );
	pUnit->SetVisible( TTRUE );

	if ( a_bPlayer == TTRUE )
	{
		if ( m_pFade && !m_pFade->IsStillFading( TTRUE ) )
		{
			AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
			m_pFade = TNULL;
		}

		ACameraManager* pCameraManager = ACameraManager::GetSingleton();
		pCameraManager->SetCameraHelper( ACameraManager::CAMERAHELPER_BIKE );
		pCameraManager->GetCurrentCameraHelper()->SetTarget( a_pUnit->GetSelfProxy() );
		pCameraManager->GetCurrentCameraHelper()->ResetPosition( TTRUE );
	}
}

// $Barnyard: FUNCTION 0052c8d0
void ABikeRaceMicroGame::UpdateSunkBikes()
{
	UpdateSunkBike( GetBikerUnit( 0 ), TTRUE );

#ifdef BARNYARD_COMMUNITY_PATCH
	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
#else
	// NOTE: retail skips the last rival, it stays in the water once it fell in
	for ( TINT i = 1; i < m_pTrack->iNumRivals; i++ )
#endif
		UpdateSunkBike( GetBikerUnit( i ), TFALSE );
}

// $Barnyard: FUNCTION 0052af70
void ABikeRaceMicroGame::UpdateRivals( TFLOAT a_fDeltaTime )
{
	// Seek to the next waypoint
	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
	{
		Biker&      rBiker = m_aBikers[ i ];
		Rival&      rRival = m_aRivals[ i - 1 ];
		ABikeSteer* pSteer = GetBikerSteer( i );

		if ( !pSteer->IsSeeking() && rRival.eState != RIVALSTATE_FINISHED )
		{
			// Reached the target
			if ( rRival.eState == RIVALSTATE_REVERSING || rRival.eState == RIVALSTATE_RECOVERING )
			{
				rRival.eState = RIVALSTATE_RACING;
			}
			else
			{
				rBiker.iWaypoint++;

				if ( rRival.eState == RIVALSTATE_BRAKING )
				{
					rRival.fTimer = 0.0f;
					rRival.eState = RIVALSTATE_RACING;
				}
			}

			pSteer->SetSeekTarget( GetSeekTarget( i ) );
		}

		// Past the end of the segment?
		TVector4 vecPos    = *pSteer->GetPosition();
		TINT     iPrevious = rBiker.iWaypoint - 1;

		if ( iPrevious < 0 )
			iPrevious += m_iNumWaypoints;

		TFLOAT fT;
		TMath::DistancePointToSegmentSq( vecPos, m_aWaypoints[ iPrevious ], m_aWaypoints[ rBiker.iWaypoint ], fT );

		if ( fT >= 1.0f )
		{
			rBiker.iWaypoint++;
			pSteer->SetSeekTarget( GetSeekTarget( i ) );
		}
	}

	// Rubber banding, starting from the rival closest to the player
	TINT iClosestRival    = -1;
	TINT iClosestDistance = -1;

	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
	{
		const Biker& rBiker    = m_aBikers[ i ];
		TINT         iDistance = IntAbs( m_aBikers[ 0 ].iWaypoint + m_iNumWaypoints * ( m_aBikers[ 0 ].iLap - rBiker.iLap ) - rBiker.iWaypoint );

		if ( iClosestDistance == -1 || iDistance < iClosestDistance )
		{
			iClosestRival    = i;
			iClosestDistance = iDistance;
		}
	}

	if ( iClosestRival != -1 )
	{
#ifdef BARNYARD_LATEST
		// Latest version (Wii 80280dec): on Easy the closest rival keeps level with the player instead of 1 ahead
		UpdateRubberBand( iClosestRival, 0, APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 0 : 1 );
#else
		UpdateRubberBand( iClosestRival, 0, 1 );
#endif

		for ( TINT i = iClosestRival + 1; i <= m_pTrack->iNumRivals; i++ )
			UpdateRubberBand( i, i - 1, 2 );

		for ( TINT i = iClosestRival - 1; i > 0; i-- )
			UpdateRubberBand( i, i + 1, -2 );
	}

	// Getting stuck and sharp turns
	for ( TINT i = 0; i < m_pTrack->iNumRivals; i++ )
	{
		Biker&      rBiker = m_aBikers[ i + 1 ];
		Rival&      rRival = m_aRivals[ i ];
		ABikeSteer* pSteer = GetBikerSteer( i + 1 );

		if ( ( rRival.eState == RIVALSTATE_RACING || rRival.eState == RIVALSTATE_UNK6 || rRival.eState == RIVALSTATE_SLOWED || rRival.eState == RIVALSTATE_RECOVERING ) &&
		     pSteer->GetSpeed() < 1.0f )
		{
			if ( rRival.eState == RIVALSTATE_RECOVERING )
				pSteer->SetSeekTarget( m_aWaypoints[ rBiker.iWaypoint ] );

			rRival.eState = RIVALSTATE_STUCK;
			rRival.fTimer = 2.0f;
		}

		switch ( rRival.eState )
		{
			case RIVALSTATE_RACING:
			case RIVALSTATE_SLOWED:
			case RIVALSTATE_UNK6:
			{
				TINT iWaypoint = rBiker.iWaypoint;

				if ( TVector4::Distance( m_aWaypoints[ iWaypoint ], *pSteer->GetPosition() ) < 8.0f )
				{
					// Brake if the next turn is sharp
					TVector4 vecVelocity = pSteer->GetVelocity();

					if ( vecVelocity.MagnitudeSq() < 0.01f )
						vecVelocity = *pSteer->GetDirection();

					TFLOAT                    fAngle  = vecVelocity.GetAngleBetweenVector( m_aWaypointDirs[ ( iWaypoint + 1 ) % m_iNumWaypoints ] );
					const APhysicsBikeParams& rParams = pSteer->GetPhysics()->m_oParams;

					if ( pSteer->GetSpeed() <= rParams.fMaxBoostSpeed )
					{
						if ( pSteer->GetSpeed() > rParams.fMaxSpeed && TMath::PI * 0.66f < fAngle )
						{
							rRival.eState = RIVALSTATE_BRAKING;
							rRival.fTimer = 1.0f;
						}
					}
					else if ( TMath::PI * 0.5f < fAngle )
					{
						rRival.eState = RIVALSTATE_BRAKING;
						rRival.fTimer = 1.0f;
					}
				}

				break;
			}
			case RIVALSTATE_STUCK:
			{
				if ( pSteer->GetSpeed() > 1.0f )
				{
					rRival.eState = RIVALSTATE_RACING;
					rRival.fTimer = 0.0f;
					break;
				}

				rRival.fTimer -= a_fDeltaTime;

				if ( rRival.fTimer < 0.0f )
				{
					// Reverse, then seek to a point a bit further on the course
					rRival.eState = RIVALSTATE_REVERSING;
					rRival.fTimer = 2.0f;

					TINT iWaypoint = rBiker.iWaypoint;
					TINT iPrevious = iWaypoint - 1;

					if ( iPrevious < 0 )
						iPrevious += m_iNumWaypoints;

					const TVector4& rcStart = m_aWaypoints[ iPrevious ];
					TFLOAT          fAlong  = TMath::DistanceAlongSegment( *pSteer->GetPosition(), rcStart, m_aWaypoints[ iWaypoint ] ) + 5.0f;

					TVector4 vecDir = m_aWaypointDirs[ iWaypoint ];

					if ( vecDir.NormaliseMag() > fAlong )
					{
						TVector4 vecOffset;
						vecOffset.Multiply( vecDir, fAlong );

						TVector4 vecTarget( rcStart.x + vecOffset.x, rcStart.y + vecOffset.y, rcStart.z + vecOffset.z, rcStart.w );
						pSteer->SetSeekTarget( vecTarget );
					}
				}

				break;
			}
			case RIVALSTATE_BRAKING:
			{
				rRival.fTimer -= a_fDeltaTime;

				if ( pSteer->GetSpeed() >= 2.0f && rRival.fTimer > 0.0f )
				{
					pSteer->SetSeekSpeed( THROTTLE_REVERSE );
				}
				else
				{
					rRival.eState = RIVALSTATE_RACING;
					rRival.fTimer = 0.0f;
				}

				break;
			}
			case RIVALSTATE_REVERSING:
			{
				rRival.fTimer -= a_fDeltaTime;

				if ( rRival.fTimer >= 0.0f )
				{
					pSteer->SetSeekSpeed( THROTTLE_REVERSE );
				}
				else
				{
					rRival.eState = RIVALSTATE_RECOVERING;
					rRival.fTimer = 0.0f;
				}

				break;
			}
			default:
				break;
		}
	}

	// Jumps
	ATerrainLocatorList* pLocatorList = ATerrainInterface::GetSingleton()->GetLocatorList();

	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
	{
		ABikeSteer* pSteer = GetBikerSteer( i );

		for ( TINT j = 0; j < m_iNumJumps; j++ )
		{
			TMatrix44 matJump;
			m_apJumps[ j ]->GetMatrix( matJump );

			// NOTE: retail copies the matrix of the transform object directly
			TMatrix44 matWorld = TREINTERPRETCAST( const TMatrix44&, pLocatorList->GetWorldTransform() );
			TMatrix44 matLocal = matJump;
			matJump.Multiply( matWorld, matLocal );

			const TVector4& rcJumpPos = matJump.AsBasisVector4( BASISVECTOR_TRANSLATION );

			if ( TVector4::DistanceSq( rcJumpPos, *pSteer->GetPosition() ) >= 100.0f )
				continue;

			// The ramp goes 4 back along the first axis of the locator
			const TVector4& rcForward = matJump.AsBasisVector4( BASISVECTOR_RIGHT );
			const TVector4& rcSide    = matJump.AsBasisVector4( BASISVECTOR_UP );

			TVector4 vecOffset;
			vecOffset.Multiply( rcForward, 4.0f );

			TVector4 vecRampEnd( rcJumpPos.x - vecOffset.x, rcJumpPos.y - vecOffset.y, rcJumpPos.z - vecOffset.z, rcJumpPos.w );

			TFLOAT fT;
			TFLOAT fDistance = TMath::DistancePointToSegmentSq( *pSteer->GetPosition(), rcJumpPos, vecRampEnd, fT );

			TVector4 vecClosest;
			vecClosest.Multiply( rcForward, fT );
			vecClosest.Multiply( 5.0f );

			TVector4 vecPoint( rcJumpPos.x - vecClosest.x, rcJumpPos.y - vecClosest.y, rcJumpPos.z - vecClosest.z, rcJumpPos.w );

			TVector4 vecToBike;
			vecToBike.Substract( *pSteer->GetPosition(), vecPoint );

			if ( TVector4::DotProduct3( vecToBike, rcSide ) > 0.0f && fDistance < 25.0f && fT > 0.0f && fT < 1.0f )
			{
				pSteer->Jump();
				break;
			}
		}
	}

	if ( IsGameOver() )
		return;

	// The rivals taunt the player when they overtake
	for ( TINT i = 0; i < m_pTrack->iNumRivals; i++ )
	{
		Rival& rRival = m_aRivals[ i ];

		if ( rRival.fTauntTimer > 0.0f )
			rRival.fTauntTimer = TMath::Max( rRival.fTauntTimer - a_fDeltaTime, 0.0f );

		TBOOL bWasAhead    = rRival.bAhead;
		TBOOL bPlayerAhead = IsAhead( 0, i + 1 );
		rRival.bAhead      = bPlayerAhead;

		if ( bWasAhead && !bPlayerAhead && rRival.fTauntTimer <= 0.0f )
		{
			// Just overtaken, taunt in half a second
			rRival.fTauntTimer = -0.5f;
		}
		else if ( rRival.fTauntTimer < 0.0f )
		{
			if ( bPlayerAhead )
			{
				rRival.fTauntTimer = 0.0f;
				continue;
			}
		}
		else
		{
			continue;
		}

		rRival.fTauntTimer += a_fDeltaTime;

		if ( rRival.fTauntTimer >= 0.0f && !ASoundManager::GetSingleton()->IsCueUsed( m_iTauntCue ) )
		{
			m_iTauntCue        = ASoundManager::GetSingleton()->PlayCueAtUnit( s_aTauntCues[ i ], GetBikerUnit( i + 1 ) );
			rRival.fTauntTimer = 15.0f;
		}
	}
}

// $Barnyard: FUNCTION 0052a6c0
void ABikeRaceMicroGame::UpdatePickups( TFLOAT a_fDeltaTime )
{
	m_fPickupTime += a_fDeltaTime;

	ASteer* pPlayerSteer = GetBikerUnit( 0 )->GetSteer();

	// Barnbucks
	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		Coin& rCoin = m_aCoins[ i ];

		if ( rCoin.iState == COINSTATE_COLLECTED )
			continue;

		TMatrix44 matCoin;
		rCoin.pObject->GetModelInstance()->GetLocalTransform( matCoin );
		TVector4 vecPosition = matCoin.AsBasisVector4( BASISVECTOR_TRANSLATION );

		TVector4 vecToPlayer;
		vecToPlayer.Substract( *pPlayerSteer->GetPosition(), vecPosition );

		if ( vecToPlayer.MagnitudeSqXZ() > 2500.0f )
		{
			rCoin.pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			continue;
		}

		rCoin.pObject->SetSkeletonUpdating( TTRUE, TTRUE );

		if ( rCoin.iState == COINSTATE_NOT_GROUNDED )
		{
			ATerrainInterface::GetSingleton()->SetToGroundHeightTop( vecPosition, 0 );

			if ( vecPosition.y != 0.0f )
				rCoin.iState = COINSTATE_GROUNDED;

			vecPosition.y -= 0.5f;
		}

		// Spin
		matCoin.Identity();
		matCoin.RotateY( TFLOAT( i ) * s_fPickupPhase + m_fPickupTime + m_fPickupTime );
		matCoin.RotateX( TMath::PI / 2 );
		matCoin.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecPosition;
		rCoin.pObject->GetModelInstance()->SetTransform( matCoin );

		if ( TVector4::DistanceSq( *GetBikerUnit( 0 )->GetSteer()->GetPosition(), vecPosition ) < 9.0f )
		{
			AShadowManager::GetSingleton()->SetShadowEnabled( rCoin.pObject->GetModelInstance(), TFALSE );
			StartCollectEffect( rCoin.pObject, 1, 30.0f );
			rCoin.iState = COINSTATE_COLLECTED;
		}
	}

	// Chilliberries
	for ( TINT i = 0; i < m_iNumBerries; i++ )
	{
		Berry& rBerry = m_aBerries[ i ];

		if ( rBerry.fTimer < -1.0f )
		{
			// Collected, the berry restores the stamina once its effect is over
			if ( rBerry.pObject->GetModelInstance()->GetOpacity() < 1.0f )
			{
				rBerry.fTimer = 16.0f;

				ASteer* pSteer = GetBikerUnit( 0 )->GetSteer();

				if ( pSteer->IsA( &TGetClass( ABikeSteer ) ) )
				{
					TSTATICCAST( ABikeSteer, pSteer )->SetStamina( 1.0f );

					if ( ASimPlayerMainHudElement* pMainHud = AHudManager::GetSingleton()->FindElement<ASimPlayerMainHudElement>() )
						pMainHud->StartStaminaFlash();
				}
			}

			continue;
		}

		if ( rBerry.fTimer > 0.0f )
		{
			// Waiting to respawn
			rBerry.fTimer -= a_fDeltaTime;

			if ( rBerry.fTimer <= 0.0f )
			{
				rBerry.fTimer = 0.0f;
				rBerry.pObject->SetSkeletonUpdating( TTRUE, TTRUE );
				rBerry.pObject->SetOpacity( 1.0f, TTRUE );
				rBerry.pObject->GetModelInstance()->SetTranslation( rBerry.vPosition.AsVector3() );
				AShadowManager::GetSingleton()->SetShadowEnabled( rBerry.pObject->GetModelInstance(), TTRUE );
			}

			continue;
		}

		TMatrix44 matBerry;
		rBerry.pObject->GetModelInstance()->GetLocalTransform( matBerry );
		TVector4 vecPosition = matBerry.AsBasisVector4( BASISVECTOR_TRANSLATION );

		ATerrainInterface::GetSingleton()->SetToGroundHeightTop( vecPosition, 0 );
		vecPosition.y -= 0.5f;

		matBerry.Identity();
		matBerry.RotateY( m_fPickupTime + m_fPickupTime );
		matBerry.RotateX( TMath::PI / 2 );

		if ( rBerry.fTimer > -1.0f )
		{
			// Grows back
			rBerry.fTimer -= a_fDeltaTime;

			if ( rBerry.fTimer < -1.0f )
				rBerry.fTimer = -1.0f;

			TFLOAT fScale = -rBerry.fTimer;
			matBerry.Scale( fScale, fScale, fScale );
		}

		matBerry.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecPosition;
		rBerry.pObject->GetModelInstance()->SetTransform( matBerry );

		if ( !pPlayerSteer->IsA( &TGetClass( ABikeSteer ) ) )
			continue;

		AAniEffectManager* pAniEffectManager = AAniEffectManager::GetSingleton();

		if ( TVector4::DistanceSq( *pPlayerSteer->GetPosition(), vecPosition ) < 9.0f && pAniEffectManager && pAniEffectManager->ReserveEffects( 2 ) )
		{
			// Throw the berry to the stamina bars and fade it out
			rBerry.pObject->SetOpacity( 1.0f, TTRUE );

			ASwirlTrackHudAniEffect* pSwirlEffect = pAniEffectManager->CreateEffect<ASwirlTrackHudAniEffect>();

			TQuaternion qRotation = TQuaternion::IDENTITY;
			qRotation.SetFromEulerRollPitchYaw( TMath::PI / 2, 0.0f, 0.0f );

			pSwirlEffect->Setup( rBerry.pObject, TVector2( 50.0f, 30.0f ), qRotation, 12.0f, 15.0f, 0.0f, 0.5f );
			pSwirlEffect->SetSpeed( 20.0f );
			pSwirlEffect->SetActive( TTRUE );

			AFadeAniEffect* pFadeEffect = pAniEffectManager->CreateEffect<AFadeAniEffect>();
			pFadeEffect->SetObject( rBerry.pObject );
			pFadeEffect->SetSpeed( 10.0f );
			pSwirlEffect->SetNextEffect( pFadeEffect );

			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_STAMINA_PICKUP );
			AShadowManager::GetSingleton()->SetShadowEnabled( rBerry.pObject->GetModelInstance(), TFALSE );
			rBerry.fTimer = -2.0f;
		}
	}
}

// $Barnyard: FUNCTION 0052a1f0
void ABikeRaceMicroGame::UpdateSpectators()
{
	for ( TINT i = 0; i < m_iNumSpectators; i++ )
	{
		AAnimatableObject* pSpectator = m_apSpectators[ i ];

		TVector4 vecPosition;
		pSpectator->GetModelInstance()->GetPosition( vecPosition );

		TVector4 vecToPlayer;
		vecToPlayer.Substract( vecPosition, *GetBikerUnit( 0 )->GetSteer()->GetPosition() );

		if ( vecToPlayer.MagnitudeSq() <= 1225.0f )
		{
			pSpectator->SetSkeletonUpdating( TTRUE, TTRUE );

			if ( !pSpectator->GetToshiAnimInterface().GetAnim() )
			{
				if ( ARandom::GetSingleton()->m_oRandom.GetFloat() >= 0.05f )
					pSpectator->PlayAnimation( TPS8( idle ) );
				else
					pSpectator->PlayAnimation( TPS8( win ) );
			}
		}
		else
		{
			pSpectator->KillAllAnimations();
			pSpectator->SetSkeletonUpdating( TFALSE, TTRUE );
		}
	}
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" lets the bike AI of the rivals ride the bike of the player along the waypoints of the course
void ABikeRaceMicroGame::Dev_UpdateAutoPlay()
{
	static TINT s_iEnabled = -1;

	if ( s_iEnabled == -1 )
		s_iEnabled = T2String8::FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;

	if ( !s_iEnabled || m_bGameOver || m_iNumWaypoints <= 0 )
		return;

	ASteer* pPlayerSteer = GetBikerUnit( 0 )->GetSteer();

	if ( !pPlayerSteer->IsA( &TGetClass( ABikeSteer ) ) )
		return;

	static TINT   s_iWaypoint    = -1;
	static TFLOAT s_fStuckTime   = 0.0f;
	static TFLOAT s_fReverseTime = 0.0f;
	static TINT   s_iLastLap     = -1;
	static TINT   s_iLastSegment = -1;

	ABikeSteer* pSteer     = TSTATICCAST( ABikeSteer, pPlayerSteer );
	TFLOAT      fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	// The bike is recreated after a fall
	if ( !pSteer->IsAIControlled() )
	{
		pSteer->SetAIControlled( TTRUE );
		s_iWaypoint = -1;
	}

	if ( s_iWaypoint < 0 )
	{
		TINT iClosest = 0;

		for ( TINT i = 0; i < m_iNumWaypoints; i++ )
		{
			if ( TVector4::DistanceSqXZ( *pSteer->GetPosition(), m_aWaypoints[ i ] ) < TVector4::DistanceSqXZ( *pSteer->GetPosition(), m_aWaypoints[ iClosest ] ) )
				iClosest = i;
		}

		s_iWaypoint = ( iClosest + 1 ) % m_iNumWaypoints;
		pSteer->SetSeekTarget( m_aWaypoints[ s_iWaypoint ] );
	}

	// Same as the rivals: next waypoint once the target is reached or the segment is passed
	TFLOAT fT;
	TMath::DistancePointToSegmentSq( *pSteer->GetPosition(), m_aWaypoints[ ( s_iWaypoint + m_iNumWaypoints - 1 ) % m_iNumWaypoints ], m_aWaypoints[ s_iWaypoint ], fT );

	if ( !pSteer->IsSeeking() || fT >= 1.0f )
	{
		s_iWaypoint = ( s_iWaypoint + 1 ) % m_iNumWaypoints;
		pSteer->SetSeekTarget( m_aWaypoints[ s_iWaypoint ] );
	}

	// Back off when stuck against something
	if ( s_fReverseTime > 0.0f )
	{
		s_fReverseTime -= fDeltaTime;
		pSteer->SetSeekSpeed( -1.0f );
	}
	else
	{
		s_fStuckTime = ( TMath::Abs( pSteer->GetSpeed() ) < 2.0f ) ? s_fStuckTime + fDeltaTime : 0.0f;

		if ( s_fStuckTime > 2.0f )
		{
			s_fStuckTime   = 0.0f;
			s_fReverseTime = 1.5f;
		}
	}

	if ( m_aBikers[ 0 ].iLap != s_iLastLap || m_aBikers[ 0 ].iSegment != s_iLastSegment )
	{
		s_iLastLap     = m_aBikers[ 0 ].iLap;
		s_iLastSegment = m_aBikers[ 0 ].iSegment;
		TINFO( "[devbikerace] lap %d segment %d/%d waypoint %d/%d placing %d\n", s_iLastLap, s_iLastSegment, m_iNumSegments, m_aBikers[ 0 ].iWaypoint, m_iNumWaypoints, m_oPlacingHud.GetPlacing() );
		fflush( stdout );
	}
}

#endif // BARNYARD_COMMUNITY_PATCH
