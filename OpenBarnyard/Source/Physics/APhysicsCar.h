#pragma once
#include "Physics/APhysicsBike.h"

//-----------------------------------------------------------------------------
// Purpose: Setup of the car itself, copied into APhysicsCar+0xDA8 by
// APhysicsCar::Setup, size 0x94.
//-----------------------------------------------------------------------------
struct APhysicsCarParams
{
	static constexpr TINT MAX_NUM_SPHERES = 4;

	TFLOAT          fUnk00;                       // 0x00 -> 0xDA8
	TFLOAT          fBrakeForce;                  // 0x04 -> 0xDAC, also the reverse force in reverse gear
	TFLOAT          fRearBrakeBias;               // 0x08 -> 0xDB0, 0..1, part of the brake force on the rear wheels
	TFLOAT          fMaxSpeed;                    // 0x0C -> 0xDB4, the down force is at its maximum at this speed
	TFLOAT          fUnk10;                       // 0x10 -> 0xDB8
	Toshi::TVector4 avSpheres[ MAX_NUM_SPHERES ]; // 0x14 -> 0xDBC, xyz: centre, w: radius
	TINT            iNumSpheres;                  // 0x54 -> 0xDFC
	TFLOAT          fFrontAntiRoll;               // 0x58 -> 0xE00, stiffness of the front anti-roll bar
	TFLOAT          fRearAntiRoll;                // 0x5C -> 0xE04, stiffness of the rear anti-roll bar
	TFLOAT          fFriction;                    // 0x60 -> 0xE08, friction of the body when it's upright
	TFLOAT          fRestitution;                 // 0x64 -> 0xE0C, restitution of the body when it's upright
	TFLOAT          fUpsideDownFriction;          // 0x68 -> 0xE10
	TFLOAT          fUpsideDownRestitution;       // 0x6C -> 0xE14
	TFLOAT          fDownForce;                   // 0x70 -> 0xE18, down force in g at fMaxSpeed
	Toshi::TVector4 vBoxExtents;                  // 0x74 -> 0xE1C, half size of the collision box of the body
	Toshi::TVector4 vBoxCentre;                   // 0x84 -> 0xE2C, centre of the collision box of the body
};

TSTATICASSERT( sizeof( APhysicsCarParams ) == 0x94 );

//-----------------------------------------------------------------------------
// Purpose: Physics of the cars (4 wheels, Joy Ride). Allocated from the car
// pool of AVehicleManager (Toshi::T2ObjectPool<APhysicsCar, 8, 3644>),
// vtable 0x0073ae54, size 0xE3C.
//-----------------------------------------------------------------------------
class APhysicsCar
    : public APhysicsVehicle
{
public:
	APhysicsCar();
	virtual ~APhysicsCar();                                   // +0x00
	virtual void  PreUpdate( TFLOAT a_fDeltaTime ) override;  // +0x04
	virtual void  PostUpdate( TFLOAT a_fDeltaTime ) override; // +0x08
	virtual TBOOL IsSliding() override;                       // +0x10, IsHandBrakeSliding
	virtual void  VTable0x1C() override;                      // +0x1C
	virtual void  UpdateSuspension() override;                // +0x20
	virtual void  FlipUpright() override;                     // +0x24, puts the car back on its wheels when it lies on its roof
	virtual void  Collide( TFLOAT a_fDeltaTime ) override;    // +0x28

	// Sets up the physics: the car, the vehicle, the engine and the wheels
	// (front left, front right, rear right, rear left)
	void Setup( const APhysicsCarParams& a_rcCarParams, const APhysicsVehicleParams& a_rcVehicleParams, const APhysicsEngineParams& a_rcEngineParams, const APhysicsWheelParams& a_rcWheel0, const APhysicsWheelParams& a_rcWheel1, const APhysicsWheelParams& a_rcWheel2, const APhysicsWheelParams& a_rcWheel3 );

	TBOOL                    IsHandBrakeSliding() const { return m_bHandBrakeSlide; }
	const APhysicsCarParams& GetParams() const { return m_oParams; }
	TFLOAT                   GetMaxSpeed() const { return m_oParams.fMaxSpeed; }

#ifdef BARNYARD_LATEST
	// Recovery point of FlipUpright: the car is put back there, facing along the direction
	// (set every frame for the player's truck of Joy Ride by ACarChaseMiniGameState::UpdateSplineFollowers)

	// $BarnyardWiiJP: FUNCTION 8042c654
	void SetRecoveryPoint( const Toshi::TVector4& a_rcPoint )
	{
		m_vRecoveryPoint    = a_rcPoint;
		m_bHasRecoveryPoint = TTRUE;
	}

	// $BarnyardWiiJP: FUNCTION 8042c680
	void SetRecoveryDirection( const Toshi::TVector4& a_rcDirection )
	{
		m_vRecoveryDirection = a_rcDirection;
		m_bHasRecoveryPoint  = TTRUE;
	}
#endif // BARNYARD_LATEST

private:
	// Steps of PreUpdate
	void UpdateStopped();    // freezes the car when it stands still
	void UpdateBrakeForce(); // brake force of the wheels
	void ApplyHandBrakeTurn();
	void ApplyDownForce();
	void UpdateMaterial(); // friction and restitution of the body depend on whether the car is upright
	void UpdateHandBrakeSlide();

	// Anti-roll bars of the front and rear axles
	void ApplyAntiRoll();

public:
	TFLOAT m_fFlipTimer;      // 0xD98, time the car has been lying on its roof
	TBOOL  m_bHandBrakeSlide; // 0xD9C
#ifdef BARNYARD_LATEST
	// NOTE: offsets of the JP Wii build are 0xE8D, 0xE90 and 0xEA0 (its APhysicsVehicle is bigger)
	TBOOL           m_bHasRecoveryPoint;      // 0xD9D
	Toshi::TVector4 m_vRecoveryPoint;         // 0xDA0
	Toshi::TVector4 m_vRecoveryDirection;     // 0xDB0
#endif                                        // BARNYARD_LATEST
	ABikePhysicsEventHandler m_oEventHandler; // 0xDA0, NOTE: same class as the one of APhysicsBike (vtable 0x0073ade4)
	APhysicsCarParams        m_oParams;       // 0xDA8
};

#ifdef BARNYARD_LATEST
TSTATICASSERT( sizeof( APhysicsCar ) == 0xE5C );
#else
TSTATICASSERT( sizeof( APhysicsCar ) == 0xE3C );
#endif
