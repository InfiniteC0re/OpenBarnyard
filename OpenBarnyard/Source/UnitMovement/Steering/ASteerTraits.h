#pragma once
#include "Units/AUnitStaticData.h"

// Movement tuning of a unit type, read from the type's "ASteerTraits" properties.
// AUnit::GetSteerTraits falls back to a default-constructed instance when a type has none.
class ASteerTraits : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ASteerTraits, AUnitStaticData );

public:
	ASteerTraits();
	~ASteerTraits();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	// Speed the movement animation of a_eMovement (ASteer::Movement) is made for
	TFLOAT GetAnimatedSpeed( TINT a_eMovement ) const;

	// Logical speed of a_eMovement (ASteer::Movement / ACharacterMovementComponent::SPEED)
	TFLOAT GetLogicalSpeed( TINT a_eMovement ) const;

public:
	TFLOAT m_fRadius;                // 0x0c
	TFLOAT m_fLength;                // 0x10
	TFLOAT m_fNHolonomicRate;        // 0x14
	TFLOAT m_fGoalToleranceSq;       // 0x18
	TFLOAT m_fArrival;               // 0x1c
	TFLOAT m_fMaxSpeed;              // 0x20
	TFLOAT m_fMass;                  // 0x24
	TFLOAT m_fAcc;                   // 0x28
	TFLOAT m_fDec;                   // 0x2c
	TFLOAT m_fTurningAcc;            // 0x30
	TFLOAT m_fTurningDec;            // 0x34
	TFLOAT m_fMaxForce;              // 0x38
	TFLOAT m_fAnimatedWalkSpeed;     // 0x3c
	TFLOAT m_fAnimatedRunSpeed;      // 0x40
	TFLOAT m_fAnimatedSprintSpeed;   // 0x44
	TFLOAT m_fAnimatedClimbSpeed;    // 0x48
	TFLOAT m_fLogicalIdleSpeed;      // 0x4c
	TFLOAT m_fLogicalWalkSpeed;      // 0x50
	TFLOAT m_fLogicalRunSpeed;       // 0x54
	TFLOAT m_fLogicalSprintSpeed;    // 0x58
	TFLOAT m_fMaxAesWalkSpeed;       // 0x5c
	TFLOAT m_fMaxAesRunSpeed;        // 0x60
	TFLOAT m_fMaxAesRunSpeedRatio;   // 0x64
	TFLOAT m_fMinLogicalClimbSpeed;  // 0x68
	TFLOAT m_fMaxLogicalClimbSpeed;  // 0x6c
	TFLOAT m_fSideStepSpeed;         // 0x70
	TFLOAT m_fJumpPower;             // 0x74
	TFLOAT m_fHeavyCarrySpeedFactor; // 0x78
	TFLOAT m_fCombatMaxSpeed;        // 0x7c
	TFLOAT m_fCombatDec;             // 0x80
	TFLOAT m_fCombatAcc;             // 0x84
	TFLOAT m_fCombatTurnAcc;         // 0x88
	TFLOAT m_fCombatAnimRunSpeed;    // 0x8c
	TFLOAT m_fCombatJumpHeight;      // 0x90
	TFLOAT m_fBounce;                // 0x94
};

TSTATICASSERT( sizeof( ASteerTraits ) == 0x98 );
