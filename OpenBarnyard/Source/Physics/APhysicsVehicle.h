#pragma once
#include "Physics/ARigidBody.h"

class APhysicsVehicle;

//-----------------------------------------------------------------------------
// Purpose: Setup of a wheel (copied into APhysicsWheel+0x9c), size 0x50.
//-----------------------------------------------------------------------------
struct APhysicsWheelParams
{
	// The tyre model of the wheel (eType)
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_BIKE = 0, // APhysicsWheel::GetBikeTyreForce
		TYPE_CAR  = 1, // APhysicsWheel::GetCarTyreForce
	};

	TFLOAT          fGrip;               // 0x00, scales the lateral force
	TFLOAT          fRadius;             // 0x04
	TFLOAT          fSpringStiffness;    // 0x08
	TFLOAT          fCompressionDamping; // 0x0C
	TFLOAT          fReboundDamping;     // 0x10
	TFLOAT          fMaxExtension;       // 0x14, the wheel is in the air above it
	TFLOAT          fMaxCompression;     // 0x18, the suspension bottoms out above it
	TFLOAT          fUnk1C;              // 0x1C
	TFLOAT          fUnk20;              // 0x20
	TFLOAT          fUnk24;              // 0x24
	TFLOAT          fUnk28;              // 0x28
	Toshi::TVector4 vPosition;           // 0x2C, local position of the hub
	TBOOL           bSteerable;          // 0x3C
	TBOOL           bUnk3D;              // 0x3D, blends the grip by the speed when braking
	TFLOAT          fDownForce;          // 0x40
	TYPE            eType;               // 0x44
	TFLOAT          fBrakeGripFast;      // 0x48
	TFLOAT          fBrakeGripSlow;      // 0x4C
};

TSTATICASSERT( sizeof( APhysicsWheelParams ) == 0x50 );

//-----------------------------------------------------------------------------
// Purpose: Wheel with suspension of an APhysicsVehicle.
// 4 of them at APhysicsVehicle+0x2a8, size 0xEC.
//-----------------------------------------------------------------------------
class APhysicsWheel
{
public:
	APhysicsWheel();

	void Create( APhysicsVehicle* a_pVehicle, const APhysicsWheelParams& a_rcParams );
	void SetParams( const APhysicsWheelParams& a_rcParams );

	// Casts the ray of the suspension and updates the contact and the axes of the wheel
	void UpdateContact();

	// Suspension force along the ground normal
	Toshi::TVector4 GetSuspensionForce( struct APhysicsVehicleBottomOut* a_pBottomOut );

	// Force of the tyre on the ground
	Toshi::TVector4 GetTyreForce( TFLOAT a_fDeltaTime );

private:
	Toshi::TVector4 GetBikeTyreForce( TFLOAT a_fDeltaTime );
	Toshi::TVector4 GetCarTyreForce( TFLOAT a_fDeltaTime );
	TFLOAT          CalculateSpringForce( TFLOAT a_fLength, TFLOAT a_fMaxForce );

	// Accumulates the impulse that stops the suspension when it is fully compressed
	void AddBottomOutImpulse( struct APhysicsVehicleBottomOut* a_pBottomOut );

public:
	APhysicsVehicle*    m_pVehicle;          // 0x00
	Toshi::TVector4     m_vForward;          // 0x04, X axis of the body x ground normal (NOTE: the Y axis of the world points down)
	Toshi::TVector4     m_vSide;             // 0x14
	Toshi::TVector4     m_vGroundNormal;     // 0x24, up from the ground (VEC_NEGY is up)
	Toshi::TVector4     m_vContactPosition;  // 0x34
	Toshi::TVector4     m_vPosition;         // 0x44, world position of the hub
	Toshi::TVector4     m_vContactVelocity;  // 0x54
	TFLOAT              m_fSlipAngle;        // 0x64
	TFLOAT              m_fAngularVelocity;  // 0x68
	TFLOAT              m_fRotation;         // 0x6C
	TBOOL               m_bOnGround;         // 0x70
	TFLOAT              m_fGroundDistance;   // 0x74, UpdateContact: ( contact - hub ) . Y axis of the body, GetSuspensionForce negates it into the compression
	TFLOAT              m_fPrevSpringLength; // 0x78
	TFLOAT              m_fSpringForce;      // 0x7C
	TBOOL               m_bBottomedOut;      // 0x80
	TFLOAT              m_fLateralForce;     // 0x84
	TFLOAT              m_fDriveForce;       // 0x88, along -m_vForward
	TFLOAT              m_fEngineForce;      // 0x8C, along -m_vForward, set by APhysicsEngine2::UpdateTorque
	TFLOAT              m_fDownForceLoad;    // 0x90, share of the down force (APhysicsCar), raises the max lateral force
	TINT                m_iSurfaceType;      // 0x94
	TBOOL               m_bEnabled;          // 0x98
	APhysicsWheelParams m_oParams;           // 0x9C
};

TSTATICASSERT( sizeof( APhysicsWheel ) == 0xEC );

//-----------------------------------------------------------------------------
// Purpose: Gear state shared by the engines, APhysicsVehicle+0x658, size 0x10.
//-----------------------------------------------------------------------------
struct APhysicsGearBox
{
	TINT   iGear;         // 0x00, 0: reverse, 1: neutral, 2+: forward gears
	TINT   iPrevGear;     // 0x04
	TFLOAT fGearTimer;    // 0x08, time since the last gear change (APhysicsBike)
	TBOOL  bAllowReverse; // 0x0C
	TBOOL  bAllowDrive;   // 0x0D
};

TSTATICASSERT( sizeof( APhysicsGearBox ) == 0x10 );

//-----------------------------------------------------------------------------
// Purpose: Setup of an engine, size 0x58.
//-----------------------------------------------------------------------------
struct APhysicsEngineParams
{
	static constexpr TINT MAX_NUM_GEARS = 7;

	TFLOAT fPeakTorque;                   // 0x00
	TFLOAT fPeakTorqueRPM;                // 0x04
	TFLOAT fIdleTorque;                   // 0x08
	TFLOAT fMaxRPMTorque;                 // 0x0C
	TFLOAT fMaxRPM;                       // 0x10
	TFLOAT fIdleRPM;                      // 0x14
	TFLOAT fUnk18;                        // 0x18, inverted at +0x64/+0x40 of the engines
	TFLOAT fUnk1C;                        // 0x1C
	TFLOAT fEngineBraking;                // 0x20
	TFLOAT afGearRatios[ MAX_NUM_GEARS ]; // 0x24
	TFLOAT fFinalDrive;                   // 0x40
	TFLOAT fNumGears;                     // 0x44
	TFLOAT fShiftDelay;                   // 0x48
	TFLOAT fClutchTime;                   // 0x4C
	TFLOAT fShiftTime;                    // 0x50
	TFLOAT fTorqueScale;                  // 0x54
};

TSTATICASSERT( sizeof( APhysicsEngineParams ) == 0x58 );

//-----------------------------------------------------------------------------
// Purpose: Simple engine (APhysicsVehicle+0x668), vtable 0x0073af74, size 0x7C.
//-----------------------------------------------------------------------------
class APhysicsEngine
{
public:
	APhysicsEngine();
	virtual ~APhysicsEngine() = default; // +0x00

	void Create( APhysicsVehicle* a_pVehicle, APhysicsGearBox* a_pGearBox, const APhysicsEngineParams& a_rcParams );

public:
	TFLOAT           m_fRPM;                                                // 0x04, rad/s
	TFLOAT           m_fUnk08;                                              // 0x08
	TINT             m_iUnk0C;                                              // 0x0C
	TFLOAT           m_fUnk10;                                              // 0x10
	APhysicsVehicle* m_pVehicle;                                            // 0x14
	APhysicsGearBox* m_pGearBox;                                            // 0x18
	APhysicsWheel*   m_pWheel1;                                             // 0x1C
	APhysicsWheel*   m_pWheel2;                                             // 0x20
	TFLOAT           m_fPeakTorque;                                         // 0x24
	TFLOAT           m_fPeakTorqueRPM;                                      // 0x28
	TFLOAT           m_fIdleTorque;                                         // 0x2C
	TFLOAT           m_fMaxRPMTorque;                                       // 0x30
	TFLOAT           m_fMaxRPM;                                             // 0x34
	TFLOAT           m_fIdleRPM;                                            // 0x38
	TFLOAT           m_fUnk3C;                                              // 0x3C
	TFLOAT           m_fInvUnk3C;                                           // 0x40
	TFLOAT           m_fEngineBraking;                                      // 0x44
	TFLOAT           m_afGearRatios[ APhysicsEngineParams::MAX_NUM_GEARS ]; // 0x48
	TFLOAT           m_fFinalDrive;                                         // 0x64
	TFLOAT           m_fNumGears;                                           // 0x68
	TFLOAT           m_fShiftDelay;                                         // 0x6C
	TFLOAT           m_fClutchTime;                                         // 0x70
	TFLOAT           m_fShiftTime;                                          // 0x74
	TFLOAT           m_fUnk78;                                              // 0x78
};

TSTATICASSERT( sizeof( APhysicsEngine ) == 0x7C );

//-----------------------------------------------------------------------------
// Purpose: Engine with gear box and clutch (APhysicsVehicle+0x6e4), vtable
// 0x0073af78, size 0xA0.
//-----------------------------------------------------------------------------
class APhysicsEngine2
{
public:
	APhysicsEngine2();
	virtual ~APhysicsEngine2() = default; // +0x00

	void Create( APhysicsVehicle* a_pVehicle, APhysicsGearBox* a_pGearBox, const APhysicsEngineParams& a_rcParams );

	// Updates the RPM
	void UpdateRPM( TFLOAT a_fDeltaTime );

	// Shifts the gears
	void UpdateGears( TFLOAT a_fDeltaTime );

	// Updates m_fTorque and the drive torque of the wheels
	void UpdateTorque( TFLOAT a_fDeltaTime );

	// Torque of the engine at the current RPM
	TFLOAT GetTorque() const;

private:
	void OnGearChanged();

public:
	TFLOAT           m_fRPM;                                                // 0x04, rad/s
	TFLOAT           m_fPrevRPM;                                            // 0x08
	TINT             m_iUnk0C;                                              // 0x0C, nothing is updated while it is zero
	TFLOAT           m_fThrottle;                                           // 0x10
	TFLOAT           m_fRevLimiterTime;                                     // 0x14
	TFLOAT           m_fTorque;                                             // 0x18
	TFLOAT           m_fClutch;                                             // 0x1C
	TFLOAT           m_fGearRatio;                                          // 0x20
	TFLOAT           m_fEfficiency;                                         // 0x24
	TFLOAT           m_fInertia;                                            // 0x28
	TFLOAT           m_fShiftTimer;                                         // 0x2C
	TFLOAT           m_fReverseTimer;                                       // 0x30, braking in neutral shifts into reverse after m_fShiftDelay
	TBOOL            m_bShiftingUp;                                         // 0x34
	TBOOL            m_bShiftingDown;                                       // 0x35
	APhysicsVehicle* m_pVehicle;                                            // 0x38
	APhysicsGearBox* m_pGearBox;                                            // 0x3C
	APhysicsWheel*   m_pWheel1;                                             // 0x40
	APhysicsWheel*   m_pWheel2;                                             // 0x44
	TFLOAT           m_fPeakTorque;                                         // 0x48
	TFLOAT           m_fPeakTorqueRPM;                                      // 0x4C
	TFLOAT           m_fIdleTorque;                                         // 0x50
	TFLOAT           m_fMaxRPMTorque;                                       // 0x54
	TFLOAT           m_fMaxRPM;                                             // 0x58
	TFLOAT           m_fIdleRPM;                                            // 0x5C
	TFLOAT           m_fUnk60;                                              // 0x60
	TFLOAT           m_fInvUnk60;                                           // 0x64
	TFLOAT           m_fEngineBraking;                                      // 0x68
	TFLOAT           m_afGearRatios[ APhysicsEngineParams::MAX_NUM_GEARS ]; // 0x6C
	TFLOAT           m_fFinalDrive;                                         // 0x88
	TFLOAT           m_fNumGears;                                           // 0x8C
	TFLOAT           m_fShiftDelay;                                         // 0x90
	TFLOAT           m_fClutchTime;                                         // 0x94
	TFLOAT           m_fShiftTime;                                          // 0x98
	TFLOAT           m_fTorqueScale;                                        // 0x9C
};

TSTATICASSERT( sizeof( APhysicsEngine2 ) == 0xA0 );

//-----------------------------------------------------------------------------
// Purpose: Impulses of the wheels whose suspension is fully compressed,
// APhysicsVehicle+0x784, size 0x28.
//-----------------------------------------------------------------------------
struct APhysicsVehicleBottomOut
{
	APhysicsVehicleBottomOut();
	void Reset();

	TINT            iCount;    // 0x00
	TFLOAT          fMaxDepth; // 0x04
	Toshi::TVector4 vImpulse;  // 0x08
	Toshi::TVector4 vTorque;   // 0x18
};

TSTATICASSERT( sizeof( APhysicsVehicleBottomOut ) == 0x28 );

//-----------------------------------------------------------------------------
// Purpose: A collision of the vehicle reported to the game (filled by the
// ARigidBodyEventHandler of the vehicle), size 0x40.
//-----------------------------------------------------------------------------
struct APhysicsVehicleContact
{
	APhysicsVehicleContact();
	void Reset();

	Toshi::TVector4   vNormal;           // 0x00
	Toshi::TVector4   vPosition;         // 0x10
	Toshi::TVector4   vRelativeVelocity; // 0x20
	TFLOAT            fImpulse;          // 0x30
	TINT              iSurfaceType;      // 0x34
	ARigidBody*       pOtherBody;        // 0x38
	ACollisionObject* pCollisionObject;  // 0x3C
};

TSTATICASSERT( sizeof( APhysicsVehicleContact ) == 0x40 );

//-----------------------------------------------------------------------------
// Purpose: Base of the physics of the vehicles (APhysicsBike, APhysicsCar).
// vtable 0x0073af80, size 0xD98.
//-----------------------------------------------------------------------------
class APhysicsVehicle
{
public:
	static constexpr TINT MAX_NUM_WHEELS   = 4;
	static constexpr TINT MAX_NUM_CONTACTS = 20;

public:
	APhysicsVehicle();
	virtual ~APhysicsVehicle();                     // +0x00
	virtual void PreUpdate( TFLOAT a_fDeltaTime );  // +0x04
	virtual void PostUpdate( TFLOAT a_fDeltaTime ); // +0x08

	// RETURNS_FALSE (00660960)
	virtual TBOOL IsDriverless() { return TFALSE; } // +0x0C
	// RETURNS_FALSE (00660960)
	virtual TBOOL IsSliding() { return TFALSE; } // +0x10
	// RETURNS_FALSE (00660960)
	virtual TBOOL IsWheelie() { return TFALSE; } // +0x14
	// RETURNS_FALSE (00660960)
	virtual TBOOL IsStoppie() { return TFALSE; } // +0x18
	// DOES_NOTHING (005acd80)
	virtual void VTable0x1C() {} // +0x1C

	virtual void UpdateSuspension();                 // +0x20
	virtual void FlipUpright()                  = 0; // +0x24, APhysicsBike: puts the bike back on its wheels
	virtual void Collide( TFLOAT a_fDeltaTime ) = 0; // +0x28

	// Teleports the vehicle and stops it
	void Reset( const Toshi::TVector4& a_rcPosition, const Toshi::TQuaternion& a_rcOrientation );

	// Stops the vehicle where it is
	void Stop();

	// Freezes the vehicle (m_bFrozen) and stops it if it wasn't frozen
	Toshi::TVector4* Freeze();

	void SetFrozen( TBOOL a_bFrozen );

	// Inputs
	void SetSteering( TFLOAT a_fSteering );   // -1..1, with dead zone
	void SetThrottle( TFLOAT a_fThrottle );   // 0..m_fMaxThrottle
	void SetBrake( TFLOAT a_fBrake );         // 0..1
	void SetHandBrake( TFLOAT a_fHandBrake ); // 0..1

	// Returns contact a_uiIndex reported in the last update or TNULL
	APhysicsVehicleContact* GetContact( TUINT a_uiIndex );

	// Adds a contact (TNULL if there are already 20)
	APhysicsVehicleContact* AddContact();

	// Updates the contacts of the wheels and m_iNumWheelsOnGround
	void UpdateWheelContacts();

	// Applies the suspension and tyre forces of the wheels
	void ApplyWheelForces();

	void UseCentreOfMass1();
	void UseCentreOfMass2();

	ARigidBody& GetBody() { return m_oBody; }

protected:
	// Forces used by the APhysicsVehicle subclasses in their PreUpdate
	void StopAirRoll();             // stops the rotation around the Z axis when the vehicle leaves the ground
	void StopAirPitch();            // stops the rotation around the X axis when the vehicle leaves the ground
	void ApplyAirPitchUpright();    // turns the vehicle upright in the air (disabled)
	void ApplyGroundPitchDamping(); // damps the rotation around the X axis on the ground
	void UpdateSteeringAngle();
	void ApplyRollingResistance();
	void ApplyGravity();
	void ApplySurfaceDrag();

	// Pushes the vehicle out of the ground with the impulses of the bottomed out wheels
	void ApplyBottomOut( APhysicsVehicleBottomOut* a_pBottomOut );

public:
	ARigidBody               m_oBody;                         // 0x004
	APhysicsWheel            m_aWheels[ MAX_NUM_WHEELS ];     // 0x2A8
	APhysicsGearBox          m_oGearBox;                      // 0x658
	APhysicsEngine           m_oEngine;                       // 0x668
	APhysicsEngine2          m_oEngine2;                      // 0x6E4
	APhysicsVehicleBottomOut m_oBottomOut;                    // 0x784
	APhysicsVehicleContact   m_aContacts[ MAX_NUM_CONTACTS ]; // 0x7AC
	TINT                     m_iNumContacts;                  // 0xCAC
	TFLOAT                   m_fSteering;                     // 0xCB0
	TFLOAT                   m_fThrottle;                     // 0xCB4
	TFLOAT                   m_fBrake;                        // 0xCB8
	TFLOAT                   m_fHandBrake;                    // 0xCBC
	TFLOAT                   m_fSteeringAngle;                // 0xCC0
	TFLOAT                   m_fDeltaTime;                    // 0xCC4
	TBOOL                    m_bUnkCC8;                       // 0xCC8
	TFLOAT                   m_fStopTimer;                    // 0xCCC, the vehicle freezes once it stood still for a second
	TBOOL                    m_bFrozen;                       // 0xCD0
	TBOOL                    m_bFlipRecovery;                 // 0xCD1, FlipUpright turns the vehicle back upright
	TBOOL                    m_bUnkCD2;                       // 0xCD2, wheel grip depends on the gear and the brake
	TINT                     m_iNumWheelsOnGround;            // 0xCD4
	TBOOL                    m_bUnkCD8;                       // 0xCD8
	TBOOL                    m_bAirRollStopped;               // 0xCD9
	TBOOL                    m_bAirPitchStopped;              // 0xCDA
	Toshi::TVector4          m_vLocalVelocity;                // 0xCDC
	Toshi::TVector4          m_vPrevLocalVelocity;            // 0xCEC
	Toshi::TVector4          m_vPrevPosition;                 // 0xCFC
	Toshi::TQuaternion       m_qPrevOrientation;              // 0xD0C
	Toshi::TVector4          m_vPrevVelocity;                 // 0xD1C
	Toshi::TVector4          m_vCentreOfMass1;                // 0xD2C
	Toshi::TVector4          m_vCentreOfMass2;                // 0xD3C
	TBOOL                    m_bEnabled;                      // 0xD4C
	TINT                     m_iUnkD50;                       // 0xD50
	TFLOAT                   m_fMass;                         // 0xD54
	Toshi::TVector4          m_vSize;                         // 0xD58, size of the box of the body
	TFLOAT                   m_fGearShiftDelay;               // 0xD68
	TFLOAT                   m_fMaxSteeringAngle;             // 0xD6C
	TFLOAT                   m_fSteeringSpeedFactor;          // 0xD70, max angle = m_fMaxSteeringAngle / ( speed^2 * this + 1 )
	TFLOAT                   m_fLowSpeed;                     // 0xD74, below it the lateral tyre force stops the sliding
	TINT                     m_iNumWheels;                    // 0xD78
	TFLOAT                   m_fRollingResistance;            // 0xD7C
	TFLOAT                   m_fSurfaceDrag;                  // 0xD80, default
	TFLOAT                   m_fSurfaceDrag1;                 // 0xD84, surface type 1
	TFLOAT                   m_fSurfaceDrag7;                 // 0xD88, surface type 7
	TFLOAT                   m_fSurfaceDrag0;                 // 0xD8C, surface type 0
	TBOOL                    m_bUseTorqueScale;               // 0xD90, scales the engine torque by m_oEngine2.m_fTorqueScale
	TFLOAT                   m_fMaxThrottle;                  // 0xD94
};

TSTATICASSERT( sizeof( APhysicsVehicle ) == 0xD98 );
