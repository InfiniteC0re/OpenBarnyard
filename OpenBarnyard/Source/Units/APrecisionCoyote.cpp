#include "pch.h"
#include "APrecisionCoyote.h"
#include "AUnitBehaviour.h"
#include "AUnitManager.h"
#include "ACharacterMovementComponent.h"
#include "FSM/AMethodPointer.h"
#include "Animation/AAnimatableObject.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005006d0, CreateObject 00500a00, CreateObjectInPlace 00500a60 (the constructor is inlined in both)
TDEFINE_CLASS_INIT( APrecisionCoyote );

TPSTRING8_DECLARE( respawn );
TPSTRING8_DECLARE( sharpcoyoteattack );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( run_lp );

// 007833e8
AFunctionTable APrecisionCoyote::ms_oFunctionTable( &TGetClass( APrecisionCoyote ), &ACharacterUnit::ms_oFunctionTable );

// $Barnyard: FUNCTION 00500c30
void APrecisionCoyote::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<APrecisionCoyote>::Create( "PlayerAttacked", &APrecisionCoyote::PlayerAttacked ) );
}

// $Barnyard: FUNCTION 005006e0
void APrecisionCoyote::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00500730
APrecisionCoyote::APrecisionCoyote()
{
	m_iNumRespawns  = 0;
	m_pPlayer       = TNULL;
	m_pNetwork      = TNULL;
	m_pSteer        = TNULL;
	m_eState        = STATE_FSM;
	m_bMilked       = TFALSE;
	m_fOpacity      = 1.0f;
	m_fUnkA8        = 2.0f;
	m_fAttackTimer  = 2.0f;
	m_bStateChanged = TTRUE;
}

// $Barnyard: FUNCTION 00501930
// $Barnyard: FUNCTION 00501950
// NOTE: the deleting destructor (00501930) is shared with APrecisionRacoon
APrecisionCoyote::~APrecisionCoyote()
{
}

// $Barnyard: FUNCTION 005007a0
AFunctionTable* APrecisionCoyote::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00500aa0
TBOOL APrecisionCoyote::Create( AUnitTransform* a_pTransform )
{
	if ( !ACharacterUnit::Create( a_pTransform ) )
		return TFALSE;

	AddFlags( FLAGS_AI_ENABLED );
	return TTRUE;
}

// $Barnyard: FUNCTION 005006f0
void APrecisionCoyote::PlayerAttacked()
{
	m_fAttackTimer = 0.0f;
}

// $Barnyard: FUNCTION 00500700
void APrecisionCoyote::SetState( STATE_ a_eState )
{
	if ( a_eState != m_eState )
	{
		m_bStateChanged = TTRUE;
		m_eState        = a_eState;
	}
}

// $Barnyard: FUNCTION 005007b0
void APrecisionCoyote::SetNetwork( AInteriorNetwork* a_pNetwork )
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

// $Barnyard: FUNCTION 00500840
void APrecisionCoyote::SetPlayer( AUnit* a_pPlayer )
{
	m_pPlayer = a_pPlayer;

	AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, FindComponentExact( &TGetClass( AUnitBehaviour ) ) );
	pBehaviour->SetCommand( TPS8( sharpcoyoteattack ), TNULL, TTRUE );

	if ( !m_pSteer )
		m_pSteer = TSTATICCAST( AInteriorSteer, GetSteer() );
}

// $Barnyard: FUNCTION 00500880
TBOOL APrecisionCoyote::FindFarthestRespawn( const TVector4& a_rcFrom, TVector4& a_rPosition )
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

// $Barnyard: FUNCTION 00500ac0
void APrecisionCoyote::SetMilked( TBOOL a_bMilked )
{
	if ( a_bMilked )
	{
		RemoveFlags( FLAGS_MILK_TARGET );

		if ( a_bMilked == TTRUE && m_bMilked != TTRUE )
		{
			SetState( STATE_MILKED );

			TSTATICCAST( AUnitBehaviour, FindComponentExact( &TGetClass( AUnitBehaviour ) ) )->ClearCommand();
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
	m_eState        = STATE_FSM;
	m_fUnkA8        = 2.0f;
	m_fAttackTimer  = 2.0f;
	m_bStateChanged = TTRUE;
	m_bMilked       = TFALSE;
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

	TSTATICCAST( AUnitBehaviour, FindComponentExact( &TGetClass( AUnitBehaviour ) ) )->SetCommand( TPS8( sharpcoyoteattack ), TNULL, TTRUE );
	m_bMilked = TFALSE;
}

// $Barnyard: FUNCTION 00500c60
void APrecisionCoyote::UpdateAI( TFLOAT a_fDeltaTime )
{
	TVector4 vPosition  = *GetSteer()->GetPosition();
	TVector4 vDirection = *GetSteer()->GetDirection();

	switch ( m_eState )
	{
		case STATE_MILKED:
			if ( m_bStateChanged )
			{
				m_pSteer->Command_CancelCurrentCommand();
				TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) )->SetRunningSpeed( ACharacterMovementComponent::APPLY_IF_MOVING, 0.0f );
				GetAnimatableObject()->KillAllAnimations();
				GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
				m_bStateChanged = TFALSE;
			}

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
				TINT iArea = m_pNetwork->FindContainingArea( vTarget );
				m_pSteer->Command_CancelCurrentCommand();
				m_pSteer->SeekPointInArea( vTarget, iArea );
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

				// NOTE: retail goes on with STATE_FSM right away
				SetState( STATE_FSM );
				break;
			}

			GetAnimatableObject()->SetOpacity( m_fOpacity, TTRUE );
			m_fOpacity -= 0.4f * a_fDeltaTime;
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

				TINT iArea = m_pNetwork->FindContainingArea( m_vSpawnPosition );
				m_pSteer->Command_CancelCurrentCommand();
				m_pSteer->SeekPointInArea( m_vSpawnPosition, iArea );
			}
			else if ( m_pSteer->HasArrived() )
			{
				SetState( STATE_FSM );
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
