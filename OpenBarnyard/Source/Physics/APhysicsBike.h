#pragma once
#include "Physics/APhysicsVehicle.h"

class APhysicsBike;

//-----------------------------------------------------------------------------
// Purpose: Setup of the APhysicsVehicle part of a bike (15 floats), copied
// by APhysicsBike::Setup into APhysicsVehicle+0xD54..0xD8C.
// NOTE: fUnk30 isn't copied by APhysicsBike::Setup.
//-----------------------------------------------------------------------------
struct APhysicsVehicleParams
{
	TFLOAT          fMass;                // 0x00 -> 0xD54
	Toshi::TVector4 vSize;                // 0x04 -> 0xD58, size of the box used to calculate the inertia
	TFLOAT          fGearShiftDelay;      // 0x14 -> 0xD68, time the brake has to be held at a stop before the bike reverses
	TFLOAT          fMaxSteeringAngle;    // 0x18 -> 0xD6C
	TFLOAT          fSteeringSpeedFactor; // 0x1C -> 0xD70, max angle = fMaxSteeringAngle / ( speed^2 * fSteeringSpeedFactor + 1 )
	TFLOAT          fLowSpeed;            // 0x20 -> 0xD74
	TINT            iNumWheels;           // 0x24 -> 0xD78
	TFLOAT          fRollingResistance;   // 0x28 -> 0xD7C
	TFLOAT          fSurfaceDrag;         // 0x2C -> 0xD80
	TFLOAT          fUnk30;               // 0x30, not copied
	TFLOAT          fSurfaceDrag7;        // 0x34 -> 0xD88
	TFLOAT          fSurfaceDrag0;        // 0x38 -> 0xD8C
};

TSTATICASSERT( sizeof( APhysicsVehicleParams ) == 0x3C );

//-----------------------------------------------------------------------------
// Purpose: Setup of the bike itself, copied into APhysicsBike+0xDE4, size 0x1E8.
//-----------------------------------------------------------------------------
struct APhysicsBikeParams
{
	TFLOAT          fHandBrakeTurnRate; // 0x00, yaw velocity (rad/s) the hand brake turns the bike with at full steering
	TFLOAT          fMaxSpeed;          // 0x04, the engine pushes up to this speed
	TFLOAT          fMaxBoostSpeed;     // 0x08, max speed when the throttle is above 1
	TFLOAT          fMaxReverseSpeed;   // 0x0C
	TFLOAT          fMaxWheelieSpeed;   // 0x10
	TFLOAT          fMaxLeanAngle;      // 0x14, lean of the bike in turns
	TFLOAT          fDriveForce;        // 0x18
	TFLOAT          fBoostDriveForce;   // 0x1C, drive force when the throttle is above 1
	TFLOAT          fBrakeForce;        // 0x20
	TFLOAT          fJumpImpulse;       // 0x24
	TFLOAT          fWheelieMinSpeed;   // 0x28, wheelies/stoppies need at least this speed
	TFLOAT          fWheelieFullSpeed;  // 0x2C, speed of the full wheelie/stoppie angle
	TFLOAT          fWheelieAngle;      // 0x30
	TFLOAT          fStoppieAngle;      // 0x34
	ARigidBodyShape oShape;             // 0x38, shape of the bike with its rider
	ARigidBodyShape oRiderlessShape;    // 0x110, shape of the bike after the rider has been thrown off
};

TSTATICASSERT( sizeof( APhysicsBikeParams ) == 0x1E8 );

//-----------------------------------------------------------------------------
// Purpose: Gives the contacts of the bike body to APhysicsBike (vtable 0x0073ade4).
//-----------------------------------------------------------------------------
class ABikePhysicsEventHandler
    : public ARigidBodyEventHandler
{
public:
	ABikePhysicsEventHandler();
	~ABikePhysicsEventHandler();

	virtual void OnStaticContact( ARigidBodyContact* a_pContact ) override; // +0x08
	virtual void OnBodyContact( ARigidBodyContact* a_pContact ) override;   // +0x0C

public:
	APhysicsVehicle* m_pBike; // 0x04, NOTE: also used by APhysicsCar (APhysicsCar+0xda0)
};

TSTATICASSERT( sizeof( ABikePhysicsEventHandler ) == 0x8 );

//-----------------------------------------------------------------------------
// Purpose: Physics of the bikes (2 wheels). Allocated from ABikePhysicsPool
// (Toshi::T2ObjectPool<APhysicsBike, 8, 4352, 16>), vtable 0x0073adf4, size 0xFCC.
//-----------------------------------------------------------------------------
class APhysicsBike
    : public APhysicsVehicle
{
public:
	enum THROWDIRECTION : TINT
	{
		THROWDIRECTION_NONE     = 0,
		THROWDIRECTION_FORWARD  = 1,
		THROWDIRECTION_BACKWARD = 2,
	};

public:
	APhysicsBike();
	virtual ~APhysicsBike();                                  // +0x00
	virtual void  PreUpdate( TFLOAT a_fDeltaTime ) override;  // +0x04
	virtual void  PostUpdate( TFLOAT a_fDeltaTime ) override; // +0x08
	virtual TBOOL IsDriverless() override;                    // +0x0C, IsRiderless
	virtual TBOOL IsSliding() override;                       // +0x10, IsPowerSliding
	virtual TBOOL IsWheelie() override;                       // +0x14, IsDoingWheelie
	virtual TBOOL IsStoppie() override;                       // +0x18, IsDoingStoppie
	virtual void  VTable0x1C() override;                      // +0x1C
	virtual void  FlipUpright() override;                     // +0x24, puts the bike back on its wheels when it lies on its side
	virtual void  Collide( TFLOAT a_fDeltaTime ) override;    // +0x28

	// Sets up the physics: the bike, the vehicle and the front and rear wheels
	void Setup( const APhysicsBikeParams& a_rcBikeParams, const APhysicsVehicleParams& a_rcVehicleParams, const APhysicsWheelParams& a_rcFrontWheel, const APhysicsWheelParams& a_rcRearWheel );

	// Wheelie (-1) / stoppie (1) input, clamped to -1..1
	void SetLean( TFLOAT a_fLean );

	// Forgets that the rider has been thrown off
	TBOOL ResetThrowRider();

	TBOOL IsRiderless() const { return m_bRiderless; }
	TBOOL IsPowerSliding() const { return m_bPowerSlide; }
	TBOOL IsDoingWheelie() const { return m_bWheelie; }
	TBOOL IsDoingStoppie() const { return m_bStoppie; }

private:
	// Steps of PreUpdate
	void UpdateJump();
	void UpdateGear();       // switches between drive, neutral and reverse
	void UpdateStopped();    // freezes the bike when it stands still
	void UpdateDriveForce(); // drive force of the rear wheel
	void UpdateBrakeForce(); // brake force of the wheels
	void UpdateWheelie();
	void UpdateStoppie();
	void ApplyAirControl(); // wheelie/stoppie and steering inputs rotate the bike in the air
	void ApplyHandBrakeTurn();
	void UpdateLean(); // leans the bike into the turns
	void UpdatePowerSlide();
	void ApplyPowerSlideTorque();

	// Steps of PostUpdate
	void CheckThrowRider();        // throws the rider off after hard impacts
	void UpdateRiderlessStopped(); // freezes the riderless bike once it stands still

	TFLOAT GetDriveForce();

public:
	TFLOAT                   m_fLean;            // 0xD98, wheelie/stoppie input
	TBOOL                    m_bJump;            // 0xD9C, set by the steer, jumps if a wheel is on the ground
	TBOOL                    m_bJumping;         // 0xD9D, the wheels are disabled until it's cleared by the steer
	TFLOAT                   m_fLeanAngle;       // 0xDA0, angle of the bike to the side in the last update
	TFLOAT                   m_fPitch;           // 0xDA4
	TFLOAT                   m_fPrevPitch;       // 0xDA8
	TBOOL                    m_bRiderless;       // 0xDAC
	TBOOL                    m_bForceThrowRider; // 0xDAD, set by the steer, throws the rider off on the next contact
	TBOOL                    m_bWheelie;         // 0xDAE
	TBOOL                    m_bStoppie;         // 0xDAF
	Toshi::TVector4          m_vThrowVelocity;   // 0xDB0, velocity of the rider thrown off
	Toshi::TVector4          m_vThrowDirection;  // 0xDC0
	TBOOL                    m_bThrowRider;      // 0xDD0
	THROWDIRECTION           m_eThrowDirection;  // 0xDD4
	TBOOL                    m_bPowerSlide;      // 0xDD8
	ABikePhysicsEventHandler m_oEventHandler;    // 0xDDC
	APhysicsBikeParams       m_oParams;          // 0xDE4
};

TSTATICASSERT( sizeof( APhysicsBike ) == 0xFCC );
