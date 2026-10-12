#include "pch.h"
#include <Render/TCameraObject.h>
#include "APrecisionRacoon.h"
#include "AUnitManager.h"
#include "ACharacterMovementComponent.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00501830, CreateObject 00501ae0, CreateObjectInPlace 00501b40 (the constructor is inlined in both)
TDEFINE_CLASS( APrecisionRacoon );

TPSTRING8_DECLARE( respawn );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( precision_slingshot );
TPSTRING8_DECLARE( precision_ball );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( cb_llocator );
TPSTRING8_DECLARE( cb_ss3 );
TPSTRING8_DECLARE( slingshot_draw );
TPSTRING8_DECLARE( slingshot_fire );

// $Barnyard: FUNCTION 005018a0
APrecisionRacoon::APrecisionRacoon()
{
	m_iNumRespawns  = 0;
	m_pPlayer       = TNULL;
	m_pNetwork      = TNULL;
	m_pSteer        = TNULL;
	m_fAimTimer     = 0.0f;
	m_eState        = STATE_IDLE;
	m_eSlingshot    = SLINGSHOT_DRAW;
	m_bHitPlayer    = TFALSE;
	m_bMilked       = TFALSE;
	m_bBallHit      = TFALSE;
	m_fOpacity      = 1.0f;
	m_fAimTime      = 1.5f;
	m_bStateChanged = TTRUE;
}

// NOTE: the deleting destructor (00501930) is shared with APrecisionCoyote
APrecisionRacoon::~APrecisionRacoon()
{
}

// $Barnyard: FUNCTION 00501cc0
TBOOL APrecisionRacoon::Create( AUnitTransform* a_pTransform )
{
	if ( !ACharacterUnit::Create( a_pTransform ) )
		return TFALSE;

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	m_pSlingshot                             = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( TPS8( precision_slingshot ) ), TNULL, 0 );
	m_pSlingshot->SetSkeletonUpdating( TFALSE, TTRUE );

	// The ball, in the right hand
	m_pBall = AUnitManager::GetSingleton()->SpawnUnit( TPS8( precision_ball ), TVector4::VEC_ZERO, TVector4::VEC_POSX );
	m_pBall->ChangeSteer( &TGetClass( AThrowItemSteer ) );
	m_pBallSteer = TSTATICCAST( AThrowItemSteer, m_pBall->GetSteer() );
	TSTATICCAST( AVisibleUnit, m_pBall )->SetVisible( TFALSE );
	m_pBallSteer->SetDirection( TVector4::VEC_POSX );
	TSTATICCAST( AVisibleUnit, m_pBall )->GetCollisionObject()->UnlinkCollisionObject();

	pObjectManager->AttachObject( TSTATICCAST( AAnimatedUnit, m_pBall )->GetAnimatableObject(), GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

	// The slingshot, in the left hand
	pObjectManager->AttachObject( m_pSlingshot, GetAnimatableObject(), TPString8(), TPS8( cb_llocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

	AddFlags( FLAGS_AI_ENABLED );
	return TTRUE;
}

// $Barnyard: FUNCTION 00501960
void APrecisionRacoon::OnDestroy()
{
	if ( m_pSlingshot )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pSlingshot );
		m_pSlingshot = TNULL;
	}

	if ( m_pBall )
	{
		AUnitManager::GetSingleton()->DestroyUnit( m_pBall );
		m_pBall = TNULL;
	}

	ACharacterUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00501840
void APrecisionRacoon::SetState( STATE_ a_eState )
{
	if ( a_eState != m_eState )
	{
		m_bStateChanged = TTRUE;
		m_eState        = a_eState;
	}

	if ( m_eState != STATE_SLINGSHOT )
	{
		m_pSlingshot->SetSkeletonUpdating( TFALSE, TTRUE );

		if ( m_pBallSteer->IsStopped() )
			TSTATICCAST( AVisibleUnit, m_pBall )->SetVisible( TFALSE );
	}
}

// $Barnyard: FUNCTION 005019b0
void APrecisionRacoon::SetNetwork( AInteriorNetwork* a_pNetwork )
{
	m_pNetwork     = a_pNetwork;
	m_iNumRespawns = 0;

	for ( TINT i = 0; i < a_pNetwork->m_pINetwork->m_iNumGoals; i++ )
	{
		AInteriorGoal* pGoal = &a_pNetwork->m_pINetwork->m_pGoals[ i ];

		if ( *pGoal->m_strName == TPS8( respawn ) )
		{
			TVector4 vPosition;
			pGoal->GetPosition( vPosition );
			m_aRespawns[ m_iNumRespawns++ ] = vPosition;
		}
	}
}

// $Barnyard: FUNCTION 00501a40
void APrecisionRacoon::SetPlayer( AUnit* a_pPlayer )
{
	m_pPlayer = a_pPlayer;

	if ( !m_pSteer )
		m_pSteer = TSTATICCAST( AInteriorSteer, GetSteer() );
}

// $Barnyard: FUNCTION 00501a60
TBOOL APrecisionRacoon::FindFarthestRespawn( const TVector4& a_rcFrom, TVector4& a_rPosition )
{
	if ( m_iNumRespawns == 0 )
		return TFALSE;

	TFLOAT fFarthest = TVector4::DistanceSq( a_rcFrom, a_rPosition );

	for ( TINT i = m_iNumRespawns; i != 0; i-- )
	{
		TFLOAT fDistance = TVector4::DistanceSq( a_rcFrom, m_aRespawns[ i - 1 ] );

		if ( fDistance > fFarthest )
		{
			fFarthest   = fDistance;
			a_rPosition = m_aRespawns[ i - 1 ];
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00501b80
void APrecisionRacoon::SetMilked( TBOOL a_bMilked )
{
	if ( a_bMilked )
	{
		RemoveFlags( FLAGS_MILK_TARGET );

		if ( a_bMilked == TTRUE && m_bMilked != TTRUE )
		{
			// NOTE: unlike SetState, the state change isn't flagged
			m_eState = STATE_MILKED;
			m_pSteer->Command_CancelCurrentCommand();
			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
			m_fOpacity = 1.0f;
		}

		m_bMilked = a_bMilked;
		return;
	}

	// Back to the spawn position
	AddFlags( FLAGS_MILK_TARGET );
	m_fAimTime      = 1.5f;
	m_fAimTimer     = 0.0f;
	m_eState        = STATE_IDLE;
	m_eSlingshot    = SLINGSHOT_DRAW;
	m_bStateChanged = TTRUE;
	m_bHitPlayer    = TFALSE;
	m_bMilked       = TFALSE;
	m_bBallHit      = TFALSE;
	m_fOpacity      = 1.0f;
	GetAnimatableObject()->KillAllAnimations();
	GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
	SetVisible( TTRUE );
	m_pSteer->Command_CancelCurrentCommand();
	SetSteerPosition( m_vSpawnPosition );

	TINT iArea = m_pNetwork->FindContainingArea( m_vSpawnPosition );

	if ( iArea > -1 )
	{
		m_pSteer->SnapToArea( iArea );
		SetSteerPosition( m_vSpawnPosition );
	}

	m_bMilked = TFALSE;
}

// $Barnyard: FUNCTION 00501e80
void APrecisionRacoon::UpdateSlingshot( TFLOAT a_fDeltaTime )
{
	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObject*        pBallObject    = TSTATICCAST( AAnimatedUnit, m_pBall )->GetAnimatableObject();

	switch ( m_eSlingshot )
	{
		case SLINGSHOT_DRAW:
		{
			m_pSlingshot->SetSkeletonUpdating( TTRUE, TTRUE );
			TSTATICCAST( AVisibleUnit, m_pBall )->SetVisible( TTRUE );

			// Put the ball in the slingshot
			if ( pBallObject->GetParentObject() )
				pObjectManager->DetachObject( pBallObject, pBallObject->GetParentObject() );

			pObjectManager->AttachObject( pBallObject, m_pSlingshot, TPString8(), TPS8( cb_ss3 ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( slingshot_draw ) );
			m_pSlingshot->PlayAnimation( TPS8( slingshot_draw ) );
			m_fAimTimer  = m_fAimTime;
			m_eSlingshot = SLINGSHOT_AIM;
			return;
		}
		case SLINGSHOT_AIM:
			m_fAimTimer -= a_fDeltaTime;

			if ( m_fAimTimer <= 0.0f )
				m_eSlingshot = SLINGSHOT_FIRE;

			return;
		case SLINGSHOT_FIRE:
		{
			pObjectManager->DetachObject( pBallObject, m_pSlingshot );

			// Lob the ball at the player (20 units per second)
			TVector4 vToPlayer;
			m_pPlayer->GetSteer()->CopyPosition( vToPlayer );

			const TVector4* pPosition = GetSteer()->GetPosition();
			vToPlayer.x -= pPosition->x;
			vToPlayer.y -= pPosition->y;
			vToPlayer.z -= pPosition->z;

			TFLOAT fDistance = vToPlayer.NormaliseMag();
			TFLOAT fAngle    = TMath::ASin( fDistance * 0.0245f ) * 0.5f;

			TVector4 vAxis;
			vAxis.CrossProduct( vToPlayer, TVector4::VEC_NEGY );

			TVector4 vDirection;
			TVector4::RotateAroundAxis( vToPlayer, vAxis, fAngle, vDirection );

			TMatrix44 oBoneTransform;
			m_pSlingshot->GetModelInstance();
			m_pSlingshot->GetModelInstance()->GetBoneTransform( m_pSlingshot->GetModelInstance()->GetBoneIndex( TPS8( cb_ss3 ) ), oBoneTransform );

			m_vBallPosition = oBoneTransform.GetTranslation();
			m_pBallSteer->SetPosition( m_vBallPosition );

			TVector4 vVelocity;
			vVelocity.Multiply( vDirection, 20.0f );
			m_pBallSteer->SetVelocity( vVelocity );

			m_bBallHit = TFALSE;
			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( slingshot_fire ) );
			m_pSlingshot->PlayAnimation( TPS8( slingshot_fire ) );
			m_eSlingshot = SLINGSHOT_FLY;
			return;
		}
		case SLINGSHOT_FLY:
		{
			if ( m_pBallSteer->IsStopped() && GetAnimatableObject()->IsAnimationFinished( TPS8( slingshot_fire ) ) )
			{
				// The ball is back in the hand
				pObjectManager->AttachObject( pBallObject, GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
				m_eSlingshot = SLINGSHOT_DRAW;
				return;
			}

			ARayData      oRay( m_vBallPosition, *m_pBallSteer->GetPosition() );
			ACollisionHit oHit;

			TRef<AUnitProxy> oHitUnit;
			AUnit*           apIgnore[ 2 ] = { this, m_pBall };

			if ( oRay.fLength > 0.0f && ACollisionManager::CollideRayUnits( oRay, 10.0f, oHitUnit, oHit, 2, apIgnore ) )
			{
				// Bounce off the unit
				TVector4 vMove;
				vMove.Multiply( oRay.vDirection, oHit.fDistance );
				m_vBallPosition.Add( vMove );
				m_pBallSteer->SetPosition( m_vBallPosition );

				TVector4 vVelocity;
				vVelocity.Multiply( oHit.vNormal, m_pBallSteer->GetVelocity().Magnitude() );
				vVelocity.Multiply( 0.1f );
				m_pBallSteer->SetVelocity( vVelocity );

				if ( !m_bBallHit && !m_bHitPlayer && oHitUnit->GetUnit() == m_pPlayer->GetSelfProxy()->GetUnit() )
				{
					m_bHitPlayer = TTRUE;
					m_bBallHit   = TTRUE;
				}
			}

			return;
		}
		default:
			return;
	}
}

// $Barnyard: FUNCTION 00502480
void APrecisionRacoon::UpdateAI( TFLOAT a_fDeltaTime )
{
	TVector4 vPosition  = *GetSteer()->GetPosition();
	TVector4 vDirection = *GetSteer()->GetDirection();

	switch ( m_eState )
	{
		case STATE_IDLE:
			if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
				GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );

			break;
		case STATE_MILKED:
			if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( yuk ), 0.0f ) )
				SetState( STATE_FLEE );

			break;
		case STATE_FLEE:
			if ( m_bStateChanged )
			{
				m_pSteer->Command_CancelCurrentCommand();
				TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 3.0f );
				m_pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
				GetAnimatableObject()->KillAllAnimations();
				GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );

				// Away from the player
				TVector4 vTarget = m_vSpawnPosition;
				FindFarthestRespawn( *m_pPlayer->GetSteer()->GetPosition(), vTarget );
				m_pSteer->SeekPointInArea( vTarget, m_pNetwork->FindContainingArea( vTarget ) );
				m_bStateChanged = TFALSE;
			}
			else if ( m_pSteer->HasArrived() )
			{
				GetAnimatableObject()->KillAllAnimations();
				AUnitManager::GetSingleton()->GetUnitHash()->RemoveHash( GetSelfProxy()->GetUnit() );
				SetState( STATE_FADE );
			}
			else
			{
				TVector4 vTarget = m_vSpawnPosition;
				FindFarthestRespawn( *m_pPlayer->GetSteer()->GetPosition(), vTarget );
				TINT iArea = m_pNetwork->FindContainingArea( vTarget );
				m_pSteer->Command_CancelCurrentCommand();
				m_pSteer->SeekPointInArea( vTarget, iArea );
			}

			break;
		case STATE_FADE:
			if ( m_fOpacity <= 0.0f )
			{
				m_fOpacity = 0.0f;
				SetState( STATE_HIDDEN );
				SetVisible( TFALSE );

				// NOTE: retail goes on with STATE_IDLE right away
				SetState( STATE_IDLE );
				break;
			}

			GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
			m_fOpacity -= 0.4f * a_fDeltaTime;
			break;
		case STATE_SLINGSHOT:
		{
			if ( m_bStateChanged )
			{
				m_pSteer->Command_CancelCurrentCommand();
				TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 0.0f );
				m_pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
				m_eSlingshot    = SLINGSHOT_DRAW;
				m_bStateChanged = TFALSE;
			}

			// Face the player
			const TVector4* pPlayerPosition = m_pPlayer->GetSteer()->GetPosition();
			const TVector4* pPosition       = m_pSteer->GetPosition();

			TVector4 vToPlayer( pPlayerPosition->x - pPosition->x, 0.0f, pPlayerPosition->z - pPosition->z, pPlayerPosition->w );
			vToPlayer.Normalise();
			m_pSteer->SetDirection( vToPlayer );

			UpdateSlingshot( a_fDeltaTime );
			break;
		}
		case STATE_CHASE:
			if ( m_bStateChanged )
			{
				m_pSteer->Command_CancelCurrentCommand();
				GetAnimatableObject()->KillAllAnimations();
				GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
				TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 2.0f );
				m_pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
				m_bStateChanged = TFALSE;
			}

			if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) )
				GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );

			m_pSteer->Command_SeekUnitPoint( m_pPlayer );
			break;
		case STATE_RETURN:
			if ( m_bStateChanged )
			{
				m_pSteer->Command_CancelCurrentCommand();
				GetAnimatableObject()->KillAllAnimations();
				GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
				TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 2.0f );
				m_pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
				m_bStateChanged = TFALSE;
				m_pSteer->SeekPointInArea( m_vSpawnPosition, m_pNetwork->FindContainingArea( m_vSpawnPosition ) );
			}
			else if ( m_pSteer->HasArrived() )
			{
				SetState( STATE_IDLE );
			}
			else if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( run_lp ), 0.0f ) )
			{
				GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			}

			break;
		default:
			break;
	}

	GetAnimatableObject()->GetModelInstance()->SetTransform( vPosition, vDirection );
	ACharacterUnit::UpdateAI( a_fDeltaTime );
}
