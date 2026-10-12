#pragma once
#include <Toshi/TObject.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Math/TVector3.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

#include "ASteerTraits.h"

class ASteer;
class AKinematicsObstruction;
class AInteriorNetwork;

//-----------------------------------------------------------------------------
// Purpose: The obstruction lists a kinematics is pushed out of while it moves
// (ASteerKinematics::Update). A steer fills it with the obstructions of its
// interior network and the ones of AKinematicsObstructionManager right before
// updating the kinematics and clears it afterwards.
// NOTE: a single static instance lives at 0x00788ca8 (s_oObstructionLists).
//-----------------------------------------------------------------------------
struct AKinematicsObstructionLists
{
	static constexpr TINT MAX_NUM_LISTS = 2;

	// NOTE: same layout as Toshi::T2Vector<AKinematicsObstruction*, N> for any N
	struct List
	{
		TINT                    iNumObstructions;
		AKinematicsObstruction* apObstructions[ 1 ];
	};

	// Adds the obstructions of a_pNetwork and the ones of AKinematicsObstructionManager
	void Add( AInteriorNetwork* a_pNetwork );

	// Removes every list
	void Clear();

	TINT  iNumLists;
	List* apLists[ MAX_NUM_LISTS ];
};

//-----------------------------------------------------------------------------
// Purpose: Movement integrator owned by a steer (ASteer vtbl +0x114). Most of the
// ASteer base class forwards to it. The base class describes a kinematics that
// doesn't move at all, ASteerKinematics2D (AInteriorSteer, TClass 0x00789680) and
// ASteerKinematics3D (APlayerSteer, TClass 0x007896b8) do the actual work.
// Allocated by AKinematicsPool. Names follow JPOG's ASteerKinematics exports
// where the behaviour matches.
// NOTE: vtable 0x007276f8, TClass 0x00789648 (version 1.0, size 0x14, not
// creatable at runtime). Slots that only have shared stubs in the base class
// keep their Unk_ names.
//-----------------------------------------------------------------------------
class ASteerKinematics
    : public Toshi::TObject
    , public Toshi::T2DList<ASteerKinematics>::Node
{
public:
	TDECLARE_CLASS( ASteerKinematics, Toshi::TObject );

	// a_uiFlags of Update
	using UPDATEFLAGS = TUINT;
	enum UPDATEFLAGS_ : UPDATEFLAGS
	{
		UPDATEFLAGS_NONE                = 0,
		UPDATEFLAGS_NO_AVOID            = BITFLAG( 0 ), // UpdateSeek doesn't steer around the obstructions
		UPDATEFLAGS_NO_PUSH             = BITFLAG( 1 ), // UpdateObstructions isn't called, the obstructions don't push the kinematics
		UPDATEFLAGS_NO_OBSTRUCTION_STOP = BITFLAG( 2 ), // doesn't stop and wait when the way is blocked
	};

public:
	ASteerKinematics();
	~ASteerKinematics();

	// Initialises the kinematics of a new steer
	virtual TBOOL Create( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, const ASteerTraits* a_pTraits, TFLOAT a_fSpeed ); // +0x0c

	// Takes over the state of a_pOther (AUnit::ChangeSteer)
	virtual TBOOL CreateFrom( ASteer* a_pOther ); // +0x10

	virtual TFLOAT                 GetMeasuredSpeedRatio() { return 0.0f; }                         // +0x14, 0056ce20 (shared), measured speed / max speed
	virtual TFLOAT                 GetMeasuredSpeed() { return 0.0f; }                              // +0x18, 0056ce20 (shared), measured speed
	virtual void                   SetMeasuredSpeed( TFLOAT a_fSpeed ) {}                           // +0x1c, DOES_NOTHING 00628f40, sets the measured speed
	virtual void                   BlendMeasuredSpeed( TFLOAT a_fSpeed ) {}                         // +0x20, DOES_NOTHING 00628f40, blends the measured speed towards a_fSpeed
	virtual const Toshi::TVector4* GetPosition();                                                   // +0x24
	virtual Toshi::TVector4        Unk_0x28() { return Toshi::TVector4::VEC_ZERO; }                 // +0x28, 005702b0 (shared), velocity
	virtual Toshi::TVector4        GetVelocity() { return Toshi::TVector4::VEC_ZERO; }              // +0x2c, 0061d680 (shared)
	virtual const Toshi::TVector4* GetFacingDir();                                                  // +0x30
	virtual const Toshi::TVector4* GetMovementDir();                                                // +0x34
	virtual TFLOAT                 GetSpeed() { return 0.0f; }                                      // +0x38, 0056ce20 (shared)
	virtual TFLOAT                 GetDesiredSpeed() { return 0.0f; }                               // +0x3c, 0056ce20 (shared), desired speed
	virtual TFLOAT                 GetTurnSpeed() { return 0.0f; }                                  // +0x40, 0056ce20 (shared), turning speed
	virtual const Toshi::TVector4* GetSeekTarget();                                                 // +0x44
	virtual TBOOL                  IsFacingAtDesiredFacing( TFLOAT a_fTolerance ) { return TTRUE; } // +0x48, RETURNS_TRUE 00413920
	virtual void                   SetDesiredFacing( const Toshi::TVector4& a_rcFacing ) {}         // +0x4c, DOES_NOTHING 00628f40
	virtual const Toshi::TVector4* GetDesiredFacing() { return &Toshi::TVector4::VEC_ZERO; }        // +0x50, folded with GetPosition (005747d0), desired facing
	virtual TFLOAT                 GetGroundHeight() { return 0.0f; }                               // +0x54, 0056ce20 (shared)

	// NOTE: a_iTime is a float in ASteerKinematics3D, ASteer::PredictPosition passes it through as an int
	virtual Toshi::TVector4& PredictPosition( TINT a_iTime, Toshi::TVector4& a_rOut ) // +0x58, 00573200 (shared), position after a_iTime seconds
	{
		a_rOut = Toshi::TVector4::VEC_ZERO;
		return a_rOut;
	}

	virtual Toshi::TVector2* GetAvoidPush() { return TNULL; }          // +0x5c, RETURNS_ZERO 0054d8b0, avoidance push (ASteerKinematics2D)
	virtual TFLOAT           GetAvoidRadius() { return 0.0f; }         // +0x60, 0056ce20 (shared)
	virtual void             CopyPosition( Toshi::TVector4& a_rOut );  // +0x64
	virtual void             CopyFacingDir( Toshi::TVector4& a_rOut ); // +0x68
	virtual void             StopMoving() {}                           // +0x6c, DOES_NOTHING 005acd80, stops

	// Integrates the movement, pushing the kinematics out of the obstructions in a_pObstructions
	// except for the a_iNumIgnored ones in a_ppIgnored.
	virtual void Update( TFLOAT a_fDeltaTime, AKinematicsObstructionLists* a_pObstructions, AKinematicsObstruction** a_ppIgnored, TINT a_iNumIgnored, UPDATEFLAGS a_uiFlags ) {} // +0x70, 00539d40 (shared, does nothing)

	virtual void   ApplyGravity( TFLOAT a_fDeltaTime ) {}                                                                                                   // +0x74, DOES_NOTHING 00628f40
	virtual void   UpdateFacing( TFLOAT a_fDeltaTime ) {}                                                                                                   // +0x78, DOES_NOTHING 00628f40
	virtual void   MoveTo( TFLOAT a_fDeltaTime, const Toshi::TVector4* a_pPrevPosition, const Toshi::TVector4* a_pPosition ) {}                             // +0x7c, DOES_NOTHING 00413930
	virtual TBOOL  TurnTowards( const Toshi::TVector4& a_rcTarget ) { return TTRUE; }                                                                       // +0x80, RETURNS_TRUE 00413920, faces a_rcTarget, TTRUE once there
	virtual TBOOL  TurnTowardsWithin( const Toshi::TVector4& a_rcTarget, TINT a_iUnused, TFLOAT a_fToleranceSq, TFLOAT a_fMinDistanceSq ) { return TTRUE; } // +0x84, 00622750 (shared, returns TTRUE)
	virtual TBOOL  SetPosition( const Toshi::TVector4& a_rcPosition ) { return TFALSE; }                                                                    // +0x88, RETURNS_FALSE 006ce000
	virtual void   SetFacing( const Toshi::TVector4& a_rcFacing ) {}                                                                                        // +0x8c, DOES_NOTHING 00628f40
	virtual void   SetSpeed( TFLOAT a_fSpeed ) {}                                                                                                           // +0x90, DOES_NOTHING 00628f40
	virtual void   SetDesiredSpeed( TFLOAT a_fSpeed ) {}                                                                                                    // +0x94, DOES_NOTHING 00628f40
	virtual void   SetDesiredMovementDir( const Toshi::TVector4& a_rcFacing ) {}                                                                            // +0x98, DOES_NOTHING 00628f40, sets the desired facing's x and z
	virtual void   Unk_0x9c( TBOOL a_bEnable ) {}                                                                                                           // +0x9c, DOES_NOTHING 00628f40
	virtual void   SetFrozen( TBOOL a_bFreeze ) {}                                                                                                          // +0xa0, DOES_NOTHING 00628f40, stops integrating the movement
	virtual void   Unk_0xa4( TINT a_iUnk ) {}                                                                                                               // +0xa4, DOES_NOTHING 00628f40
	virtual TFLOAT GetNHolonomicRate();                                                                                                                     // +0xa8, returns m_pTraits->m_fNHolonomicRate

	// Starts seeking a_rcTarget from a_rcStart
	virtual TBOOL SeekAlongLine( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcTarget ) { return TFALSE; } // +0xac, RETURNS_ZERO 00556a00

	virtual TBOOL  Seek( const Toshi::TVector4& a_rcTarget ) { return TFALSE; }                                                         // +0xb0, RETURNS_FALSE 006ce000
	virtual TBOOL  SeekFacing( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcFacing, TFLOAT a_fUnk ) { return TFALSE; } // +0xb4, 006528c0 (shared, returns TFALSE)
	virtual void   Halt() {}                                                                                                            // +0xb8, DOES_NOTHING 005acd80
	virtual TBOOL  IsSeekingConstant() { return TFALSE; }                                                                               // +0xbc, RETURNS_FALSE 00660960
	virtual TBOOL  HasMoved() { return TFALSE; }                                                                                        // +0xc0, RETURNS_FALSE 00660960, moved during the last update
	virtual void   SetSeekOffset( TFLOAT a_fOffset ) {}                                                                                 // +0xc4, DOES_NOTHING 00628f40
	virtual void   ResetSeekOffset() {}                                                                                                 // +0xc8, DOES_NOTHING 005acd80, resets the seek offset
	virtual void   Unk_0xcc( TFLOAT a_fUnk ) {}                                                                                         // +0xcc, DOES_NOTHING 00628f40
	virtual void   SetSeekSlowdown( TBOOL a_bEnable ) {}                                                                                // +0xd0, DOES_NOTHING 00628f40, slows down close to the seek target
	virtual TFLOAT GetAirTime() { return 0.0f; }                                                                                        // +0xd4, 0056ce20 (shared), time spent in the air
	virtual void   SetAirTime( TFLOAT a_fTime ) {}                                                                                      // +0xd8, DOES_NOTHING 00628f40, sets the time spent in the air
	virtual void   ResetAirTime() {}                                                                                                    // +0xdc, DOES_NOTHING 005acd80, resets the time spent in the air
	virtual void   SetVerticalSpeed( TFLOAT a_fSpeed ) {}                                                                               // +0xe0, DOES_NOTHING 00628f40, sets the vertical speed
	virtual TFLOAT GetVerticalSpeed() { return 0.0f; }                                                                                  // +0xe4, 0056ce20 (shared), vertical speed
	virtual void   GetGroundPosition( TINT a_iUnk1, TINT a_iUnk2 ) {}                                                                   // +0xe8, 0054df00 (shared, does nothing)

	// Tilt of the model (pitch and roll), TFALSE while it shouldn't be applied
	virtual TBOOL GetTilt( TFLOAT* a_pPitch, TFLOAT* a_pRoll ) { return TFALSE; } // +0xec, RETURNS_ZERO 00556a00

	// $Barnyard: FUNCTION 00574460
	TFLOAT GetSpeedRatio()
	{
		if ( m_pTraits->m_fMaxSpeed <= 0.0f )
			return 0.0f;

		return GetSpeed() / m_pTraits->m_fMaxSpeed;
	}

	// $Barnyard: FUNCTION 00574790
	TFLOAT GetSpeedFromModifier( TFLOAT a_fModifier )
	{
		return a_fModifier * m_pTraits->m_fMaxSpeed;
	}

	ASteer*             GetSteer() const { return m_pSteer; }
	void                SetSteer( ASteer* a_pSteer ) { m_pSteer = a_pSteer; }
	const ASteerTraits* GetTraits() const { return m_pTraits; }

protected:
	// Direction from a_rcPosition to follow the line from a_rcStart to a_rcTarget, VEC_ZERO once
	// there's nothing to correct. a_pOutDistance is the distance from the line, a_pOutPassed is set
	// once a_rcPosition is past a_rcTarget. a_bDirect ignores the line and heads for a_rcTarget.
	static Toshi::TVector4 GetSeekDirection( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcFacing, TFLOAT* a_pOutDistance, TBOOL* a_pOutPassed, TBOOL a_bDirect );

public:
	static AKinematicsObstructionLists s_oObstructionLists; // 0x00788ca8

protected:
	ASteer*             m_pSteer;  // 0x0c, the owner steer (set by the steer after Create/CreateFrom)
	const ASteerTraits* m_pTraits; // 0x10
};

TSTATICASSERT( sizeof( ASteerKinematics ) == 0x14 );
