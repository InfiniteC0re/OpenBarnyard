#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChasingChicksChick.h"
#include "AChasingChicksHanna.h"
#include "ARandom.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "GUI/AGUI2MiniMap.h"
#include "MiniGame/AChasingChicksMiniGame.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/chicks.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Physics/ACollisionManager.h"

#include <Math/TMath.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0050b130, CreateObject 0050b410, CreateObjectInPlace 0050b470 (the constructor is inlined in both)
TDEFINE_CLASS( AChasingChicksChick );

TPSTRING8_DECLARE( distress_pleased );
TPSTRING8_DECLARE( fly_lp );
TPSTRING8_DECLARE( held_lp );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( jump_start );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( walk_lp );

// Tuning globals (.data, never written)
static TFLOAT s_fFleeSpeed       = 3.0f;  // 00773d5c
static TFLOAT s_fPlayerNearRange = 64.0f; // 00773d60, see AChasingChicksGame::FindPlayerNear
static TFLOAT s_fMaxFleeTime     = 5.0f;  // 00773d64

// NOTE: inlined twice in retail (SetState and Think). Makes the camera follow
// the player again after it followed a chick thrown by the player.
static void ResetCameraToPlayer( AGTAStyleCameraHelper* a_pHelper, AChasingChicksGame* a_pGame )
{
	a_pHelper->SetTarget( a_pGame->aPlayers[ 0 ].oUnit );
	a_pHelper->SetHeightOffset( 1.7f );
	a_pHelper->SetDefaultPitch( 11.0f );
	a_pHelper->SetTargetPitch( 11.0f );
	a_pHelper->SetDefaultDistance( 5.0f );
	a_pHelper->SetMaxPitch( 22.0f, 22.0f );
	a_pHelper->SetCollisionRadius( 0.43f, 0.2f );
	a_pHelper->ResetPosition( TTRUE );
}

// Returns the camera helper if the camera follows a_pUnit
static AGTAStyleCameraHelper* GetCameraFollowing( AUnit* a_pUnit )
{
	ACameraHelper* pHelper = ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 );
	AUnitProxy*    pTarget = pHelper->GetTarget();

	return ( ( pTarget ? pTarget->GetUnit() : TNULL ) == a_pUnit ) ? TSTATICCAST( AGTAStyleCameraHelper, pHelper ) : TNULL;
}

// $Barnyard: FUNCTION 0050b140
AChasingChicksChick::AChasingChicksChick()
    : m_fTimer( 0.0f )
    , m_fFleeTime( 0.0f )
    , m_pNetwork( TNULL )
    , m_eState( STATE_FREE )
    , m_fCheckTimer( 0.0f )
    , m_iPlayer( -1 )
    , m_pGame( TNULL )
    , m_pTrail( TNULL )
    , m_iValue( 0 )
    , m_iMapIcon( -1 )
    , m_bShowOnMap( TFALSE )
    , m_iChuckleCue( -1 )
    , m_iTailBone( -1 )
    , m_iThrowEffect( -1 )
    , m_iPickupEffect( -1 )
    , m_iPocketEffect( -1 )
{
}

// $Barnyard: FUNCTION 0050b1c0
// $Barnyard: FUNCTION 0050b4b0
AChasingChicksChick::~AChasingChicksChick()
{
}

// $Barnyard: FUNCTION 0050b1e0
void AChasingChicksChick::UpdateAnimation( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pAnimObject = GetAnimatableObject();

	switch ( m_eState )
	{
		case STATE_HELD:
			if ( !pAnimObject->IsAnimPlaying( TPS8( held_lp ), 0.0f ) )
				pAnimObject->PlayAnimation( TPS8( held_lp ) );
			break;
		case STATE_THROWN:
		case STATE_JUMPING:
			if ( !pAnimObject->IsAnimPlaying( TPS8( fly_lp ), 0.0f ) )
				pAnimObject->PlayAnimation( TPS8( fly_lp ) );
			break;
		case STATE_RESPAWN:
			break;
		default:
			switch ( m_pSteer->GetMovementType() )
			{
				case ASteer::MOVEMENT_STOPPED:
					if ( !pAnimObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
						pAnimObject->PlayAnimation( TPS8( idle_lp ) );
					break;
				case ASteer::MOVEMENT_WALK:
				{
					AToshiAnimationInterface& rAnimInterface = pAnimObject->GetToshiAnimInterface();
					TFLOAT                    fSpeed         = m_pSteer->GetMeasuredSpeed();
					AToshiAnimationRef*       pAnimRef       = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( walk_lp ) );

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
					AToshiAnimationInterface& rAnimInterface = pAnimObject->GetToshiAnimInterface();
					TFLOAT                    fSpeed         = m_pSteer->GetMeasuredSpeed();
					AToshiAnimationRef*       pAnimRef       = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( run_lp ) );

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
			break;
	}
}

// $Barnyard: FUNCTION 0050b3a0
void AChasingChicksChick::StopTrail()
{
	if ( m_pTrail )
	{
		m_pTrail->GetInstance()->Stop();
		m_pTrail = TNULL;
	}
}

// $Barnyard: FUNCTION 0050b3d0
void AChasingChicksChick::KillTrail()
{
	if ( m_pTrail )
	{
		m_pTrail->GetInstance()->Kill();
		m_pTrail = TNULL;
	}
}

// $Barnyard: FUNCTION 0050b4d0
void AChasingChicksChick::OnDestroy()
{
	m_pGame = TNULL;
	KillTrail();

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 0050b510
TBOOL AChasingChicksChick::TryJump( TINT a_iArea )
{
	AInteriorGoal* pGoal = m_pNetwork->FindGoalInArea( TPS8( jump_start ), a_iArea );

	if ( !pGoal )
		return TFALSE;

	TVector4 vDirection = pGoal->m_vForward;
	vDirection.w        = 1.0f;

	TVector4 vVelocity = vDirection;
	vVelocity.Multiply( 4.0f );
	vVelocity.y = -5.0f;

	if ( !m_pSteer->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) )
		ChangeSteer( &TGetClass( AThrowItemSteer ) );

	TVector4 vPosition;
	m_pSteer->CopyPosition( vPosition );
	vPosition.y -= 0.2f;
	m_pSteer->SetPosition( vPosition );

	m_pSteer->SetVelocity( vVelocity );
	m_pSteer->SetDirection( vDirection );

	SetState( STATE_JUMPING );
	return TTRUE;
}

// $Barnyard: FUNCTION 0050b610
TBOOL AChasingChicksChick::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	AShadowManager::GetSingleton()->AddUnitShadow( this, 0.5f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );

	// NOTE: AModelInstance::GetBoneID( const TCHAR* ) (00610e40) inlined
	AModelInstanceRef pModelInstance = GetAnimatableObject()->GetModelInstance();
	m_iTailBone                      = pModelInstance->GetSkeleton()->GetBoneID( "CB_Chick_Tail", 0 );

	m_iThrowEffect  = AParticleEffectManager::FindEffect( "chickthrow" );
	m_iPickupEffect = AParticleEffectManager::FindEffect( "chickpickup" );
	m_iPocketEffect = AParticleEffectManager::FindEffect( "chickpocket" );

	return TTRUE;
}

// $Barnyard: FUNCTION 0050b6d0
void AChasingChicksChick::StartTrail()
{
	if ( m_pTrail )
		return;

	m_pTrail = AParticleEffectManager::GetSingleton()->PlayEffect( m_iThrowEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

	if ( m_pTrail )
		m_pTrail->AttachToBone( GetAnimatableObject()->GetModelInstance().Get(), m_iTailBone, TTRUE, TFALSE );
}

// $Barnyard: FUNCTION 0050b780
TBOOL AChasingChicksChick::SetState( STATE a_eState )
{
	if ( m_eState == STATE_RESPAWN )
		return TFALSE;

	STATE eNewState = a_eState;

	switch ( a_eState )
	{
		case STATE_FREE:
			m_pSteer->Command_CancelCurrentCommand();
			m_pSteer->Command_WanderPathfindingSpecific();
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_CHICKS, *m_pSteer->GetPosition() );
			break;
		case STATE_FLEEING:
			m_pSteer->Command_CancelCurrentCommand();
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_CHICKS, *m_pSteer->GetPosition() );
			break;
		case STATE_HELD:
			ChangeSteer( &TGetClass( AStationarySteer ) );
			GetAnimatableObject()->PlayAnimation( TPS8( held_lp ) );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iPickupEffect, *m_pSteer->GetPosition(), TVector4::VEC_NEGY );
			m_bShowOnMap = TFALSE;
			m_eState     = a_eState;
			return TTRUE;
		case STATE_THROWN:
			GetAnimatableObject()->PlayAnimation( TPS8( fly_lp ) );
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_THROW, *m_pSteer->GetPosition() );
			m_iChuckleCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_TWINK_CHUCKLE, *m_pSteer->GetPosition() );
			StartTrail();
			break;
		case STATE_JUMPING:
			m_iPlayer = -1;
			GetAnimatableObject()->PlayAnimation( TPS8( fly_lp ) );
			break;
		case STATE_LANDED:
		{
			if ( m_iPlayer == 0 )
			{
				// The camera followed the chick thrown by the player
				AGTAStyleCameraHelper* pHelper = GetCameraFollowing( this );

				if ( pHelper )
				{
					ResetCameraToPlayer( pHelper, m_pGame );
					m_pGame->ePhase = AChasingChicksMiniGame::PHASE_PLAYING;
				}
			}

			TVector4 vPosition;
			m_pSteer->CopyPosition( vPosition );
			GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );

			TINT iArea = m_pNetwork->FindContainingArea( vPosition );

			if ( iArea == -1 )
			{
				iArea = m_pNetwork->FindClosestArea( vPosition );
				m_pNetwork->m_pINetwork->m_pAreas[ iArea ].GetPosition( vPosition );
			}

			if ( TryJump( iArea ) )
			{
				m_eState = STATE_JUMPING;
				return TTRUE;
			}

			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
				ChangeSteer( &TGetClass( AInteriorSteer ) );

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			pSteer->SetNetwork( m_pNetwork, 0 );
			pSteer->SnapToArea( iArea );
			pSteer->SetPosition( vPosition );
			pSteer->Command_WanderPathfindingSpecific();
			StopTrail();

			eNewState = STATE_FREE;
			break;
		}
		case STATE_POCKETED:
		{
			ChangeSteer( &TGetClass( AStationarySteer ) );

			// Score the chick for the player that threw it
			AChasingChicksPlayer* pPlayer = &m_pGame->aPlayers[ m_iPlayer ];
			pPlayer->iScore += m_iValue;

			for ( TINT i = 0; i < TARRAYSIZE( pPlayer->aNumChicksOfKind ); i++ )
				pPlayer->aNumChicksOfKind[ i ] += ( m_pGame->pGameData->aChickKinds[ i ].iValue == m_iValue );

			m_iChuckleCue = -1;
			StopTrail();
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iPocketEffect, *m_pSteer->GetPosition(), TVector4::VEC_NEGY );
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_CHICKEN_IMPACT, *m_pSteer->GetPosition() );

			if ( m_iPlayer == 0 && GetCameraFollowing( this ) )
			{
				// The camera follows the chick: keep it hidden in the basket for a second (see Think)
				SetVisible( TFALSE );
				m_pSteer->SetPosition( m_pGame->vCentre );
				m_pSteer->SetDirection( m_pGame->vCentreDir );
				m_fTimer = 1.0f;
				m_eState = a_eState;
				return TTRUE;
			}

			// Respawn at once
			[[fallthrough]];
		}
		case STATE_RESPAWN:
		{
			StopTrail();
			SetVisible( TTRUE );

			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
				ChangeSteer( &TGetClass( AInteriorSteer ) );

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );

			// Picks a random spawn, preferably far from or not beside the player
			// NOTE: retail checks the dot product with the player's direction for
			// inequality with zero, so in practice the first spawn tried is used.
			auto IsSpawnUsable = [ this ]( TINT a_iSpawn ) {
				ASteer*         pPlayerSteer = m_pGame->aPlayers[ 0 ].oUnit->GetUnit()->GetSteer();
				const TVector4& rcSpawn      = m_pGame->pSpawns[ a_iSpawn ];
				const TVector4* pPlayerPos   = pPlayerSteer->GetPosition();

				TVector4 vToPlayer( pPlayerPos->x - rcSpawn.x, pPlayerPos->y - rcSpawn.y, pPlayerPos->z - rcSpawn.z, pPlayerPos->w );

				if ( vToPlayer.NormaliseMag() > 30.0f )
					return TTRUE;

				return TVector4::DotProduct3( vToPlayer, *m_pGame->aPlayers[ 0 ].oUnit->GetUnit()->GetSteer()->GetDirection() ) != 0.0f;
			};

			TINT iStart = ARandom::GetSingleton()->m_oRandom.GetInt( m_pGame->iNumSpawns );
			TINT iSpawn = -1;

			for ( TINT i = iStart; i < m_pGame->iNumSpawns; i++ )
			{
				if ( IsSpawnUsable( i ) )
				{
					iSpawn = i;
					break;
				}
			}

			if ( iSpawn < 0 )
			{
				for ( TINT i = 0; i < iStart; i++ )
				{
					if ( IsSpawnUsable( i ) )
					{
						iSpawn = i;
						break;
					}
				}

				if ( iSpawn < 0 )
					iSpawn = 0;
			}

			pSteer->SetPosition( m_pGame->pSpawns[ iSpawn ] );
			pSteer->ConnectToNetwork( m_pNetwork );
			pSteer->Command_WanderPathfindingSpecific();

			eNewState = STATE_FREE;
			break;
		}
		default:
			m_eState = a_eState;
			return TTRUE;
	}

	m_bShowOnMap = TTRUE;
	m_eState     = eNewState;
	return TTRUE;
}

// $Barnyard: FUNCTION 0050be90
void AChasingChicksChick::UpdateFree( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;

	if ( m_fCheckTimer < 0.0f )
	{
		m_fCheckTimer = 0.1f;

		if ( TryJump( TSTATICCAST( AInteriorSteer, pSteer )->GetPath().m_iStartArea ) )
			return;

		TFLOAT fRange = s_fPlayerNearRange;
		TINT   iPlayer;

		if ( m_pGame->FindPlayerNear( *pSteer->GetPosition(), fRange, iPlayer ) )
		{
			m_iPlayer   = iPlayer;
			m_fFleeTime = 0.0f;
			SetState( STATE_FLEEING );
		}
	}

	if ( pSteer->IsCommandFinished() )
		pSteer->Command_WanderPathfindingSpecific();
}

// $Barnyard: FUNCTION 0050bf30
void AChasingChicksChick::UpdateFleeing( TFLOAT a_fDeltaTime )
{
	if ( m_fCheckTimer >= 0.0f )
		return;

	ASteer* pSteer = m_pSteer;
	m_fCheckTimer  = 0.1f;

	if ( TryJump( TSTATICCAST( AInteriorSteer, pSteer )->GetPath().m_iStartArea ) )
		return;

	if ( m_iPlayer == -1 )
	{
		SetState( STATE_FREE );
		return;
	}

	TVector4 vPlayerPos;
	m_pGame->aPlayers[ m_iPlayer ].oUnit->GetUnit()->GetSteer()->CopyPosition( vPlayerPos );

	TVector4 vAway;
	pSteer->CopyPosition( vAway );
	vAway.x -= vPlayerPos.x;
	vAway.y = 0.0f;
	vAway.z -= vPlayerPos.z;
	vAway.w = 1.0f;

	TFLOAT fDistance = vAway.NormaliseMag();

	if ( fDistance < 1.5f )
		vAway.Multiply( -1.0f );
	else if ( fDistance < 3.0f )
		vAway.Multiply( 0.5f );

	vAway.Multiply( s_fFleeSpeed );
	TSTATICCAST( AInteriorSteer, pSteer )->SetDesiredVelocity( vAway );

	TFLOAT fRange = s_fPlayerNearRange;
	TINT   iPlayer;

	if ( m_pGame->FindPlayerNear( *pSteer->GetPosition(), fRange, iPlayer ) )
	{
		m_iPlayer   = iPlayer;
		m_fFleeTime = 0.0f;
		return;
	}

	m_fFleeTime += a_fDeltaTime;

	if ( m_fFleeTime > s_fMaxFleeTime )
		SetState( STATE_FREE );
}

// $Barnyard: FUNCTION 0050c0b0
void AChasingChicksChick::UpdateThrown( TFLOAT a_fDeltaTime )
{
	TVector4 vToCentre = m_pGame->vCentre;
	vToCentre.Substract( *m_pSteer->GetPosition() );

	TFLOAT fDistanceSq = vToCentre.MagnitudeSq();

	if ( fDistanceSq < 1.0f )
	{
		// In the basket
		SetState( STATE_POCKETED );

		AChasingChicksHanna* pHanna = m_pGame->pHanna;
		pHanna->GetAnimatableObject()->PlayAnimation( TPS8( distress_pleased ) );
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_HANNA_HAPPY, *pHanna->GetSteer()->GetPosition() );
	}
	else
	{
		TVector4 vVelocity;
		m_pSteer->CopyVelocity( vVelocity );

		ARayData oRay( m_pGame->vCentre, *m_pSteer->GetPosition() );
		TFLOAT   fDistance;

		// Pulls the chick into the basket when it's close
		// NOTE: the first test can never pass here (fDistanceSq >= 1), kept as in retail
		if ( fDistanceSq * 4.0f < 4.0f ||
		     ( fDistanceSq < 4.0f && TVector4::DotProduct3( vToCentre, vVelocity ) > 0.0f && !ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, 0 ) ) )
		{
			TVector4 vPull;
			vPull.Multiply( vToCentre, 100.0f );
			vPull.Multiply( vPull, a_fDeltaTime );
			vPull.Divide( vPull, TMath::Sqrt( fDistanceSq ) );

			vVelocity.x += vPull.x;
			vVelocity.y += vPull.y;
			vVelocity.z += vPull.z;
			m_pSteer->SetVelocity( vVelocity );
		}

		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		if ( m_iChuckleCue == -1 )
		{
			if ( m_fTimer < 0.0f )
				m_iChuckleCue = pSoundManager->PlayCueAtPosition( soundbank::CHICKS_TWINK_CHUCKLE, *m_pSteer->GetPosition() );
		}
		else if ( pSoundManager->IsCueUsed( m_iChuckleCue ) && pSoundManager->GetCueSoundId( m_iChuckleCue ) == soundbank::CHICKS_TWINK_CHUCKLE )
		{
			// NOTE: ASoundManager::SetCuePosition( TINT, const TVector4& ) is a wrapper of this call
			pSoundManager->SetCuePosition( m_iChuckleCue, *m_pSteer->GetPosition(), 0.0f );
		}
		else
		{
			m_iChuckleCue = -1;
			m_fTimer      = ( ARandom::GetSingleton()->m_oRandom.GetFloat() + 1.0f ) * 0.5f;
		}

		StartTrail();
	}

	if ( m_iPlayer == 0 )
	{
		// Tilts and pulls back the camera following the chick thrown by the player
		TVector4               vVelocity = m_pSteer->GetVelocity();
		AGTAStyleCameraHelper* pHelper   = GetCameraFollowing( this );

		if ( pHelper && vVelocity.MagnitudeSqXZ() > 0.0f )
		{
			TFLOAT fPitch = TMath::ATan2( ( vVelocity.y + vVelocity.y ) / vVelocity.MagnitudeXZ(), 1.0f ) * 57.295776f;

			if ( fPitch > 0.0f )
			{
				pHelper->SetTargetPitch( fPitch + 11.0f );
				pHelper->SetDefaultPitch( fPitch + 11.0f );
			}

			pHelper->SetDefaultDistance( ( TMath::Abs( vVelocity.y ) * 0.1f + 1.0f ) * 5.0f );
		}
	}
}

// $Barnyard: FUNCTION 0050c4d0
TBOOL AChasingChicksChick::PickUp( TINT a_iPlayer )
{
	if ( !SetState( STATE_HELD ) )
		return TFALSE;

	m_iPlayer = a_iPlayer;
	ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::CHICKS_TWINK_CHUCKLE, *m_pSteer->GetPosition() );

	return TTRUE;
}

// $Barnyard: FUNCTION 0050c520
TBOOL AChasingChicksChick::Think( TFLOAT a_fDeltaTime )
{
	TINT ePhase = m_pGame->ePhase;

	if ( ePhase > AChasingChicksMiniGame::PHASE_PLAYING && ePhase <= AChasingChicksMiniGame::PHASE_ENDED && m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
	{
		// Chicks on the ground stop while the game isn't running
		m_pSteer->Command_CancelCurrentCommand();
		return AVisibleUnit::Think( a_fDeltaTime );
	}

	m_fTimer -= a_fDeltaTime;
	m_fCheckTimer -= a_fDeltaTime;

	switch ( m_eState )
	{
		case STATE_FREE:
			UpdateFree( a_fDeltaTime );
			break;
		case STATE_FLEEING:
			UpdateFleeing( a_fDeltaTime );
			break;
		case STATE_THROWN:
			UpdateThrown( a_fDeltaTime );
			break;
		case STATE_JUMPING:
			if ( TSTATICCAST( AThrowItemSteer, m_pSteer )->IsStopped() )
				SetState( STATE_LANDED );
			break;
		case STATE_POCKETED:
			if ( m_fTimer < 0.0f )
			{
				if ( m_iPlayer == 0 )
				{
					AGTAStyleCameraHelper* pHelper = GetCameraFollowing( this );

					if ( pHelper )
					{
						ResetCameraToPlayer( pHelper, m_pGame );

						if ( m_pGame->ePhase != AChasingChicksMiniGame::PHASE_ENDING && m_pGame->ePhase != AChasingChicksMiniGame::PHASE_ENDED )
							m_pGame->ePhase = AChasingChicksMiniGame::PHASE_PLAYING;
					}
				}

				SetState( STATE_RESPAWN );
			}
			break;
		default:
			break;
	}

	UpdateAnimation( a_fDeltaTime );

	// Icon on the minimap
	if ( m_iMapIcon < 0 )
	{
		m_iMapIcon = m_pGame->pMap->AddIcon( *m_pSteer->GetPosition(), *m_pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_DOT_WHITE, 0.1f );
		m_pGame->pMap->SetIconColour( m_iMapIcon, 0xffffc800 );
	}

	if ( m_iMapIcon > -1 )
	{
		m_pGame->pMap->SetIconTransform( m_iMapIcon, *m_pSteer->GetPosition(), *m_pSteer->GetDirection() );

		AChasingChicksPlayer::STATE_ ePlayerState = m_pGame->aPlayers[ 0 ].eState;
		TBOOL                        bPlayerFree  = ePlayerState == AChasingChicksPlayer::STATE_FREE || ePlayerState == AChasingChicksPlayer::STATE_STUNNED;
		m_pGame->pMap->SetIconVisible( m_iMapIcon, m_bShowOnMap && bPlayerFree );
	}

	return AVisibleUnit::Think( a_fDeltaTime );
}
