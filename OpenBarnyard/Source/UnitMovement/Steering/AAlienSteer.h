#pragma once
#include "APlayerSteer.h"

//-----------------------------------------------------------------------------
// Purpose: Steer of the playable characters in the sim (unit type "player").
// Adds a strafing mode on top of APlayerSteer: while strafing the character
// keeps facing a target point (turning at a limited rate) and the movement
// input is mapped to moving towards, around or away from the target. Jumping
// while strafing becomes a combat roll that circles around the target.
// AAlienAnimController picks the animations from IsStrafing/IsRolling and the
// facing angle.
// NOTE: vtable 0x00724b68, TClass 0x00788ac4 (parent APlayerSteer, version 0.4,
// size 0x628).
//-----------------------------------------------------------------------------
class AAlienSteer : public APlayerSteer
{
public:
	TDECLARE_CLASS( AAlienSteer, APlayerSteer );

public:
	AAlienSteer();
	~AAlienSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x30

	//-----------------------------------------------------------------------------
	// ACommonSteer
	//-----------------------------------------------------------------------------
	virtual TBOOL SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x174
	virtual TBOOL StopAndFace( const Toshi::TVector4& a_rcFacing ) OVERRIDE;          // +0x178

	//-----------------------------------------------------------------------------
	// APlayerSteer
	//-----------------------------------------------------------------------------
	virtual TBOOL SetState( STATE a_eState ) OVERRIDE; // +0x198
	virtual TBOOL EndState( STATE a_eState ) OVERRIDE; // +0x19c

	// Enables or disables strafing around the target point
	void SetStrafing( TBOOL a_bStrafing );

	// TTRUE while strafing or rolling
	TBOOL IsStrafingOrRolling() const;

	// TTRUE while strafing (after the first update) or rolling
	TBOOL IsStrafingActive() const;

	// The point to face while strafing
	void SetTarget( const Toshi::TVector4& a_rcTarget );

	// Desired velocity direction of the last SetDesiredVelocity while strafing
	const Toshi::TVector4* GetDesiredVelocity() const;

	// TTRUE if the character turned towards the target during the last update
	TBOOL IsTurning() const;

	// TTRUE if a combat roll can be started in the desired direction
	TBOOL CanRoll();

	TBOOL IsRolling() const;

	// Ends the combat roll and stops
	void StopRoll();

	// Angle between the facing and the direction to the target
	TFLOAT GetFacingAngle() const;

private:
	// Maps a_rDirection (in/out) to towards, around or away from the target
	void GetStrafeDirection( Toshi::TVector4& a_rDirection );

	// Bends a_rDirection (in/out) so that moving by a_fDistance circles around the target
	void CircleTarget( Toshi::TVector4& a_rDirection, TFLOAT a_fRadius, TFLOAT a_fDistance );

private:
	TBOOL           m_bStrafing;        // 0x5d8
	TBOOL           m_bStrafingUpdated; // 0x5d9, set by Update while strafing
	TBOOL           m_bTurningToTarget; // 0x5da
	TBOOL           m_bRolling;         // 0x5db
	Toshi::TVector4 m_vTarget;          // 0x5dc, not initialised by the ctor
	TINT            PADDING[ 4 ];       // 0x5ec, unused
	Toshi::TVector4 m_vDesiredVelocity; // 0x5fc
	Toshi::TVector4 m_vRollVelocity;    // 0x60c, not initialised by the ctor
	TFLOAT          m_fDesiredSpeed;    // 0x61c, not initialised by the ctor
	TFLOAT          m_fStrafeAngle;     // 0x620, not initialised by the ctor
	TFLOAT          m_fFacingAngle;     // 0x624, not initialised by the ctor
};

TSTATICASSERT( sizeof( AAlienSteer ) == 0x628 );
