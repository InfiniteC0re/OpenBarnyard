#pragma once
#include "AUnitComponent.h"

// Picks the walk/run speed of a character (ACharacterUnit, AUnitPlayer) by changing its
// steer's speed modifier. A run desire can be timed (see SetDesireRun), the owner's Think
// falls back to walking once the time is up.
//
// TClass 0x00784098 (version 0.2, size 0x18, runtime creatable), vtable 0x00724a18.
// Created for every unit whose type has ACharacterMovementData.
class ACharacterMovementComponent : public AUnitComponent
{
public:
	TDECLARE_CLASS( ACharacterMovementComponent, AUnitComponent );

	// How the new speed is applied to the owner's steer
	using APPLY = TINT;
	enum APPLY_ : APPLY
	{
		APPLY_MODIFIER_ONLY = 0, // only changes the steer's speed modifier
		APPLY_IF_MOVING     = 1, // also sets the desired speed if the steer is moving
		APPLY_IMMEDIATELY   = 2, // also sets the steer's speed right away
	};

	// Index of the ASteerTraits logical speed the speed is relative to
	using SPEED = TINT;
	enum SPEED_ : SPEED
	{
		SPEED_IDLE   = 0, // m_fLogicalIdleSpeed
		SPEED_WALK   = 1, // m_fLogicalWalkSpeed
		SPEED_RUN    = 2, // m_fLogicalRunSpeed, keeps the run timer
		SPEED_SPRINT = 3, // m_fLogicalSprintSpeed
	};

public:
	// NOTE: inlined into CreateObject 0054f4d0 / CreateObjectInPlace 0054f540 / ACharacterMovementData::OnUnitCreate
	ACharacterMovementComponent()
	    : m_fRunTimer( -1.0f )
	    , m_fWalkSpeed( 1.0f )
	{}

	~ACharacterMovementComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AUnitComponentManager* GetManager() OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	virtual void   SetWalkSpeed( TFLOAT a_fSpeed ) { m_fWalkSpeed = a_fSpeed; } // +0x38, folded setter 0054f230
	virtual TFLOAT GetWalkSpeed() { return m_fWalkSpeed; }                      // +0x3c, folded getter 0058ac50
	virtual TFLOAT GetRunSpeed() { return 1.0f; }                               // +0x40, folded 006bf680

	// Sets the steer's speed modifier to a_fSpeed times the a_eSpeed logical speed,
	// relative to the logical run speed. Clears the run timer unless a_eSpeed is SPEED_RUN.
	void SetSpeed( APPLY a_eApply, TFLOAT a_fSpeed, SPEED a_eSpeed );

	void SetWalkingSpeed( APPLY a_eApply, TFLOAT a_fSpeed );
	void SetRunningSpeed( APPLY a_eApply, TFLOAT a_fSpeed );

	// Walks at GetWalkSpeed()
	void ResetSpeed( APPLY a_eApply );

	void SetDesireWalk();

	// Runs for a_fTime seconds (until SetDesireWalk is called for TMath::MAXFLOAT)
	void SetDesireRun( TFLOAT a_fTime );
	void SetDesireRun();

	// Counted down by the owner's Think (ACharacterUnit::Think)
	TFLOAT GetRunTimer() const { return m_fRunTimer; }
	void   SetRunTimer( TFLOAT a_fTime ) { m_fRunTimer = a_fTime; }

private:
	TFLOAT m_fRunTimer;  // 0x10, seconds of running left, <= 0 if not timed
	TFLOAT m_fWalkSpeed; // 0x14
};

TSTATICASSERT( sizeof( ACharacterMovementComponent ) == 0x18 );
