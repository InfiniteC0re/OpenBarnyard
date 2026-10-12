#pragma once
#include "ASteerKinematics.h"

//-----------------------------------------------------------------------------
// Purpose: Kinematics of the NPC steers (AInteriorSteer). Moves on the ground
// plane (the steer resolves the height), turns with a turning speed that
// accelerates towards the desired facing, integrates the movement with fixed
// steps of 1/30s, seeks a target along a line, steers around and pushes itself
// out of kinematics obstructions. When something blocks it, it waits and then
// backs off for a while (the avoid states). When it got pushed away while
// standing still, it walks back to where it stood.
// NOTE: vtable 0x00727818, TClass 0x00789680 (version 5.0, size 0xe8).
//-----------------------------------------------------------------------------
class ASteerKinematics2D : public ASteerKinematics
{
public:
	TDECLARE_CLASS( ASteerKinematics2D, ASteerKinematics );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_UNK0          = BITFLAG( 0 ),  // Unk_0x9c (maybe JPOG's SetLockToGround: interior units set it, the player clears it), set by Create/CreateFrom, never tested
		FLAGS_FROZEN        = BITFLAG( 1 ),  // SetFrozen, Update does nothing
		FLAGS_SEEKING       = BITFLAG( 2 ),  // following the line from m_vSeekStart to m_vSeekTarget
		FLAGS_SEEK_FACING   = BITFLAG( 4 ),  // stops at m_vSeekTarget facing m_vSeekFacing
		FLAGS_MOVED         = BITFLAG( 5 ),  // the position changed during the last Update
		FLAGS_SEEK_SLOWDOWN = BITFLAG( 6 ),  // SetSeekSlowdown, slows down close to the seek target
		FLAGS_NO_RETURN     = BITFLAG( 7 ),  // doesn't walk back to the position it got pushed from
		FLAGS_BLOCKED       = BITFLAG( 8 ),  // something is in the way, waits (m_fAvoidTimer)
		FLAGS_AVOIDING      = BITFLAG( 9 ),  // waits until the way is free (checked every 0.3s)
		FLAGS_AVOIDED       = BITFLAG( 10 ), // cooldown after FLAGS_AVOIDING
		FLAGS_CHECK_BLOCKED = BITFLAG( 11 ), // checks if the seek is blocked
	};

public:
	ASteerKinematics2D();
	~ASteerKinematics2D();

	//-----------------------------------------------------------------------------
	// ASteerKinematics
	//-----------------------------------------------------------------------------
	virtual TBOOL                  Create( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, const ASteerTraits* a_pTraits, TFLOAT a_fSpeed ) OVERRIDE;                          // +0x0c
	virtual TBOOL                  CreateFrom( ASteer* a_pOther ) OVERRIDE;                                                                                                                               // +0x10
	virtual TFLOAT                 GetMeasuredSpeedRatio() OVERRIDE;                                                                                                                                      // +0x14
	virtual TFLOAT                 GetMeasuredSpeed() OVERRIDE;                                                                                                                                           // +0x18
	virtual void                   BlendMeasuredSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                                                        // +0x20
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                                                                                                                                // +0x24
	virtual Toshi::TVector4        Unk_0x28() OVERRIDE;                                                                                                                                                   // +0x28
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                                                                                                                                // +0x2c
	virtual const Toshi::TVector4* GetFacingDir() OVERRIDE;                                                                                                                                               // +0x30
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                                                                                                                                   // +0x38
	virtual TFLOAT                 GetDesiredSpeed() OVERRIDE;                                                                                                                                            // +0x3c
	virtual const Toshi::TVector4* GetSeekTarget() OVERRIDE;                                                                                                                                              // +0x44
	virtual TBOOL                  IsFacingAtDesiredFacing( TFLOAT a_fTolerance ) OVERRIDE;                                                                                                               // +0x48
	virtual void                   SetDesiredFacing( const Toshi::TVector4& a_rcFacing ) OVERRIDE;                                                                                                        // +0x4c
	virtual const Toshi::TVector4* GetDesiredFacing() OVERRIDE;                                                                                                                                           // +0x50
	virtual TFLOAT                 GetGroundHeight() OVERRIDE;                                                                                                                                            // +0x54
	virtual Toshi::TVector4&       PredictPosition( TINT a_iTime, Toshi::TVector4& a_rOut ) OVERRIDE;                                                                                                     // +0x58
	virtual Toshi::TVector2*       GetAvoidPush() OVERRIDE { return &m_vAvoidPush; }                                                                                                                      // +0x5c, 00416ab0 (shared getter of +0x98)
	virtual TFLOAT                 GetAvoidRadius() OVERRIDE;                                                                                                                                             // +0x60
	virtual void                   StopMoving() OVERRIDE { m_fSpeed = 0.0f; }                                                                                                                             // +0x6c, 0057b5e0 (shared, zeroes +0x34)
	virtual void                   Update( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags ) OVERRIDE; // +0x70
	virtual void                   UpdateFacing( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                                          // +0x78
	virtual void                   MoveTo( TFLOAT a_fDeltaTime, const Toshi::TVector4* a_pPrevPosition, const Toshi::TVector4* a_pPosition ) OVERRIDE;                                                    // +0x7c
	virtual TBOOL                  TurnTowards( const Toshi::TVector4& a_rcTarget ) OVERRIDE;                                                                                                             // +0x80
	virtual TBOOL                  TurnTowardsWithin( const Toshi::TVector4& a_rcTarget, TINT a_iUnused, TFLOAT a_fToleranceSq, TFLOAT a_fMinDistanceSq ) OVERRIDE;                                       // +0x84
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                                                           // +0x88
	virtual void                   SetFacing( const Toshi::TVector4& a_rcFacing ) OVERRIDE;                                                                                                               // +0x8c
	virtual void                   SetSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                                                                  // +0x90
	virtual void                   SetDesiredSpeed( TFLOAT a_fSpeed ) OVERRIDE { m_fDesiredSpeed = a_fSpeed; }                                                                                            // +0x94, 0043ce00 (shared setter of +0x3c)
	virtual void                   SetDesiredMovementDir( const Toshi::TVector4& a_rcFacing ) OVERRIDE;                                                                                                   // +0x98
	virtual void                   Unk_0x9c( TBOOL a_bEnable ) OVERRIDE;                                                                                                                                  // +0x9c
	virtual void                   SetFrozen( TBOOL a_bFreeze ) OVERRIDE;                                                                                                                                 // +0xa0
	virtual TBOOL                  SeekAlongLine( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcTarget ) OVERRIDE;                                                                         // +0xac
	virtual TBOOL                  Seek( const Toshi::TVector4& a_rcTarget ) OVERRIDE;                                                                                                                    // +0xb0
	virtual TBOOL                  SeekFacing( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcFacing, TFLOAT a_fUnk ) OVERRIDE;                                                            // +0xb4
	virtual void                   Halt() OVERRIDE;                                                                                                                                                       // +0xb8
	virtual TBOOL                  IsSeekingConstant() OVERRIDE;                                                                                                                                          // +0xbc
	virtual TBOOL                  HasMoved() OVERRIDE;                                                                                                                                                   // +0xc0
	virtual void                   SetSeekOffset( TFLOAT a_fOffset ) OVERRIDE { m_fSeekOffsetSq = a_fOffset; }                                                                                            // +0xc4, 00502a20 (shared setter of +0xa4)
	virtual void                   ResetSeekOffset() OVERRIDE;                                                                                                                                            // +0xc8
	virtual void                   Unk_0xcc( TFLOAT a_fUnk ) OVERRIDE { m_fUnkA8 = a_fUnk; }                                                                                                              // +0xcc, 0041a3e0 (shared setter of +0xa8)
	virtual void                   SetSeekSlowdown( TBOOL a_bEnable ) OVERRIDE;                                                                                                                           // +0xd0

	// Remembers the current position and facing to walk back to
	void SaveStuckPosition();

	// TTRUE while blocked, avoiding or in the cooldown after avoiding
	TBOOL IsAvoiding();

	// Stops waiting for blocked ways
	TBOOL ResetAvoiding();

	// Whether it walks back to the position it got pushed from (stops returning when disabled)
	void SetReturnEnabled( TBOOL a_bEnabled );

	FLAGS GetFlags() const { return m_uiFlags; }
	void  SetFlags( FLAGS a_uiFlags ) { m_uiFlags = a_uiFlags; }

private:
	// Takes the next fixed step out of m_fTimeToUpdate, TFALSE once there's nothing left
	TBOOL GetNextTimeStep( TFLOAT a_fDeltaTime, TFLOAT* a_pOutStep );

	// Turns m_vFacing towards m_vDesiredFacing, returns the factor of the desired speed to move with
	TFLOAT UpdateTurning( TFLOAT a_fDeltaTime, TFLOAT a_fStep );

	// Updates the desired facing while seeking, moves straight to the target with FLAGS_SEEK_FACING
	TBOOL UpdateSeek( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags );

	// Direction to the seek target, TFALSE once the target is reached
	TBOOL GetSeekFacing( TFLOAT a_fDeltaTime, TFLOAT* a_pOutDistance, Toshi::TVector4* a_pOutFacing );

	// Bends a_pFacing away from the obstructions in front
	TBOOL AvoidObstructions( TFLOAT a_fDeltaTime, Toshi::TVector4* a_pFacing, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored );

	// TTRUE if an obstruction stands right in front in the direction of a_pFacing
	TBOOL IsBlocked( TFLOAT a_fDeltaTime, Toshi::TVector4* a_pFacing, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored );

	// Pushes the kinematics out of every obstruction that isn't ignored
	void UpdateObstructions( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored );

	// Pushes the kinematics out of a unit obstruction
	TBOOL PushOutOfUnit( TFLOAT a_fDeltaTime, AKinematicsObstruction* a_pObstruction );

	// Pushes the kinematics out of a box obstruction
	TBOOL PushOutOfBox( TFLOAT a_fDeltaTime, AKinematicsObstruction* a_pObstruction );

private:
	Toshi::TVector4 m_vPosition;        // 0x14
	Toshi::TVector4 m_vFacing;          // 0x24
	TFLOAT          m_fSpeed;           // 0x34
	TFLOAT          m_fMeasuredSpeed;   // 0x38, from the actual change of the position
	TFLOAT          m_fDesiredSpeed;    // 0x3c
	Toshi::TVector4 m_vDesiredFacing;   // 0x40
	TFLOAT          m_fTimeToUpdate;    // 0x50, integrated in steps of 1/30s
	FLAGS           m_uiFlags;          // 0x54
	Toshi::TVector4 m_vSeekTarget;      // 0x58
	Toshi::TVector4 m_vSeekStart;       // 0x68
	Toshi::TVector4 m_vSeekDirection;   // 0x78
	Toshi::TVector4 m_vSeekFacing;      // 0x88
	Toshi::TVector2 m_vAvoidPush;       // 0x98, not initialised by the ctor
	TFLOAT          m_fAvoidRadius;     // 0xa0
	TFLOAT          m_fSeekOffsetSq;    // 0xa4, distance (squared) at which the seek target is reached
	TFLOAT          m_fUnkA8;           // 0xa8
	TFLOAT          m_fTurnSpeed;       // 0xac, radians per second
	TFLOAT          m_fGroundHeight;    // 0xb0, the y of the last SetPosition/MoveTo
	Toshi::TVector4 m_vStuckPosition;   // 0xb4
	Toshi::TVector4 m_vStuckFacing;     // 0xc4
	TBOOL           m_bReturning;       // 0xd4, walking back to m_vStuckPosition
	TFLOAT          m_fStuckTimer;      // 0xd8
	TFLOAT          m_fUnkDC;           // 0xdc
	TFLOAT          m_fAvoidTimer;      // 0xe0
	TFLOAT          m_fAvoidCheckTimer; // 0xe4
};

TSTATICASSERT( sizeof( ASteerKinematics2D ) == 0xe8 );
