#pragma once
#include "ACommonSteer.h"

class APhysicsBike;
class AParticleEffectHandle;

//-----------------------------------------------------------------------------
// Purpose: Steer of the motorbikes. The movement comes from an APhysicsBike
// (allocated from the bike physics pool in CreatePhysics) driven by
// SetDesiredVelocity (the input of ABikeControlComponent, or the computer
// when m_bAIControlled is set). Also plays the sounds and particle effects
// of the bike, its lights at night and its tyre tracks (AVehicleSkidManager).
// TClass 0x00788b68, vtable 0x007250c8, size 0x134.
//-----------------------------------------------------------------------------
class ABikeSteer : public ACommonSteer
{
public:
	TDECLARE_CLASS( ABikeSteer, ACommonSteer );

	static constexpr TINT NUM_TYRE_TRACKS = 2;

	// The trick animation (m_iTrickState), advanced by ABikeAnimController
	using TRICKSTATE = TINT;
	enum TRICKSTATE_ : TRICKSTATE
	{
		TRICKSTATE_NONE  = 0,
		TRICKSTATE_START = 1, // "bk_trick_*_start"
		TRICKSTATE_LOOP  = 2, // "bk_trick_*_loop", sets APhysicsBike::m_bForceThrowRider
		TRICKSTATE_END   = 3, // "bk_trick_*_end"
	};

	enum CUE_ : TINT
	{
		CUE_ENGINE, // 0x88, 247
		CUE_LAND,   // 0x8c, 246, landing hard
		CUE_CRASH,  // 0x90, 245, crashing into things
		CUE_SKID,   // 0x94, 267
		CUE_CHAIN,  // 0x98, 269, only played by the latest version (the chain ticks)
		CUE_THROWN, // 0x9c, 607/641, the rider is thrown off
		CUE_BUSH,   // 0xa0, 272
		CUE_NUMOF
	};

	enum EFFECT_ : TINT
	{
		EFFECT_WATERSPLASH2A,   // 0xc8
		EFFECT_WATERSPLASH2B,   // 0xcc
		EFFECT_WATERRIPPLE,     // 0xd0
		EFFECT_MUDSPLASH,       // 0xd4
		EFFECT_MUDRIPPLES,      // 0xd8
		EFFECT_DUSTPUFFSMALL,   // 0xdc
		EFFECT_GRASSWALK,       // 0xe0
		EFFECT_BIKE_TAILLIGHT,  // 0xe4
		EFFECT_BIKE_FRONTLIGHT, // 0xe8
		EFFECT_BUSHHIT,         // 0xec
		EFFECT_NUMOF
	};

public:
	ABikeSteer();
	~ABikeSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual TBOOL                  CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL                  Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL                  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition ) OVERRIDE;                                                                // +0x84
	virtual TBOOL                  SetPosition3( const Toshi::TVector3& a_rcPosition ) OVERRIDE;                                                               // +0x88
	virtual void                   SetDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE;                                                              // +0x8c
	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition ) OVERRIDE;                                                                   // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition ) OVERRIDE;                                                                  // +0xa8
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                                                                                     // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection ) OVERRIDE;                                                                 // +0xb0
	virtual void                   CopyDirection3( Toshi::TVector3& a_rOutDirection ) OVERRIDE;                                                                // +0xb4
	virtual const Toshi::TVector4* GetDirection() OVERRIDE;                                                                                                    // +0xb8
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                                                                                     // +0xc0

	// NOTE: folded with other getters of a float at +0x30 (0061bfc0)
	virtual TFLOAT GetSpeed() OVERRIDE { return m_fSpeed; } // +0xc4

	// NOTE: folded with other float getters returning 0.0f (0056ce20)
	virtual TFLOAT GetDesiredSpeed() OVERRIDE { return 0.0f; } // +0xc8

	// NOTE: folded with other setters of a float at +0x30 (005f0e00). The bike has no
	// ASteerKinematics, so ASteer::SetSpeed (GetKinematics()->SetSpeed) must not be reached
	virtual void SetSpeed( TFLOAT a_fSpeed ) OVERRIDE { m_fSpeed = a_fSpeed; } // +0x100

	// RETURNS_TRUE stubs: the bike can't be halted/seeked through the kinematics
	virtual TBOOL Halt() OVERRIDE { return TTRUE; }                                                // +0x138, 0046cde0
	virtual TBOOL HaltAtPosition( const Toshi::TVector3& a_rcPosition ) OVERRIDE { return TTRUE; } // +0x13c, 00413920
	virtual TBOOL HaltFacing( const Toshi::TVector3& a_rcFacing ) OVERRIDE { return TTRUE; }       // +0x140, 00413920

	// $Barnyard: FUNCTION 00558f80
	virtual TBOOL Seek( const Toshi::TVector4& a_rcTarget, TFLOAT a_fSeekOffset ) OVERRIDE { return TTRUE; }                    // +0x144
	virtual TBOOL SeekFacing( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcFacing ) OVERRIDE { return TTRUE; } // +0x148, 00558f80

	virtual TBOOL HaltFacingDesired( const Toshi::TVector3& a_rcFacing ) OVERRIDE { return TTRUE; } // +0x14c, 00413920
	virtual TBOOL IsSeekingConstant() OVERRIDE { return TFALSE; }                                   // +0x150, RETURNS_FALSE 00660960
	virtual TBOOL IsFacingAtDesiredFacing() OVERRIDE { return TTRUE; }                              // +0x154, 0046cde0

	//-----------------------------------------------------------------------------
	// ACommonSteer
	//-----------------------------------------------------------------------------

	// The input: x = steering (-1..1), y = lean, z = throttle (> 0) or brake (< 0), w = boost (>= 2)
	virtual TBOOL SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x174

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// The bike is driven by the computer towards the seek target (bike races)
	void SetAIControlled( TBOOL a_bAIControlled );

	void   SetStamina( TFLOAT a_fStamina );
	TFLOAT GetStamina() const { return m_fStamina; }

	// TRUE if the bike has been in the water for more than a second
	TBOOL IsSunk() const;

	// Makes the computer drive to a_rcTarget
	TBOOL SetSeekTarget( const Toshi::TVector4& a_rcTarget );

	void SetTrick( TBOOL a_bHeld, TINT a_iDirection );

	// Faces the bike towards the point
	TBOOL FacePosition( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW );

	// Makes the bike jump
	void Jump();

	// Stops the bike (the rider gets off), a_bFreeze also freezes the physics
	void Stop( TBOOL a_bFreeze );

	// Lets the bike be ridden
	void Start();

	// No wheel touches the ground
	TBOOL IsAirborne() const;

	void SetStoppingToGetOff( TBOOL a_bUnk );

	// Whether the rider can get off on the right (TRUE) or left (FALSE) side
	TBOOL CanGetOff( TBOOL a_bRight );

	void SetBikeType( TINT a_iType );
	TINT GetBikeType() const { return m_iBikeType; }

	void PlayHorn();
	void PlayCrashSound();
	void PlayLandSound();

	// Turns the bike into a parked one (heavier, can't move)
	void Park( TINT a_iUnk );

	// Puts the bike at the position, stopped
	void Reset( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	APhysicsBike*    GetPhysics() const { return m_pPhysics; }
	Toshi::TVector4& GetLastGroundPosition() { return m_vecLastGroundPosition; }
	Toshi::TVector4& GetLastPosition() { return m_vecLastPosition; }
	TBOOL            HasCrashAnim() const { return m_bHasCrashAnim; }
	void             SetHasCrashAnim( TBOOL a_bUnk ) { m_bHasCrashAnim = a_bUnk; }
	TBOOL            IsAIControlled() const { return m_bAIControlled; }
	TBOOL            IsSeeking() const { return m_bSeeking; }                     // the AI hasn't reached the seek target yet
	void             SetSeekSpeed( TFLOAT a_fSpeed ) { m_fSeekSpeed = a_fSpeed; } // throttle of the AI for the next update (folded setter 004a3920)
	TBOOL            IsStopped() const { return m_bStopped; }
	TBOOL            IsInWater() const { return m_bInWater; }
	TBOOL            IsJumping() const { return m_bJump; } // starts bk_jump_start (ABikeAnimController)
	TBOOL            IsSkidding() const { return m_bSkidding; }
	TRICKSTATE       GetTrickState() const { return m_iTrickState; }
	TBOOL            IsTrickHeld() const { return m_bTrickHeld; }            // trick button held (ABikeAnimController)
	TINT             GetTrickDirection() const { return m_iTrickDirection; } // trick direction: 1 left, 2 centre, else right (ABikeAnimController)
	TBOOL            WasAirborne() const { return m_bAirborne; }             // no wheel touched the ground in the last update
	void             SetTrickState( TRICKSTATE a_iUnk ) { m_iTrickState = a_iUnk; }

private:
	void Initialise( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );
	void CreatePhysics( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	void  UpdateControlLock();
	void  UpdateSkidding();
	void  UpdateSpeed( TFLOAT a_fDeltaTime );
	void  UpdateRespawn();
	void  UpdateTyreTrackSizes();
	void  UpdateStamina( TFLOAT a_fDeltaTime );
	void  UpdateAIControl( TFLOAT a_fDeltaTime );
	void  UpdateWater( TFLOAT a_fDeltaTime );
	TBOOL UpdatePlayerControl( TFLOAT a_fDeltaTime );
	void  UpdateSounds( TFLOAT a_fDeltaTime );
	void  UpdateEffects( TFLOAT a_fDeltaTime );
	void  UpdateCollisions( TFLOAT a_fDeltaTime );

private:
	Toshi::TVector4        m_vecSeekTarget;                          // 0x20
	TFLOAT                 m_fSpeed;                                 // 0x30, velocity along the direction
	TFLOAT                 m_fAirTime;                               // 0x34, time spent in the air without touching anything
	Toshi::TVector4        m_vecLastGroundPosition;                  // 0x38, position when the bike last stood on both wheels
	Toshi::TVector4        m_vecLastPosition;                        // 0x48
	TFLOAT                 m_fSeekSpeed;                             // 0x58
	TFLOAT                 m_fHornTime;                              // 0x5c
	TFLOAT                 m_fStamina;                               // 0x60
	TFLOAT                 m_fStaminaTime;                           // 0x64
	TBOOL                  m_bExhausted;                             // 0x68
	TBOOL                  m_bTrickHeld;                             // 0x69
	TINT                   m_iTrickDirection;                        // 0x6c
	TRICKSTATE             m_iTrickState;                            // 0x70
	TBOOL                  m_bAirborne;                              // 0x74
	TBOOL                  m_bSeeking;                               // 0x75
	TBOOL                  m_bStopped;                               // 0x76
	TBOOL                  m_bUnk77;                                 // 0x77
	TBOOL                  m_bInWater;                               // 0x78
	TFLOAT                 m_fInWaterTime;                           // 0x7c
	TBOOL                  m_bStoppingToGetOff;                      // 0x80
	TBOOL                  m_bJump;                                  // 0x81, the bike jumped
	TBOOL                  m_bSkidding;                              // 0x82
	TBOOL                  m_bHasCrashAnim;                          // 0x83
	TBOOL                  m_bAIControlled;                          // 0x84
	TBOOL                  m_bOffTerrain;                            // 0x85
	TINT                   m_aCues[ CUE_NUMOF ];                     // 0x88
	TFLOAT                 m_fChainTickTime;                         // 0xa4, time since the last chain tick (latest version)
	TINT                   m_iChainTickPitch;                        // 0xa8, index of the pitch of the next chain tick (latest version)
	TFLOAT                 m_afEffectTimers[ 6 ];                    // 0xac
	APhysicsBike*          m_pPhysics;                               // 0xc4
	TINT                   m_aiEffects[ EFFECT_NUMOF ];              // 0xc8, AParticleEffectManager::FindEffect
	AParticleEffectHandle* m_pTailLight;                             // 0xf0, attached to breaklight_loc at night
	AParticleEffectHandle* m_pFrontLight;                            // 0xf4, attached to headlight_loc at night
	TINT                   m_iBikeType;                              // 0xf8
	Toshi::TVector4        m_aTyreTrackPositions[ NUM_TYRE_TRACKS ]; // 0xfc
	TFLOAT                 m_afTyreTrackOffsets[ NUM_TYRE_TRACKS ];  // 0x11c
	TFLOAT                 m_afTyreTrackWidths[ NUM_TYRE_TRACKS ];   // 0x124
	TINT                   m_aiGroundMaterials[ NUM_TYRE_TRACKS ];   // 0x12c, ground material under each wheel (-1 if in the air)

#ifdef BARNYARD_LATEST
	// NOTE: at 0xb8 on the Wii, here at the end so the PC layout is kept
	TFLOAT m_fSkidSoundTime; // the skid sound plays at least this long
#endif
};

#ifdef BARNYARD_LATEST
TSTATICASSERT( sizeof( ABikeSteer ) == 0x138 );
#else
TSTATICASSERT( sizeof( ABikeSteer ) == 0x134 );
#endif
