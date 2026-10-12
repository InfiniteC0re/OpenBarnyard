#pragma once
#include "ASteerKinematics.h"

//-----------------------------------------------------------------------------
// Purpose: Kinematics of the player steers (APlayerSteer). Moves on the ground
// plane with a turning speed that accelerates towards the desired facing,
// integrates the movement with fixed steps of 1/30s, handles the vertical speed
// (gravity is +y in Barnyard, jumping sets a negative vertical speed), seeking
// a target along a line and pushes itself out of kinematics obstructions.
// Also keeps the tilt of the model (pitch, roll) that APlayerSteer sets while
// jumping and falling.
// NOTE: vtable 0x00727948, TClass 0x007896b8 (version 1.0, size 0xc0).
//-----------------------------------------------------------------------------
class ASteerKinematics3D : public ASteerKinematics
{
public:
	TDECLARE_CLASS( ASteerKinematics3D, ASteerKinematics );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_UNK0          = BITFLAG( 0 ), // Unk_0x9c (maybe JPOG's SetLockToGround: interior units set it, the player clears it), set by Create/CreateFrom, never tested
		FLAGS_FROZEN        = BITFLAG( 1 ), // SetFrozen, Update does nothing
		FLAGS_SEEKING       = BITFLAG( 2 ), // following the line from m_vSeekStart to m_vSeekTarget
		FLAGS_SEEK_FACING   = BITFLAG( 4 ), // stops at m_vSeekTarget facing m_vSeekFacing
		FLAGS_MOVED         = BITFLAG( 5 ), // the position changed during the last Update
		FLAGS_SEEK_SLOWDOWN = BITFLAG( 6 ), // SetSeekSlowdown, slows down close to the seek target
		FLAGS_HEAVY_CARRY   = BITFLAG( 7 ), // SetCarryingHeavy, ASteerTraits::m_fHeavyCarrySpeedFactor applies
		FLAGS_NO_TILT       = BITFLAG( 8 ), // set by APlayerSteer while climbing ladders
		FLAGS_OBSTRUCTION   = BITFLAG( 9 ), // APlayerSteer::DisableKinematicsObstructionStop, see UpdateObstruction
	};

public:
	ASteerKinematics3D();
	~ASteerKinematics3D();

	//-----------------------------------------------------------------------------
	// ASteerKinematics
	//-----------------------------------------------------------------------------
	virtual TBOOL                  Create( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, const ASteerTraits* a_pTraits, TFLOAT a_fSpeed ) OVERRIDE;                          // +0x0c
	virtual TBOOL                  CreateFrom( ASteer* a_pOther ) OVERRIDE;                                                                                                                               // +0x10
	virtual TFLOAT                 GetMeasuredSpeedRatio() OVERRIDE;                                                                                                                                      // +0x14
	virtual TFLOAT                 GetMeasuredSpeed() OVERRIDE;                                                                                                                                           // +0x18
	virtual void                   SetMeasuredSpeed( TFLOAT a_fSpeed ) OVERRIDE { m_fMeasuredSpeed = a_fSpeed; }                                                                                          // +0x1c, 0043cdf0 (shared setter of +0x44)
	virtual void                   BlendMeasuredSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                                                        // +0x20
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                                                                                                                                // +0x24
	virtual Toshi::TVector4        Unk_0x28() OVERRIDE;                                                                                                                                                   // +0x28
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                                                                                                                                // +0x2c
	virtual const Toshi::TVector4* GetFacingDir() OVERRIDE;                                                                                                                                               // +0x30
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                                                                                                                                   // +0x38
	virtual TFLOAT                 GetDesiredSpeed() OVERRIDE;                                                                                                                                            // +0x3c
	virtual TFLOAT                 GetTurnSpeed() OVERRIDE;                                                                                                                                               // +0x40
	virtual const Toshi::TVector4* GetSeekTarget() OVERRIDE;                                                                                                                                              // +0x44
	virtual TBOOL                  IsFacingAtDesiredFacing( TFLOAT a_fTolerance ) OVERRIDE;                                                                                                               // +0x48
	virtual void                   SetDesiredFacing( const Toshi::TVector4& a_rcFacing ) OVERRIDE;                                                                                                        // +0x4c
	virtual const Toshi::TVector4* GetDesiredFacing() OVERRIDE;                                                                                                                                           // +0x50
	virtual Toshi::TVector4&       PredictPosition( TINT a_iTime, Toshi::TVector4& a_rOut ) OVERRIDE;                                                                                                     // +0x58
	virtual void                   StopMoving() OVERRIDE;                                                                                                                                                 // +0x6c
	virtual void                   Update( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags ) OVERRIDE; // +0x70
	virtual void                   ApplyGravity( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                                          // +0x74
	virtual void                   UpdateFacing( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                                          // +0x78
	virtual void                   MoveTo( TFLOAT a_fDeltaTime, const Toshi::TVector4* a_pPrevPosition, const Toshi::TVector4* a_pPosition ) OVERRIDE;                                                    // +0x7c
	virtual TBOOL                  TurnTowards( const Toshi::TVector4& a_rcTarget ) OVERRIDE;                                                                                                             // +0x80
	virtual TBOOL                  TurnTowardsWithin( const Toshi::TVector4& a_rcTarget, TINT a_iUnused, TFLOAT a_fToleranceSq, TFLOAT a_fMinDistanceSq ) OVERRIDE;                                       // +0x84
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                                                           // +0x88
	virtual void                   SetFacing( const Toshi::TVector4& a_rcFacing ) OVERRIDE;                                                                                                               // +0x8c
	virtual void                   SetSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                                                                  // +0x90
	virtual void                   SetDesiredSpeed( TFLOAT a_fSpeed ) OVERRIDE;                                                                                                                           // +0x94
	virtual void                   SetDesiredMovementDir( const Toshi::TVector4& a_rcFacing ) OVERRIDE;                                                                                                   // +0x98
	virtual void                   Unk_0x9c( TBOOL a_bEnable ) OVERRIDE;                                                                                                                                  // +0x9c
	virtual void                   SetFrozen( TBOOL a_bFreeze ) OVERRIDE;                                                                                                                                 // +0xa0
	virtual TBOOL                  SeekAlongLine( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcTarget ) OVERRIDE;                                                                         // +0xac
	virtual TBOOL                  Seek( const Toshi::TVector4& a_rcTarget ) OVERRIDE;                                                                                                                    // +0xb0
	virtual TBOOL                  SeekFacing( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcFacing, TFLOAT a_fUnk ) OVERRIDE;                                                            // +0xb4
	virtual void                   Halt() OVERRIDE;                                                                                                                                                       // +0xb8
	virtual TBOOL                  IsSeekingConstant() OVERRIDE;                                                                                                                                          // +0xbc
	virtual TBOOL                  HasMoved() OVERRIDE;                                                                                                                                                   // +0xc0
	virtual void                   SetSeekOffset( TFLOAT a_fOffset ) OVERRIDE { m_fSeekOffsetSq = a_fOffset; }                                                                                            // +0xc4, 0063e530 (shared setter of +0x60)
	virtual void                   ResetSeekOffset() OVERRIDE;                                                                                                                                            // +0xc8
	virtual void                   Unk_0xcc( TFLOAT a_fUnk ) OVERRIDE { m_fUnk64 = a_fUnk; }                                                                                                              // +0xcc, 0063e540 (shared setter of +0x64)
	virtual void                   SetSeekSlowdown( TBOOL a_bEnable ) OVERRIDE;                                                                                                                           // +0xd0
	virtual TFLOAT                 GetAirTime() OVERRIDE;                                                                                                                                                 // +0xd4
	virtual void                   SetAirTime( TFLOAT a_fTime ) OVERRIDE { m_fAirTime = a_fTime; }                                                                                                        // +0xd8, 0063e560 (shared setter of +0x6c)
	virtual void                   ResetAirTime() OVERRIDE;                                                                                                                                               // +0xdc
	virtual void                   SetVerticalSpeed( TFLOAT a_fSpeed ) OVERRIDE { m_fVerticalSpeed = a_fSpeed; }                                                                                          // +0xe0, 0066b140 (shared setter of +0x28)
	virtual TFLOAT                 GetVerticalSpeed() OVERRIDE;                                                                                                                                           // +0xe4
	virtual TBOOL                  GetTilt( TFLOAT* a_pPitch, TFLOAT* a_pRoll ) OVERRIDE;                                                                                                                 // +0xec

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	// While carrying something heavy the speed is scaled by ASteerTraits::m_fHeavyCarrySpeedFactor
	virtual void SetCarryingHeavy( TBOOL a_bCarrying ); // +0xf0

	// Scales the gravity, a_fUnused is ignored
	void SetGravityScale( TFLOAT a_fScale, TFLOAT a_fUnused );

	// Sets the tilt the model blends to, a_bImmediate also sets the current tilt
	void SetTilt( TFLOAT a_fPitch, TFLOAT a_fRoll, TBOOL a_bImmediate );

	void SetClimbing( TBOOL a_bClimbing )
	{
		if ( a_bClimbing )
			m_uiFlags |= FLAGS_NO_TILT;
		else
			m_uiFlags &= ~FLAGS_NO_TILT;
	}

	void SetObstructionStop( TBOOL a_bEnable )
	{
		if ( a_bEnable )
			m_uiFlags |= FLAGS_OBSTRUCTION;
		else
			m_uiFlags &= ~FLAGS_OBSTRUCTION;
	}

	FLAGS GetFlags() const { return m_uiFlags; }

	// Sets the current pitch (the tilt still blends to the target pitch)
	void SetPitch( TFLOAT a_fPitch ) { m_fPitch = a_fPitch; }

private:
	// Takes the next fixed step out of m_fTimeToUpdate, TFALSE once there's nothing left
	TBOOL GetNextTimeStep( TFLOAT a_fDeltaTime, TFLOAT* a_pOutStep );

	// Turns m_vFacing towards m_vDesiredFacing, returns the factor of the desired speed to move with
	TFLOAT UpdateTurning( TFLOAT a_fDeltaTime, TFLOAT a_fStep );

	// Blends the tilt towards the target tilt
	void UpdateTilt( TFLOAT a_fDeltaTime );

	// Updates the desired facing while seeking, moves straight to the target with FLAGS_SEEK_FACING
	TBOOL UpdateSeek( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags );

	// Direction to the seek target, TFALSE once the target is reached
	TBOOL GetSeekFacing( TFLOAT a_fDeltaTime, TFLOAT* a_pOutDistance, Toshi::TVector4* a_pOutFacing );

	// Moves back when something is in front (FLAGS_OBSTRUCTION)
	void UpdateObstruction( TFLOAT a_fDeltaTime );

	// Pushes the kinematics out of every obstruction that isn't ignored
	void UpdateObstructions( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored );

	// Pushes the kinematics out of a_pObstruction
	void PushOutOfObstruction( TFLOAT a_fDeltaTime, AKinematicsObstruction* a_pObstruction );

private:
	FLAGS           m_uiFlags;        // 0x14
	Toshi::TVector4 m_vPosition;      // 0x18
	TFLOAT          m_fVerticalSpeed; // 0x28, +y is down
	TFLOAT          m_fGravityScale;  // 0x2c
	Toshi::TVector4 m_vFacing;        // 0x30
	TFLOAT          m_fSpeed;         // 0x40
	TFLOAT          m_fMeasuredSpeed; // 0x44, from the actual change of the position
	TFLOAT          m_fDesiredSpeed;  // 0x48
	Toshi::TVector4 m_vDesiredFacing; // 0x4c
	TFLOAT          m_fTimeToUpdate;  // 0x5c, integrated in steps of 1/30s
	TFLOAT          m_fSeekOffsetSq;  // 0x60, distance (squared) at which the seek target is reached
	TFLOAT          m_fUnk64;         // 0x64
	TFLOAT          m_fTurnSpeed;     // 0x68, radians per second
	TFLOAT          m_fAirTime;       // 0x6c
	Toshi::TVector4 m_vSeekTarget;    // 0x70
	Toshi::TVector4 m_vSeekStart;     // 0x80
	Toshi::TVector4 m_vSeekDirection; // 0x90
	Toshi::TVector4 m_vSeekFacing;    // 0xa0
	TFLOAT          m_fPitch;         // 0xb0
	TFLOAT          m_fRoll;          // 0xb4
	TFLOAT          m_fTargetPitch;   // 0xb8
	TFLOAT          m_fTargetRoll;    // 0xbc
};

TSTATICASSERT( sizeof( ASteerKinematics3D ) == 0xc0 );
