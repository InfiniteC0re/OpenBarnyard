#include "pch.h"
#include "ASteerTraits.h"
#include "ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00579050, CreateObject 005794e0, CreateObjectInPlace 00579540
TDEFINE_CLASS( ASteerTraits );

TPSTRING8_DECLARE( Radius );
TPSTRING8_DECLARE( Length );
TPSTRING8_DECLARE( Mass );
TPSTRING8_DECLARE( NHolonomicRate );
TPSTRING8_DECLARE( GoalToleranceSq );
TPSTRING8_DECLARE( Arrival );
TPSTRING8_DECLARE( MaxForce );
TPSTRING8_DECLARE( MaxSpeed );
TPSTRING8_DECLARE( Acc );
TPSTRING8_DECLARE( Dec );
TPSTRING8_DECLARE( TurningAcc );
TPSTRING8_DECLARE( TurningDec );
TPSTRING8_DECLARE( AnimatedWalkSpeed );
TPSTRING8_DECLARE( AnimatedRunSpeed );
TPSTRING8_DECLARE( AnimatedSprintSpeed );
TPSTRING8_DECLARE( AnimatedClimbSpeed );
TPSTRING8_DECLARE( LogicalIdleSpeed );
TPSTRING8_DECLARE( LogicalWalkSpeed );
TPSTRING8_DECLARE( LogicalRunSpeed );
TPSTRING8_DECLARE( LogicalSprintSpeed );
TPSTRING8_DECLARE( MinLogicalClimbSpeed );
TPSTRING8_DECLARE( MaxLogicalClimbSpeed );
TPSTRING8_DECLARE( MaxAesWalkSpeed );
TPSTRING8_DECLARE( MaxAesRunSpeed );
TPSTRING8_DECLARE( SideStepSpeed );
TPSTRING8_DECLARE( JumpPower );
TPSTRING8_DECLARE( HeavyCarrySpeedFactor );
TPSTRING8_DECLARE( MaxAesRunSpeedRatio );
TPSTRING8_DECLARE( combatacc );
TPSTRING8_DECLARE( combatdec );
TPSTRING8_DECLARE( combatmaxspeed );
TPSTRING8_DECLARE( combatturnacc );
TPSTRING8_DECLARE( combatanimrunspeed );
TPSTRING8_DECLARE( combatjumpheight );
TPSTRING8_DECLARE( bounce );

// $Barnyard: FUNCTION 00579060
ASteerTraits::ASteerTraits()
{
	// NOTE: m_fRadius is left uninitialised, like retail
	m_fLength                = 0.0f;
	m_fNHolonomicRate        = 1.0f;
	m_fGoalToleranceSq       = 2.0f;
	m_fArrival               = 10.0f;
	m_fMaxSpeed              = 5.0f;
	m_fMass                  = 1200.0f;
	m_fAcc                   = 1.0f;
	m_fDec                   = 1.0f;
	m_fTurningAcc            = 1.0f;
	m_fTurningDec            = 1.0f;
	m_fMaxForce              = 500.0f;
	m_fAnimatedWalkSpeed     = -1.0f;
	m_fAnimatedRunSpeed      = -1.0f;
	m_fAnimatedSprintSpeed   = -1.0f;
	m_fAnimatedClimbSpeed    = 7.7f;
	m_fLogicalIdleSpeed      = 0.4f;
	m_fLogicalWalkSpeed      = 1.0f;
	m_fLogicalRunSpeed       = 1.0f;
	m_fLogicalSprintSpeed    = 1.0f;
	m_fMaxAesWalkSpeed       = -1.0f;
	m_fMaxAesRunSpeed        = -1.0f;
	m_fMaxAesRunSpeedRatio   = -1.0f;
	m_fMinLogicalClimbSpeed  = 0.06f;
	m_fMaxLogicalClimbSpeed  = 4.0f;
	m_fSideStepSpeed         = 5.588f;
	m_fJumpPower             = 10.0f;
	m_fHeavyCarrySpeedFactor = 0.5f;
	m_fCombatMaxSpeed        = 6.5f;
	m_fCombatDec             = 30.0f;
	m_fCombatAcc             = 15.0f;
	m_fCombatTurnAcc         = 160.0f;
	m_fCombatAnimRunSpeed    = 6.5f;
	m_fCombatJumpHeight      = 1.2f;
	m_fBounce                = 1.0f;
}

// $Barnyard: FUNCTION 00579580
// $Barnyard: FUNCTION 00579140
ASteerTraits::~ASteerTraits()
{
}

// $Barnyard: FUNCTION 00579150
TBOOL ASteerTraits::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_fRadius, TPS8( Radius ) );
	a_pProperties->GetOptionalPropertyValue( m_fLength, TPS8( Length ) );
	a_pProperties->GetOptionalPropertyValue( m_fMass, TPS8( Mass ) );
	a_pProperties->GetOptionalPropertyValue( m_fNHolonomicRate, TPS8( NHolonomicRate ) );
	a_pProperties->GetOptionalPropertyValue( m_fGoalToleranceSq, TPS8( GoalToleranceSq ) );
	a_pProperties->GetOptionalPropertyValue( m_fArrival, TPS8( Arrival ) );
	a_pProperties->GetOptionalPropertyValue( m_fMaxForce, TPS8( MaxForce ) );
	a_pProperties->GetOptionalPropertyValue( m_fMaxSpeed, TPS8( MaxSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fAcc, TPS8( Acc ) );
	a_pProperties->GetOptionalPropertyValue( m_fDec, TPS8( Dec ) );
	a_pProperties->GetOptionalPropertyValue( m_fTurningAcc, TPS8( TurningAcc ) );
	a_pProperties->GetOptionalPropertyValue( m_fTurningDec, TPS8( TurningDec ) );
	a_pProperties->GetOptionalPropertyValue( m_fAnimatedWalkSpeed, TPS8( AnimatedWalkSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fAnimatedRunSpeed, TPS8( AnimatedRunSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fAnimatedSprintSpeed, TPS8( AnimatedSprintSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fAnimatedClimbSpeed, TPS8( AnimatedClimbSpeed ) );

	// NOTE: retail seeds the logical run speed from the animated walk speed and the
	// logical walk speed from the animated run speed (crossed over)
	m_fLogicalRunSpeed  = m_fAnimatedWalkSpeed;
	m_fLogicalWalkSpeed = m_fAnimatedRunSpeed;

	a_pProperties->GetOptionalPropertyValue( m_fLogicalIdleSpeed, TPS8( LogicalIdleSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fLogicalWalkSpeed, TPS8( LogicalWalkSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fLogicalRunSpeed, TPS8( LogicalRunSpeed ) );

	if ( m_fLogicalRunSpeed < m_fLogicalWalkSpeed )
		m_fLogicalRunSpeed = m_fLogicalWalkSpeed;

	a_pProperties->GetOptionalPropertyValue( m_fLogicalSprintSpeed, TPS8( LogicalSprintSpeed ) );

	if ( m_fLogicalSprintSpeed < m_fLogicalRunSpeed )
		m_fLogicalSprintSpeed = m_fLogicalRunSpeed;

	a_pProperties->GetOptionalPropertyValue( m_fMinLogicalClimbSpeed, TPS8( MinLogicalClimbSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fMaxLogicalClimbSpeed, TPS8( MaxLogicalClimbSpeed ) );

	if ( 0.0f < m_fAnimatedWalkSpeed && 0.0f < m_fAnimatedRunSpeed )
	{
		a_pProperties->GetOptionalPropertyValue( m_fMaxAesWalkSpeed, TPS8( MaxAesWalkSpeed ) );

		// Not set: a third of the way from walking to running
		if ( m_fMaxAesWalkSpeed <= 0.0f )
			m_fMaxAesWalkSpeed = ( m_fAnimatedRunSpeed - m_fAnimatedWalkSpeed ) * 0.33f + m_fAnimatedWalkSpeed;

		if ( 0.0f < m_fAnimatedSprintSpeed )
		{
			a_pProperties->GetOptionalPropertyValue( m_fMaxAesRunSpeed, TPS8( MaxAesRunSpeed ) );

			if ( m_fMaxAesRunSpeed <= 0.0f )
				m_fMaxAesRunSpeed = m_fAnimatedRunSpeed;
		}
	}

	a_pProperties->GetOptionalPropertyValue( m_fSideStepSpeed, TPS8( SideStepSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fJumpPower, TPS8( JumpPower ) );
	a_pProperties->GetOptionalPropertyValue( m_fHeavyCarrySpeedFactor, TPS8( HeavyCarrySpeedFactor ) );
	a_pProperties->GetOptionalPropertyValue( m_fMaxAesRunSpeedRatio, TPS8( MaxAesRunSpeedRatio ) );
	a_pProperties->GetOptionalPropertyValue( m_fCombatAcc, TPS8( combatacc ) );
	a_pProperties->GetOptionalPropertyValue( m_fCombatDec, TPS8( combatdec ) );
	a_pProperties->GetOptionalPropertyValue( m_fCombatMaxSpeed, TPS8( combatmaxspeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fCombatTurnAcc, TPS8( combatturnacc ) );
	a_pProperties->GetOptionalPropertyValue( m_fCombatAnimRunSpeed, TPS8( combatanimrunspeed ) );
	a_pProperties->GetOptionalPropertyValue( m_fCombatJumpHeight, TPS8( combatjumpheight ) );
	a_pProperties->GetOptionalPropertyValue( m_fBounce, TPS8( bounce ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00579440
TFLOAT ASteerTraits::GetAnimatedSpeed( TINT a_eMovement ) const
{
	switch ( a_eMovement )
	{
		case ASteer::MOVEMENT_STOPPED: return 0.05f;
		case ASteer::MOVEMENT_WALK: return m_fAnimatedWalkSpeed;
		case ASteer::MOVEMENT_RUN: return m_fAnimatedRunSpeed;
		case ASteer::MOVEMENT_SPRINT: return m_fAnimatedSprintSpeed;
		default: return 1.0f;
	}
}

// $Barnyard: FUNCTION 00579490
TFLOAT ASteerTraits::GetLogicalSpeed( TINT a_eMovement ) const
{
	switch ( a_eMovement )
	{
		case ASteer::MOVEMENT_STOPPED: return m_fLogicalIdleSpeed;
		case ASteer::MOVEMENT_WALK: return m_fLogicalWalkSpeed;
		case ASteer::MOVEMENT_RUN: return m_fLogicalRunSpeed;
		case ASteer::MOVEMENT_SPRINT: return m_fLogicalSprintSpeed;
		default: return 1.0f;
	}
}
