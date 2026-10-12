#pragma once
#include "AAnimatedUnit.h"
#include "Render/AGlowViewport.h"
#include "Sound/ASound.h"

#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class AParticleEffectHandle;
class ARacingCarSteer;
struct APhysicsVehicleContact;

//-----------------------------------------------------------------------------
// A car of the Joy Ride minigame (ACarChaseMiniGameState): the player's car,
// the cop car and the cars of the other drivers. The car is driven by its
// steer (ARacingCarSteer, which owns the APhysicsCar); the unit draws the four
// wheels, the drivers sitting in the car, the lights, the tyre tracks and
// plays the engine, skid and crash sounds.
// TClass "AUnitCarChaseCar" (parent AAnimatedUnit, size 0x250), vtables
// 00721558, 00721548 (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AUnitCarChaseCar
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitCarChaseCar, AAnimatedUnit );

	// Passed in AUnitTransform::m_iModelIndex when the car is created
	enum CARTYPE_ : TINT
	{
		CARTYPE_PLAYER, // 0, "carwheel_truck" wheels, the player as the driver
		CARTYPE_COP,    // 1, "carwheel_cop" wheels, sirens
		CARTYPE_BEADY,  // 2, "carwheel_beady" wheels, CARCHASE_B_* engine sounds
	};

	// m_eEngineState
	enum ENGINESTATE_ : TINT
	{
		ENGINESTATE_OFF,      // 0
		ENGINESTATE_STARTING, // 1, ignition cue playing (StartEngine)
		ENGINESTATE_IDLE,     // 2
		ENGINESTATE_REVVING,  // 3, while accelerating or braking
		ENGINESTATE_RUNNING,  // 4
	};

	// m_iState, set by ACarChaseMiniGameState
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_NONE     = 0,
		STATE_FINISHED = 1, // reached the end of the track (or the kids got home)
		STATE_RACING   = 3, // the sounds are updated, the input is handled
		STATE_FAILED   = 4, // too far away or spotted
	};

	// m_iDriveState, the pedal command of the input (see ACarChaseMiniGameState::DriveCar)
	using DRIVESTATE = TINT;
	enum DRIVESTATE_ : DRIVESTATE
	{
		DRIVESTATE_IDLE       = 0,
		DRIVESTATE_ACCELERATE = 1, // ARacingCarSteer::PEDAL_THROTTLE
		DRIVESTATE_BRAKE      = 2, // ARacingCarSteer::PEDAL_BRAKE
		DRIVESTATE_UNK7       = 7, // the shoot command, ARacingCarSteer::PEDAL_BRAKE_ALT
	};

	static constexpr TINT NUM_WHEELS = 4;

public:
	AUnitCarChaseCar();
	~AUnitCarChaseCar();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  OnDestroy() override;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	//-----------------------------------------------------------------------------
	// AVisibleUnit
	//-----------------------------------------------------------------------------
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x90

	//-----------------------------------------------------------------------------
	// Used by ACarChaseMiniGameState
	//-----------------------------------------------------------------------------

	// Plays the ignition and the idle cues and turns the engine of the steer on (only when the engine is off)
	void StartEngine();

	// Fades the siren cue in (a_bOn) or out
	void SetSirenOn( TBOOL a_bOn );

	// Creates the drivers sitting in the car. a_iVariant picks the models for CARTYPE_BEADY
	// (0-3: "jr_jerseycar", 4/5/7: the player + "jr_otiscar" + "jr_jerseycar", 6: the player, other: "car_igg").
	void CreateDrivers( TINT a_iVariant );

	// Stops the engine, crash, suspension and gravel cues
	void StopSounds();

	// Pauses or resumes the cues stopped by StopSounds
	void PauseSounds( TBOOL a_bPause );

	// Resets the car before a race: stops every cue, turns the engine off and clears the tyre tracks
	void Reset();

	ARacingCarSteer* GetRacingCarSteer();
	CARTYPE_         GetCarType() const { return m_eCarType; }

	// $Barnyard: FUNCTION 004f5ce0
	TINT GetUnkB0() const { return m_iUnkB0; }

	// $Barnyard: FUNCTION 004f5cf0
	void SetFinishPosition( TINT a_iValue ) { m_iFinishPosition = a_iValue; }

	// $Barnyard: FUNCTION 004f5d00
	TINT GetFinishPosition() const { return m_iFinishPosition; }

private:
	// Kills the light and siren effects
	void KillLights();

	// Plays the light and siren effects and adds the shadow
	void CreateLights();

	// Tyre tracks, dust and grass effects of the wheels
	void UpdateWheelEffects( TFLOAT a_fDeltaTime );

	// Plays the sparks effect at a_pContact
	void PlaySparks( APhysicsVehicleContact* a_pContact );

	// Suspension and crash sounds, sparks and the positions of the engine cues
	void UpdateCollisionSounds( TFLOAT a_fDeltaTime );

	// The gear change sound
	void UpdateGearSound();

	// The engine sounds
	void UpdateEngineSounds( TFLOAT a_fDeltaTime );

	// The skid sound
	void UpdateSkidSound( TFLOAT a_fDeltaTime );

	// Transform of the a_iWheel'th wheel model
	void GetWheelTransform( TINT a_iWheel, Toshi::TMatrix44& a_rOutTransform );

public:
	TINT  m_iUnk98; // 0x98
	TINT  m_iUnk9C; // 0x9c
	TINT  m_iUnkA0; // 0xa0
	TINT  m_iUnkA4; // 0xa4
	TINT  m_iUnkA8; // 0xa8, 5 after Reset
	TINT  m_iUnkAC; // 0xac, 5 after Reset
	TINT  m_iUnkB0; // 0xb0
	STATE m_iState; // 0xb4

	TINT   m_iEngineCue;     // 0xb8, played when the steer isn't an ARacingCarSteer
	TINT   m_iEngineLoopCue; // 0xbc
	TINT   m_iUnkC0Cue;      // 0xc0
	TINT   m_iIdleCue;       // 0xc4
	TINT   m_iSuspensionCue; // 0xc8
	TINT   m_iCrashCue;      // 0xcc
	TINT   m_iUnkD0Cue;      // 0xd0
	TINT   m_iSirenCue;      // 0xd4
	TINT   m_iSkidCue;       // 0xd8
	TFLOAT m_fEnginePitch;   // 0xdc, of m_iEngineCue

	AAnimatableObject* m_apWheels[ NUM_WHEELS ];     // 0xe0
	AAnimatableObject* m_pDriver;                    // 0xf0
	AAnimatableObject* m_pPassenger1;                // 0xf4
	AAnimatableObject* m_pPassenger2;                // 0xf8
	TINT               m_aiWheelBones[ NUM_WHEELS ]; // 0xfc, "wheel03", "wheel04", "wheel01", "wheel02"
	TINT               m_iBaseBone;                  // 0x10c, "base_bone"
	CARTYPE_           m_eCarType;                   // 0x110

	TINT                   m_iHeadlightEffect;     // 0x114, "car_headlight"
	TINT                   m_iTaillightEffect;     // 0x118, "car_taillight"
	TINT                   m_iSirenEffect;         // 0x11c, "car_copsiren"
	TINT                   m_iSparksEffect;        // 0x120, "carsparks"
	AParticleEffectHandle* m_apHeadlights[ 2 ];    // 0x124
	AParticleEffectHandle* m_apTaillights[ 2 ];    // 0x12c
	AParticleEffectHandle* m_apBrakeLights[ 2 ];   // 0x134, while m_iDriveState is 2
	AParticleEffectHandle* m_apReverseLights[ 2 ]; // 0x13c, in reverse ("carchasecar1" and "carchasecar3" only)
	AParticleEffectHandle* m_apSirens[ 2 ];        // 0x144, CARTYPE_COP

	TINT         m_iUnk14C;          // 0x14c
	TINT         m_iUnk150;          // 0x150
	TINT         m_iFinishPosition;  // 0x154
	DRIVESTATE   m_iDriveState;      // 0x158
	TINT         m_iUnk15C;          // 0x15c
	TINT         m_iUnk160;          // 0x160
	TFLOAT       m_fSteering;        // 0x164, analog steering input (ACarChaseMiniGameState::ProcessCommand)
	TINT         m_iUnk168;          // 0x168
	TBOOL        m_bWheelEffects;    // 0x16c, UpdateWheelEffects is called while set
	TBOOL        m_bUnk16D;          // 0x16d
	TINT         m_iIgnitionCue;     // 0x170
	TINT         m_iGearCue;         // 0x174
	ENGINESTATE_ m_eEngineState;     // 0x178
	TINT         m_iLastGear;        // 0x17c, see UpdateGearSound
	TINT         m_iEngineLoopSound; // 0x180, ASoundId of m_iEngineLoopCue / m_iEngineCue

	AGlowViewport::GlowObject* m_pGlowObject; // 0x184, removed by the destructor
	TINT                       m_iUnk188;     // 0x188
	TBOOL                      m_bUnk18C;     // 0x18c
	TFLOAT                     m_fMaxSpeed;   // 0x190, of the steer

	TINT m_aiHeadlightBones[ 2 ];    // 0x194
	TINT m_aiTaillightBones[ 2 ];    // 0x19c
	TINT m_aiBrakeLightBones[ 2 ];   // 0x1a4
	TINT m_aiReverseLightBones[ 2 ]; // 0x1ac
	TINT m_iDustEffect;              // 0x1b4, "dustpuffsmall"
	TINT m_iGrassEffect;             // 0x1b8, "grasswalk"

	Toshi::TVector4 m_avTyreTrackPositions[ NUM_WHEELS ]; // 0x1bc, VEC_ZERO when there's no track
	TFLOAT          m_afTyreTrackOffsets[ NUM_WHEELS ];   // 0x1fc
	TFLOAT          m_afTyreTrackWidths[ NUM_WHEELS ];    // 0x20c
	TINT            m_aiWheelSurfaces[ NUM_WHEELS ];      // 0x21c, -1 when the wheel is in the air
	TFLOAT          m_afDustTimers[ NUM_WHEELS ];         // 0x22c
	TFLOAT          m_afGrassTimers[ NUM_WHEELS ];        // 0x23c
	TINT            m_iGravelCue;                         // 0x24c, the player only
};

TSTATICASSERT( sizeof( AUnitCarChaseCar ) == 0x250 );
