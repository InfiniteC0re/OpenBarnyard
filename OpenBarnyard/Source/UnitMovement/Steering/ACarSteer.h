#pragma once
#include "ASteer.h"

//-----------------------------------------------------------------------------
// Purpose: Steer of the cars that drive on the terrain without physics (the
// car is placed on the ground under its 4 wheels). ARacingCarSteer derives
// from it in C++, but registers ASteer as the parent of its TClass.
// TClass 0x00788c48 (version 3.0, runtime creatable), vtable 0x00725440, size 0x21C.
// NOTE: ARacingCarSteer replaces all of the overrides below, they only run for
// units whose steer is ACarSteer itself (created by name).
// It moves along a circle when steering (m_fTurningRadius is the turning radius),
// sweeps a sphere of radius m_fRadius along the ground and slides along what it
// hits: the terrain, other ACarSteer cars and the obstacle units
// (m_ppObstacles, never set in retail).
//-----------------------------------------------------------------------------
class ACarSteer : public ASteer
{
public:
	TDECLARE_CLASS( ACarSteer, ASteer );

	static constexpr TINT NUM_WHEELS = 4;

	// Pedal input (SetPedal, m_ePedal)
	using PEDAL = TINT;
	enum PEDAL_ : PEDAL
	{
		PEDAL_NONE      = 0, // releases the throttle and the brake (ACarSteer rolls to a stop)
		PEDAL_THROTTLE  = 1,
		PEDAL_BRAKE     = 2, // ACarSteer accelerates backwards
		PEDAL_BRAKE_ALT = 3, // same as PEDAL_BRAKE in ARacingCarSteer, ACarSteer brakes
	};

	// Steering input (SetSteering, m_eSteer)
	using STEER = TINT;
	enum STEER_ : STEER
	{
		STEER_LEFT     = 0, // the steering value is decreased by 1
		STEER_RIGHT    = 1, // the steering value is increased by 1
		STEER_ABSOLUTE = 2, // any other value: the steering value is used as is
	};

public:
	ACarSteer();
	~ACarSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;                                                                                                // +0x0c
	virtual TBOOL                  CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL                  Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL                  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL                  SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void                   SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c, ignores a_rcDirection (see the cpp)
	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;                                                                   // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;                                                                  // +0xa8
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                                                                                     // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE;                                                                 // +0xb0
	virtual void                   CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE;                                                                // +0xb4
	virtual const Toshi::TVector4* GetDirection() OVERRIDE;                                                                                                    // +0xb8
	virtual void                   CopyVelocity( Toshi::TVector4& a_rOutVelocity ) OVERRIDE;                                                                   // +0xbc
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                                                                                     // +0xc0
	virtual void                   SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE;                                                                // +0x108

	// NOTE: folded with other float getters of a field at +0x20 (0056d180)
	virtual TFLOAT GetSpeed() OVERRIDE { return m_fSpeed; } // +0xc4

	// NOTE: folded with other float getters returning 0.0f (0056ce20)
	virtual TFLOAT GetMeasuredSpeed() OVERRIDE { return 0.0f; } // +0xd4

	// NOTE: folded with other float getters of a field at +0x70 (0061d670)
	virtual TFLOAT GetRadius() OVERRIDE { return m_fRadius; } // +0x160

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Matrix of the driver's seat
	virtual void GetSeatMatrix( Toshi::TMatrix44& a_rOutMatrix ); // +0x174

	// Places the car at the position, facing the direction
	virtual void Reset( Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vDirection ); // +0x178

	// Pedal input (PEDAL_)
	virtual void SetPedal( PEDAL a_ePedal, TBOOL a_bGradual ); // +0x17c

	// Steering input (STEER_). ACarSteer turns the wheels by a_fAngle degrees (STEER_LEFT/RIGHT) or straightens them (other values).
	virtual void SetSteering( STEER a_eSteer, TFLOAT a_fSteering, TFLOAT a_fAngle ); // +0x180

	// NOTE: folded with AStationarySteer::GetGroundY (00570320)
	virtual TFLOAT GetMaxSpeed() { return m_fMaxSpeed; } // +0x184

	// Set by AUnitCarChaseCar when the engine is started, cleared by AUnitCarChaseCar::Reset
	void SetEngineStarted( TBOOL a_bValue ) { m_bEngineStarted = a_bValue; }

	// NOTE: folded with ACharacterUnitCarrable::GetCarryableHelper (0055d090)
	const Toshi::TVector4& GetBodyPosition() const { return m_vBodyPosition; }

protected:
	// Places the car on the ground facing the direction (updates the wheels and the matrix)
	void PlaceOnGround( Toshi::TVector4 a_vDirection );

	// Reads the wheel offsets from the bones wheel01-04 of the model
	void SetupWheels();

	// Adds a_fAmount to the speed (clamped to m_fMinSpeed..m_fMaxSpeed)
	void Accelerate( TFLOAT a_fAmount );

	// Sets the angle (degrees) of the front wheels, and the turning radius from it
	void SetSteeringAngle( TFLOAT a_fAngle );

	// Adds a_fAmount degrees to the angle of the front wheels
	void AddSteeringAngle( TFLOAT a_fAmount );

	// Moves the car, sliding along whatever it hits
	void UpdateMovement( TFLOAT a_fDeltaTime );

	// Sweeps the car against the other ACarSteer cars. On hit, outputs the velocities after the impact and the unit hit.
	TBOOL CollideCars( Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, Toshi::TVector4& a_rOutVelocity, Toshi::TVector4& a_rOutOtherVelocity, AUnit*& a_rpHitUnit );

	// Sweeps the car against the obstacle units (planes through their position, along their direction)
	TBOOL CollideObstacles( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal );

public:
	static AFunctionTable ms_oFunctionTable;

protected:
	TFLOAT           m_fFriction;                      // 0x18
	TFLOAT           m_fWheelHeight;                   // 0x1C, subtracted from the ground height under the wheels
	TFLOAT           m_fSpeed;                         // 0x20
	TFLOAT           m_fMaxSpeed;                      // 0x24
	TFLOAT           m_fMinSpeed;                      // 0x28
	TFLOAT           m_fAcceleration;                  // 0x2C
	TFLOAT           m_fRollingDeceleration;           // 0x30
	TFLOAT           m_fBrakeDeceleration;             // 0x34
	TFLOAT           m_fSteerSpeed;                    // 0x38
	TFLOAT           m_fSteerReturnScale;              // 0x3C
	TFLOAT           m_fSteeringAngle;                 // 0x40
	TFLOAT           m_fMaxSteeringAngle;              // 0x44
	TFLOAT           m_fUnk48;                         // 0x48
	TFLOAT           m_fSlideAngle;                    // 0x4C
	TFLOAT           m_fSkidAmount;                    // 0x50
	TFLOAT           m_fSkidMinSpeed;                  // 0x54
	TFLOAT           m_fSkidMinAngle;                  // 0x58
	TFLOAT           m_fUnk5C;                         // 0x5C
	TFLOAT           m_fCollisionTurnSpeed;            // 0x60
	TFLOAT           m_fCollisionSpeedScale;           // 0x64
	TFLOAT           m_fTurningRadius;                 // 0x68, not initialised
	TFLOAT           m_fWheelBase;                     // 0x6C
	TFLOAT           m_fRadius;                        // 0x70, the position is put this much (twice) above the ground
	TFLOAT           m_fSlopeScale;                    // 0x74
	TFLOAT           m_fGravity;                       // 0x78
	TFLOAT           m_fUnk7C;                         // 0x7C
	TFLOAT           m_fSteerAmount;                   // 0x80, degrees left to turn the wheels (SetSteering)
	TFLOAT           m_fUnk84;                         // 0x84, not initialised
	TFLOAT           m_fCollisionCooldown;             // 0x88
	TFLOAT           m_fCollisionTimer;                // 0x8C
	Toshi::TVector4  m_vPosition;                      // 0x90
	Toshi::TVector4  m_vBodyPosition;                  // 0xA0, m_vBodyOffset in the world
	Toshi::TVector4  m_vVelocity;                      // 0xB0
	Toshi::TVector4  m_vDirection;                     // 0xC0
	Toshi::TVector4  m_vMoveDirection;                 // 0xD0
	Toshi::TVector4  m_vUp;                            // 0xE0
	Toshi::TVector4  m_vRight;                         // 0xF0
	Toshi::TVector4  m_vUnk100;                        // 0x100
	Toshi::TVector4  m_vBodyOffset;                    // 0x110
	Toshi::TVector4  m_avWheelPositions[ NUM_WHEELS ]; // 0x120
	Toshi::TVector4  m_avWheelOffsets[ NUM_WHEELS ];   // 0x160
	Toshi::TVector4  m_vAxle1;                         // 0x1A0, centre of the wheels 0 and 1 (ARacingCarSteer: 2 and 3)
	Toshi::TVector4  m_vAxle2;                         // 0x1B0, centre of the wheels 2 and 3 (ARacingCarSteer: 0 and 1)
	PEDAL            m_ePedal;                         // 0x1C0
	STEER            m_eSteer;                         // 0x1C4
	TBOOL            m_bTurning;                       // 0x1C8
	AUnitProxy**     m_ppObstacles;                    // 0x1CC, not initialised, never set
	TINT             m_iNumObstacles;                  // 0x1D0
	TFLOAT           m_fObstacleRadius;                // 0x1D4
	Toshi::TMatrix44 m_oTransform;                     // 0x1D8
	TBOOL            m_bEngineStarted;                 // 0x218
	TBOOL            m_bCollided;                      // 0x219
};

TSTATICASSERT( sizeof( ACarSteer ) == 0x21C );
