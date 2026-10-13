#include "pch.h"
#include "APrecisionJersyCow.h"
#include "ACharacterMovementComponent.h"
#include "Animation/AAnimatableObject.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005010b0, CreateObject 00501490, CreateObjectInPlace 005014f0 (the constructor is inlined in both)
TDEFINE_CLASS( APrecisionJersyCow );

TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( yuk_start );
TPSTRING8_DECLARE( yuk_end );
TPSTRING8_DECLARE( yawn );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( run_lp );

// $Barnyard: FUNCTION 00501240
APrecisionJersyCow::APrecisionJersyCow()
{
	m_oWaypoints.iCount = 0;
	m_bMilked           = TFALSE;
	m_bHitPlayer        = TFALSE;
	m_iWaypoint         = 0;
	m_iCowIndex         = 0;
	m_pPlayer           = TNULL;
	m_pSteer            = TNULL;
	m_bStateChanged     = TTRUE;
	m_eState            = STATE_IDLE;
	m_fSquirtTimer      = 999.0f;
}

// $Barnyard: FUNCTION 005012a0
// $Barnyard: FUNCTION 005012b0
// $Barnyard: FUNCTION 005012d0
APrecisionJersyCow::~APrecisionJersyCow()
{
}

// $Barnyard: FUNCTION 00501530
TBOOL APrecisionJersyCow::Create( AUnitTransform* a_pTransform )
{
	AddFlags( FLAGS_MILK_TARGET );

	if ( !AUnitPlayer::Create( a_pTransform ) )
		return TFALSE;

	// The milk of the cow
	GetLauncher( 1 )->GetAmmo()->SetProjectileType( ASimpleProjectileManager::TYPE_MILK_PRECISION );
	TSTATICCAST( AAimedSquirtProjectileLauncher, GetLauncher( 1 ) )->SetNPCControlled( TTRUE );
	SetHashed( TTRUE );
	AddFlags( FLAGS_AI_ENABLED );

	return TTRUE;
}

// NOTE: only calls the base (a jump to AUnitPlayer::OnDestroy in retail)
// $Barnyard: FUNCTION 005010d0
void APrecisionJersyCow::OnDestroy()
{
	AUnitPlayer::OnDestroy();
}

// $Barnyard: FUNCTION 005010c0
void APrecisionJersyCow::SetHitPlayer( TBOOL a_bHit )
{
	m_bHitPlayer = a_bHit;
}

// $Barnyard: FUNCTION 00501220
void APrecisionJersyCow::SetState( STATE_ a_eState )
{
	if ( a_eState != m_eState )
	{
		m_bStateChanged = TTRUE;
		m_eState        = a_eState;
	}
}

// $Barnyard: FUNCTION 005012e0
void APrecisionJersyCow::SetMilked( TBOOL a_bMilked )
{
	if ( a_bMilked )
	{
		m_fMilkedTime = 0.0f;

		if ( m_bMilked != a_bMilked )
		{
			SetState( STATE_MILKED );
			m_pSteer->Command_CancelCurrentCommand();
			GetAnimatableObject()->KillAllAnimations();
			GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
		}
	}

	m_bMilked = a_bMilked;
}

// $Barnyard: FUNCTION 00501360
void APrecisionJersyCow::SetPlayer( AUnit* a_pPlayer )
{
	m_pPlayer = a_pPlayer;

	if ( !m_pSteer )
		m_pSteer = TSTATICCAST( AInteriorSteer, GetSteer() );
}

// $Barnyard: FUNCTION 005010e0
TBOOL APrecisionJersyCow::UpdateMilked( TFLOAT a_fDeltaTime )
{
	m_fMilkedTime += a_fDeltaTime;

	if ( m_iMilkedState == MILKSTATE_START )
	{
		GetAnimatableObject()->PlayAnimation( TPS8( yuk_start ) );
		m_iMilkedState = MILKSTATE_YUK;
	}
	else if ( m_iMilkedState == MILKSTATE_YUK )
	{
		if ( m_fMilkedTime > 16.0f )
		{
			GetAnimatableObject()->PlayAnimation( TPS8( yuk_end ) );
			m_iMilkedState = MILKSTATE_END;
			return TFALSE;
		}
	}
	else if ( m_iMilkedState == MILKSTATE_END )
	{
		if ( GetAnimatableObject()->IsAnimationFinished( TPS8( yuk_end ) ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00501180
TBOOL APrecisionJersyCow::UpdateIdle( TFLOAT a_fDeltaTime )
{
	m_fIdleTime += a_fDeltaTime;

	if ( m_fIdleTime > 4.0f )
		return TTRUE;

	if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) && !GetAnimatableObject()->IsAnimPlaying( TPS8( yawn ), 0.0f ) )
	{
		// NOTE: retail picks the animation with the frame time, which is almost always below 1 (yawn)
		if ( TINT( TINT64( a_fDeltaTime ) ) % 2 != 0 )
		{
			GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
			return TFALSE;
		}

		GetAnimatableObject()->PlayAnimation( TPS8( yawn ) );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00501380
void APrecisionJersyCow::UpdateSquirt( TFLOAT a_fDeltaTime )
{
	TVector4 vPlayerPosition = *m_pPlayer->GetSteer()->GetPosition();
	TVector4 vToPlayer       = vPlayerPosition;

	const TVector4* pPosition = m_pSteer->GetPosition();
	vToPlayer.x               = vPlayerPosition.x - pPosition->x;
	vToPlayer.y               = 0.0f;
	vToPlayer.z               = vPlayerPosition.z - pPosition->z;
	vToPlayer.Normalise();

	// Face the player
	m_pSteer->GetKinematics()->SetDesiredSpeed( 0.0f );
	m_pSteer->GetKinematics()->SetDesiredFacing( vToPlayer );

	m_fSquirtTimer += a_fDeltaTime;

	if ( m_fSquirtTimer > 1.6f )
	{
		m_fSquirtTimer = 0.0f;
		vPlayerPosition.y -= 0.7f;
		TSTATICCAST( AAimedSquirtProjectileLauncher, GetLauncher( 1 ) )->FireAt( vPlayerPosition, TFALSE );
	}
}

// $Barnyard: FUNCTION 00501580
void APrecisionJersyCow::UpdateAI( TFLOAT a_fDeltaTime )
{
	GetLauncher( 1 )->Update( a_fDeltaTime );

	TVector4 vPosition  = *GetSteer()->GetPosition();
	TVector4 vDirection = *GetSteer()->GetDirection();

	switch ( m_eState )
	{
		case STATE_RUN:
			if ( m_bStateChanged )
			{
				m_bStateChanged = TFALSE;
				m_iWaypoint++;

				if ( m_iWaypoint >= m_oWaypoints.iCount )
					m_iWaypoint = 0;

				TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 1.5f );
				m_pSteer->Command_CancelCurrentCommand();
				m_pSteer->SetMode( AInteriorSteer::MODE_NORMAL );
				m_pSteer->SeekPosition( m_oWaypoints.aPositions[ m_iWaypoint ] );
				GetAnimatableObject()->KillAllAnimations();
				GetAnimatableObject()->PlayAnimation( TPS8( run_lp ) );
			}
			else if ( m_pSteer->HasArrived() || m_pSteer->IsOnSpecialPathArea() )
			{
				SetState( STATE_IDLE );
			}

			break;
		case STATE_IDLE:
			if ( m_bStateChanged )
			{
				m_bStateChanged = TFALSE;
				m_fIdleTime     = 0.0f;

				// NOTE: retail picks the animation with the frame time, which is almost always below 1 (yawn)
				GetAnimatableObject()->PlayAnimation( ( TINT( TINT64( a_fDeltaTime ) ) % 2 != 0 ) ? TPS8( idle_lp ) : TPS8( yawn ) );
			}

			if ( UpdateIdle( a_fDeltaTime ) )
				SetState( STATE_RUN );

			break;
		case STATE_MILKED:
			if ( m_bStateChanged )
			{
				m_bStateChanged = TFALSE;
				m_fMilkedTime   = 0.0f;
				m_iMilkedState  = MILKSTATE_START;
			}

			if ( UpdateMilked( a_fDeltaTime ) )
			{
				m_bMilked = TFALSE;
				SetState( STATE_IDLE );
			}

			break;
		case STATE_SQUIRT:
			if ( m_bStateChanged )
			{
				m_bStateChanged = TFALSE;
				m_pSteer->Command_CancelCurrentCommand();
			}

			UpdateSquirt( a_fDeltaTime );
			break;
		case STATE_UNK4:
			if ( m_bStateChanged )
				m_bStateChanged = TFALSE;

			break;
		default:
			break;
	}

	GetAnimatableObject()->GetModelInstance()->SetTransform( vPosition, vDirection );
	AUnit::UpdateAI( a_fDeltaTime );
}
