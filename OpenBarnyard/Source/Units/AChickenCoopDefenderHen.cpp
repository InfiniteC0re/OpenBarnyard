#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChickenCoopDefenderHen.h"
#include "AChickenCoopDefenderThief.h"
#include "AUnitManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FX/AParticleEffectManager.h"
#include "GUI/AGUI2MiniMap.h"
#include "MiniGame/AChickenCoopDefender.h"
#include "Physics/ACollisionManager.h"
#include "Sound/ASoundManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005139e0, CreateObject 00513e90, CreateObjectInPlace 00513ef0 (the constructor is inlined in both)
TDEFINE_CLASS( AChickenCoopDefenderHen );

TPSTRING8_DECLARE( held_lp );
TPSTRING8_DECLARE( fly_lp );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( distress_pleased );

static constexpr ASoundId SOUND_HEN_THROWN   = 532;
static constexpr ASoundId SOUND_HEN_POCKETED = 533;
static constexpr ASoundId SOUND_HEN_RUN      = 534;

// Number of AChickenCoopDefenderGame::ms_aGoals a thrown hen goes for (0x00722860)
static TINT s_iNumGoals = AChickenCoopDefenderGame::NUM_GOALS;

// $Barnyard: FUNCTION 005139f0
AChickenCoopDefenderHen::AChickenCoopDefenderHen()
{
	m_pNetwork      = TNULL;
	m_pGame         = TNULL;
	m_fTimer        = 0.0f;
	m_pThreat       = TNULL;
	m_iValue        = 0;
	m_eState        = STATE_HOME;
	m_iMapIcon      = -1;
	m_iPocketEffect = -1;
}

// $Barnyard: FUNCTION 00513f30
// $Barnyard: FUNCTION 00513a50
AChickenCoopDefenderHen::~AChickenCoopDefenderHen()
{
	m_pGame = TNULL;
}

// $Barnyard: FUNCTION 00513a70
TBOOL AChickenCoopDefenderHen::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_iPocketEffect = AParticleEffectManager::FindEffect( "chickpocket" );
	return TTRUE;
}

TBOOL AChickenCoopDefenderHen::IsFollowedByCamera()
{
	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
	AUnitProxy*            pTarget = pHelper->GetTarget();

	return ( pTarget ? pTarget->GetUnit() : TNULL ) == this;
}

// $Barnyard: FUNCTION 00513aa0
void AChickenCoopDefenderHen::UpdateAnimation( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pObject = GetAnimatableObject();

	switch ( m_eState )
	{
		case STATE_HELD:
			if ( !pObject->IsAnimPlaying( TPS8( held_lp ), 0.0f ) )
				pObject->PlayAnimation( TPS8( held_lp ) );

			return;
		case STATE_HOME:
			return;
		case STATE_THROWN:
			if ( !pObject->IsAnimPlaying( TPS8( fly_lp ), 0.0f ) )
				pObject->PlayAnimation( TPS8( fly_lp ) );

			return;
		default:
			break;
	}

	AToshiAnimationInterface& rAnimInterface = pObject->GetToshiAnimInterface();

	switch ( m_pSteer->GetMovementType() )
	{
		case ASteer::MOVEMENT_STOPPED:
			if ( !pObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
				pObject->PlayAnimation( TPS8( idle_lp ) );

			break;
		case ASteer::MOVEMENT_WALK:
		{
			TFLOAT              fSpeed   = m_pSteer->GetMeasuredSpeed();
			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( walk_lp ) );

			if ( !pAnimRef )
			{
				if ( rAnimInterface.IsAnimPlaying( TPS8( walk_lp ), TFALSE ) )
					break;

				pAnimRef = rAnimInterface.PlayAnim( TPS8( walk_lp ), -1.0f, TFALSE );

				if ( !pAnimRef )
					break;
			}

			rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed / m_pSteer->GetTraits()->m_fAnimatedWalkSpeed );
			break;
		}
		case ASteer::MOVEMENT_RUN:
		case ASteer::MOVEMENT_SPRINT:
		{
			TFLOAT              fSpeed   = m_pSteer->GetMeasuredSpeed();
			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( run_lp ) );

			if ( !pAnimRef )
			{
				if ( rAnimInterface.IsAnimPlaying( TPS8( run_lp ), TFALSE ) )
					break;

				pAnimRef = rAnimInterface.PlayAnim( TPS8( run_lp ), -1.0f, TFALSE );

				if ( !pAnimRef )
					break;
			}

			rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed / m_pSteer->GetTraits()->m_fAnimatedRunSpeed );
			break;
		}
	}
}

// $Barnyard: FUNCTION 00513c50
void AChickenCoopDefenderHen::SetMapIconVisible( TBOOL a_bVisible )
{
	AGUI2MiniMap* pMap = m_pGame->pMap;

	if ( m_iMapIcon < 0 )
	{
		m_iMapIcon = pMap->AddIcon( *m_pSteer->GetPosition(), *m_pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_DOT_WHITE, 0.1f );
		m_pGame->pMap->SetIconColour( m_iMapIcon, 0xFFFFC800 );
	}

	m_pGame->pMap->SetIconVisible( m_iMapIcon, a_bVisible );
}

// $Barnyard: FUNCTION 00514100
void AChickenCoopDefenderHen::FindThreat( TFLOAT a_fRadius )
{
	TFLOAT fClosestSq = a_fRadius * a_fRadius;
	m_pThreat         = TNULL;

	for ( auto it = m_pGame->oThieves.Begin(); it != m_pGame->oThieves.End(); it++ )
	{
		AUnit* pThief = it.Value() ? it.Value()->GetUnit() : TNULL;

		TFLOAT fDistanceSq = TVector4::DistanceSq( *pThief->GetSteer()->GetPosition(), *m_pSteer->GetPosition() );

		if ( fDistanceSq < fClosestSq )
		{
			fClosestSq = fDistanceSq;
			m_pThreat  = it.Value() ? it.Value()->GetUnit() : TNULL;
		}
	}
}

void AChickenCoopDefenderHen::ReturnToPool( AChickenCoopDefenderGame::UnitList15& a_rFrom )
{
	AChickenCoopDefenderGame::MoveUnit( a_rFrom, m_pGame->oHenPool, GetSelfProxy() );
}

// $Barnyard: FUNCTION 005142b0
TBOOL AChickenCoopDefenderHen::SetState( STATE_ a_eState )
{
	STATE_ eNewState = a_eState;

	switch ( a_eState )
	{
		case STATE_HELD:
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
				ChangeSteer( &TGetClass( AStationarySteer ) );

			SetVisible( TTRUE );
			SetMapIconVisible( TFALSE );
			break;
		case STATE_WANDER:
		{
			ASteer* pSteer = m_pSteer;
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_WanderPathfindingSpecific();

			GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_HEN_RUN, *m_pSteer->GetPosition() );
			break;
		}
		case STATE_FLEE:
		{
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
				ChangeSteer( &TGetClass( AInteriorSteer ) );

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );

			if ( !pSteer->GetCurrentNetwork() )
				pSteer->ConnectToNetwork( m_pNetwork );

			SetVisible( TTRUE );
			SetMovement( TTRUE );

			GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_HEN_RUN, *m_pSteer->GetPosition() );
			SetMapIconVisible( TTRUE );
			break;
		}
		case STATE_HOME:
			break;
		case STATE_SCATTER:
		{
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
				ChangeSteer( &TGetClass( AInteriorSteer ) );

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			pSteer->ConnectToNetwork( m_pNetwork );
			pSteer->Command_WanderPathfindingSpecific();

			SetVisible( TTRUE );
			SetMovement( TTRUE );

			GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			m_fTimer = 10.0f;
			SetMapIconVisible( TTRUE );
			break;
		}
		case STATE_THROWN:
			SetMapIconVisible( TTRUE );
			ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_HEN_THROWN, *m_pSteer->GetPosition() );
			break;
		case STATE_LANDED:
		{
			if ( IsFollowedByCamera() )
				m_pGame->ResetCamera();

			// Back on the network
			TVector4 vPosition;
			m_pSteer->CopyPosition( vPosition );

			TINT iArea = m_pNetwork->FindContainingArea( vPosition );

			if ( iArea == -1 )
			{
				iArea = m_pNetwork->FindClosestArea( vPosition );
				m_pNetwork->m_pINetwork->m_pAreas[ iArea ].GetPosition( vPosition );
			}

			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
				ChangeSteer( &TGetClass( AInteriorSteer ) );

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			pSteer->SetNetwork( m_pNetwork, 0 );
			pSteer->SnapToArea( iArea );
			pSteer->SetPosition( vPosition );
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_WanderPathfindingSpecific();

			// A thrown hen is loose again
			if ( m_eState == STATE_THROWN )
				AChickenCoopDefenderGame::MoveUnit( m_pGame->oCarriedHens, m_pGame->oLooseHens, GetSelfProxy() );

			eNewState = STATE_WANDER;
			break;
		}
		case STATE_POCKETED:
		{
			ChangeSteer( &TGetClass( AStationarySteer ) );

			m_pGame->iNumHensCarried--;
			m_pGame->iBonus += m_iValue;

			AParticleEffectManager::GetSingleton()->PlayEffect( m_iPocketEffect, *m_pSteer->GetPosition(), TVector4::VEC_NEGY );
			ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_HEN_POCKETED, *m_pSteer->GetPosition() );

			if ( IsFollowedByCamera() )
			{
				// The camera keeps looking at the coop for a second
				SetVisible( TFALSE );
				m_pSteer->SetPosition( AChickenCoopDefenderGame::ms_aGoals[ 1 ] );
				m_pSteer->SetDirection( AChickenCoopDefenderGame::ms_aGoals[ 1 ] );
				m_fTimer = 1.0f;
				break;
			}

			eNewState = STATE_HOME;
		}
		default:
			break;
	}

	if ( eNewState == STATE_HOME )
	{
		ChangeSteer( &TGetClass( AStationarySteer ) );
		SetVisible( TFALSE );

		switch ( m_eState )
		{
			case STATE_HELD:
				m_pGame->iNumHensLost++;
				ReturnToPool( m_pGame->oCarriedHens );
				break;
			case STATE_FLEE:
				m_pGame->iNumHensCarried--;
				ReturnToPool( m_pGame->oLooseHens );
				break;
			case STATE_THROWN:
			case STATE_POCKETED:
				ReturnToPool( m_pGame->oCarriedHens );
				break;
			default:
				break;
		}

		if ( IsFollowedByCamera() )
			m_pGame->ResetCamera();

		// Far away
		m_pSteer->SetPosition3( TVector3( 0.0f, 0.0f, 1000000.0f ) );
		SetMovement( TFALSE );
		SetMapIconVisible( TFALSE );
	}

	m_eState = eNewState;
	return TTRUE;
}

// $Barnyard: FUNCTION 00514ae0
void AChickenCoopDefenderHen::UpdateFlee( TFLOAT a_fDeltaTime )
{
	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pSteer );

	if ( m_pThreat )
		m_fTimer = 5.0f;

	FindThreat( 20.0f );

	TVector4 vVelocity;

	if ( m_pThreat )
	{
		// Away from the thief
		const TVector4* pThreatPosition = m_pThreat->GetSteer()->GetPosition();
		vVelocity.Substract( *pSteer->GetPosition(), *pThreatPosition );
		AChickenCoopDefenderNormaliseXZ( vVelocity );
		vVelocity.Multiply( 3.0f );
	}
	else
	{
		m_fTimer -= a_fDeltaTime;

		if ( m_fTimer < 0.0f )
		{
			SetState( STATE_WANDER );
			return;
		}

		vVelocity.Multiply( *pSteer->GetDirection(), 3.0f );
	}

	pSteer->SetDesiredVelocity( vVelocity );
}

// $Barnyard: FUNCTION 00514bd0
void AChickenCoopDefenderHen::UpdateWander( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;
	FindThreat( 20.0f );

	if ( m_pThreat )
		SetState( STATE_FLEE );
	else if ( pSteer->HasArrived() )
		SetState( STATE_WANDER );
}

// $Barnyard: FUNCTION 00514c20
void AChickenCoopDefenderHen::UpdateScatter( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer < 0.0f )
		SetState( STATE_FLEE );
}

// $Barnyard: FUNCTION 00514c50
void AChickenCoopDefenderHen::UpdateThrown( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < s_iNumGoals; i++ )
	{
		const TVector4& rcGoal    = AChickenCoopDefenderGame::ms_aGoals[ i ];
		const TVector4* pPosition = m_pSteer->GetPosition();

		TVector4 vToGoal;
		vToGoal.Substract( rcGoal, *pPosition );

		TFLOAT fDistanceSq = vToGoal.MagnitudeSq();

		if ( fDistanceSq < 1.0f )
		{
			// In the coop
			SetState( STATE_POCKETED );
			TSTATICCAST( AAnimatedUnit, m_pGame->pHanna )->GetAnimatableObject()->PlayAnimation( TPS8( distress_pleased ) );
		}
		else
		{
			TVector4 vVelocity;
			m_pSteer->CopyVelocity( vVelocity );

			ARayData oRay( rcGoal, *m_pSteer->GetPosition() );
			TFLOAT   fDistance;

			// Pulled into the goal once close and flying towards it
			// NOTE: fDistanceSq * 4.0f < 4.0f can't be true here (fDistanceSq >= 1.0f)
			if ( fDistanceSq * 4.0f < 4.0f ||
			     ( fDistanceSq < 4.0f && TVector4::DotProduct3( vToGoal, vVelocity ) > 0.0f &&
			       !ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) ) )
			{
				TVector4 vPull;
				vPull.Multiply( vToGoal, 100.0f );

				TVector4 vPullDelta;
				vPullDelta.Multiply( vPull, a_fDeltaTime );

				TVector4 vAcceleration;
				vAcceleration.Divide( vPullDelta, TMath::Sqrt( fDistanceSq ) );

				vVelocity.x += vAcceleration.x;
				vVelocity.y += vAcceleration.y;
				vVelocity.z += vAcceleration.z;

				m_pSteer->SetVelocity( vVelocity );
			}
		}
	}

	// The camera following the hen looks down at it
	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );

	if ( IsFollowedByCamera() )
	{
		TVector4 vVelocity = m_pSteer->GetVelocity();

		if ( IsFollowedByCamera() && vVelocity.MagnitudeSqXZ() > 0.0f )
		{
			TFLOAT fPitch = TMath::ATan2( ( vVelocity.y + vVelocity.y ) / vVelocity.MagnitudeXZ(), 1.0f ) * ( 180.0f / TMath::PI );

			if ( fPitch > 0.0f )
			{
				pHelper->SetTargetPitch( fPitch + 11.0f );
				pHelper->SetDefaultPitch( fPitch + 11.0f );
			}

			pHelper->SetDefaultDistance( ( TMath::Abs( vVelocity.y ) * 0.1f + 1.0f ) * 5.0f );
		}
	}
}

// $Barnyard: FUNCTION 005156c0
TBOOL AChickenCoopDefenderHen::Think( TFLOAT a_fDeltaTime )
{
	TINT ePhase = m_pGame->ePhase;

	// The game is paused while the camera follows a throw and after the end
	if ( ePhase > AChickenCoopDefender::PHASE_PLAYING && ePhase <= AChickenCoopDefender::PHASE_ENDED &&
	     m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
	{
		m_pSteer->Command_CancelCurrentCommand();
		return AVisibleUnit::Think( a_fDeltaTime );
	}

	m_fTimer -= a_fDeltaTime;

	// Nothing moves while the player picks up or throws a raccoon
	TINT ePlayerState = m_pGame->ePlayerState;

	if ( ePlayerState > AChickenCoopDefender::PLAYERSTATE_FREE && ePlayerState <= AChickenCoopDefender::PLAYERSTATE_THROW_THIEF )
		return TFALSE;

	switch ( m_eState )
	{
		case STATE_WANDER:
			UpdateWander( a_fDeltaTime );
			break;
		case STATE_FLEE:
			UpdateFlee( a_fDeltaTime );
			break;
		case STATE_SCATTER:
			UpdateScatter( a_fDeltaTime );
			break;
		case STATE_THROWN:
			UpdateThrown( a_fDeltaTime );
			break;
		case STATE_POCKETED:
			if ( m_fTimer < 0.0f )
				SetState( STATE_HOME );

			break;
		default:
			break;
	}

	UpdateAnimation( a_fDeltaTime );

	if ( m_iMapIcon > -1 )
	{
		ASteer* pSteer = m_pSteer;
		m_pGame->pMap->SetIconTransform( m_iMapIcon, *pSteer->GetPosition(), *pSteer->GetDirection() );
	}

	return AVisibleUnit::Think( a_fDeltaTime );
}
