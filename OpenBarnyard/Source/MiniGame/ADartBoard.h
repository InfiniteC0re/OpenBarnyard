#pragma once
#include "Physics/ACollisionModelInstance.h"
#include "Physics/ACollisionModelSet.h"

#include <Toshi/TPString8.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class AUnit;
class AModelInstance;

//-----------------------------------------------------------------------------
// Purpose: One dart of the Darts minigame (ADartMiniGame). The dart is a unit
// (type "dart") that the player holds, throws and that sticks in the board.
// NOTE: no class name in the original, the struct is 0x30 bytes and lives in
// the players of ADartMiniGame (ten each) and ADartMiniGame::m_oAimDart.
//-----------------------------------------------------------------------------
struct ADart
{
	// State of the dart, ADartMiniGame::UpdateGame is a state machine over the current dart
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_IDLE,      // 0, gets attached to the player's hand
		STATE_AIM_X,     // 1, human: horizontal aim
		STATE_AIM_Y,     // 2, human: vertical aim
		STATE_THROW,     // 3, throw animation starts
		STATE_AI_START,  // 4, computer: picks its timings
		STATE_AI_TARGET, // 5, computer: picks a target
		STATE_AI_MOVE,   // 6, computer: moves the aim to the target
		STATE_AI_AIM_X,  // 7, computer: horizontal aim
		STATE_AI_AIM_Y,  // 8, computer: vertical aim
		STATE_THROWING,  // 9, waiting for the release (ADartMiniGame::OnUnitAction)
		STATE_RAISE,     // 10, the player raises the dart
		STATE_FLYING,    // 11, the dart flies to the board
		STATE_RESULT,    // 12, the hit is shown
	};

	ADart();

	// Points the dart along a_rcDirection (rotated by -90 degrees around -Y)
	void SetDirection( const Toshi::TVector4& a_rcDirection );

	// $Barnyard: FUNCTION 0051f440
	void SetSpinning( TBOOL a_bSpinning ) { m_bSpinning = a_bSpinning; }

	// $Barnyard: FUNCTION 0051f450
	void UpdateSpin( TFLOAT a_fAngle )
	{
		if ( m_bSpinning == TTRUE )
			m_fSpin += a_fAngle;
	}

	// Lets the dart fall off the board
	void Drop();

	// Updates the model transform of the dart (spin and scale)
	void Update();

	// Returns the position of the dart (taking the spin into account)
	Toshi::TVector4& GetPosition( Toshi::TVector4& a_rOutPosition );

	void Reset();

	// Moves falling darts down
	static void UpdateFalling( TFLOAT a_fDeltaTime, ADart* a_pDarts, TINT a_iNumDarts );

	AUnit*          m_pUnit;     // 0x00
	STATE           m_eState;    // 0x04
	TINT            m_iScore;    // 0x08
	TINT            m_iRing;     // 0x0c, ring the dart is stuck in, 4 = none
	TBOOL           m_bOnTarget; // 0x10, stuck in a target, knocked off with it
	TBOOL           m_bFalling;  // 0x11
	Toshi::TVector4 m_vVelocity; // 0x14
	TFLOAT          m_fScale;    // 0x24
	TFLOAT          m_fSpin;     // 0x28, angle around X added by the ring it's stuck in
	TBOOL           m_bSpinning; // 0x2c
};

TSTATICASSERT( sizeof( ADart ) == 0x30 );

//-----------------------------------------------------------------------------
// Purpose: The aim and result of a throw (ADartMiniGame + 0x2ee4).
// NOTE: no class name in the original.
//-----------------------------------------------------------------------------
struct ADartThrow
{
	static constexpr TINT TARGET_NONE = 21;
	static constexpr TINT RING_NONE   = 4;
	static constexpr TINT COG_NONE    = 5;

	// The colour of the targets a player aims at
	using TEAM = TINT;
	enum TEAM_ : TEAM
	{
		TEAM_NONE   = 0,
		TEAM_YELLOW = 1,
		TEAM_BLUE   = 2,
	};

	void Reset();

	// Computes m_vDirection (from the camera to the aimed point of the board)
	void ComputeDirection();

	// Whether one of the four inner targets (the bullseyes) has been hit
	// $Barnyard: FUNCTION 0051f6a0
	static TBOOL IsBullseye( const ADartThrow& a_rcThrow ) { return TUINT( a_rcThrow.m_iTarget ) < 4; }

	TFLOAT          m_fAimX;      // 0x00, -1..1
	TFLOAT          m_fAimY;      // 0x04, -1..1
	TFLOAT          m_fTimeUsed;  // 0x08, the clock of the player shows the time minus this
	Toshi::TVector4 m_vDirection; // 0x0c
	Toshi::TVector4 m_vTarget;    // 0x1c, where the dart lands
	TINT            m_iScore;     // 0x2c
	TEAM            m_iTeam;      // 0x30
	TINT            m_iTarget;    // 0x34, TARGET_NONE = no target hit
	TINT            m_iRing;      // 0x38, RING_NONE = no ring hit
	TINT            m_iCategory;  // 0x3c, ADartBoard::Target::iCategory
	TINT            m_iCog;       // 0x40, COG_NONE = no cog hit
};

TSTATICASSERT( sizeof( ADartThrow ) == 0x44 );

//-----------------------------------------------------------------------------
// Purpose: The dart board of the Darts minigame (ADartMiniGame + 0x1430).
// Three rings (dartboard_outer/middle/inner) rotate around the X axis and
// carry twenty targets (dart_yellow_*/dart_blue_*), four cogs spin in the
// background. The targets' collision groups ("coll_y_NN"/"coll_b_NN") tell
// the team and the score of the target.
// NOTE: no class name in the original.
//-----------------------------------------------------------------------------
class ADartBoard
{
public:
	static constexpr TINT NUM_TARGETS          = 20;
	static constexpr TINT NUM_RINGS            = 3;
	static constexpr TINT NUM_COGS             = 4;
	static constexpr TINT MAX_TARGETS_PER_RING = 8;

	enum RING_ : TINT
	{
		RING_OUTER,
		RING_MIDDLE,
		RING_INNER,
	};

	struct Target
	{
		Target();

		// $Barnyard: FUNCTION 00520fa0
		~Target() = default;

		// Reads the team and the score from the name of the first collision group
		static void ReadCollisionInfo( Target* a_pTarget );

		ACollisionModelInstance oCollision;    // 0x00
		ACollisionModelSet      oCollisionSet; // 0x10
		TINT                    iBone;         // 0xa4, "<team>_<ring>_center"
		TINT                    iScore;        // 0xa8
		Toshi::TPString8        strLocator;    // 0xac, terrain locator (dart_<team>_<ring>NN)
		AModelInstance*         pModel;        // 0xb0
		ADartThrow::TEAM        iTeam;         // 0xb4
		TINT                    iRing;         // 0xb8, RING_
		TINT                    iCategory;     // 0xbc, 0 middle, 1 outer, 2 inner
		TBOOL                   bActive;       // 0xc0
		ADart*                  pDart;         // 0xc4, dart stuck in the target
		Toshi::TVector4         vVelocity;     // 0xc8
		Toshi::TVector4         vPosition;     // 0xd8
		TFLOAT                  fTime;         // 0xe8, time since it was knocked off
		TFLOAT                  fScale;        // 0xec
		TFLOAT                  fRadius;       // 0xf0
		TBOOL                   bKnockedOff;   // 0xf4
	};

	struct Ring
	{
		// $Barnyard: FUNCTION 0051f470
		Ring() = default;

		// $Barnyard: FUNCTION 0051f4c0
		~Ring() = default;

		AModelInstance*         pModel;                            // 0x00
		TINT                    iNumTargets;                       // 0x04
		TFLOAT                  fAngle;                            // 0x08
		TFLOAT                  fSpeed;                            // 0x0c, radians per second
		TFLOAT                  fScale;                            // 0x10
		ACollisionModelInstance oCollision;                        // 0x14
		ACollisionModelSet      oCollisionSet;                     // 0x24
		Target*                 apTargets[ MAX_TARGETS_PER_RING ]; // 0xb8
	};

	struct Cog
	{
		// $Barnyard: FUNCTION 0051f510
		Cog() = default;

		// $Barnyard: FUNCTION 0051f560
		~Cog() = default;

		AModelInstance*         pModel;        // 0x00
		TFLOAT                  fSpeed;        // 0x04
		TUINT8                  PADDING[ 4 ];  // 0x08
		Toshi::TMatrix44        mTransform;    // 0x0c, terrain locator cog_large/small0N
		ACollisionModelInstance oCollision;    // 0x4c
		ACollisionModelSet      oCollisionSet; // 0x5c
	};

public:
	ADartBoard();
	~ADartBoard();

	// Creates the rings, targets and cogs
	void Create();
	void Destroy();

	void SetSkeletonUpdating( TBOOL a_bUpdating );

	// Puts back every target that has been knocked off
	void ResetTargets();

	void UpdateCogs( TFLOAT a_fDeltaTime );
	void UpdateTargets( TFLOAT a_fDeltaTime );
	void RotateRing( TINT a_iRing, TFLOAT a_fAngle );

	// Casts the ray of the throw at the board. a_bUpdateTarget stores the hit position
	// in m_vTarget, a_bScore stores what has been hit (score, team, target, ring, cog).
	void CastThrow( ADartThrow& a_rThrow, TBOOL a_bUpdateTarget, TBOOL a_bScore );

	// Finds the point the dart hits (behind the board if nothing is hit)
	void ComputeTarget( ADartThrow& a_rThrow );

	// $Barnyard: FUNCTION 00520930
	TBOOL ComputeTargetEx( ADartThrow& a_rThrow )
	{
		ComputeTarget( a_rThrow );
		return TTRUE;
	}

	// $Barnyard: FUNCTION 00520ab0
	void Score( ADartThrow& a_rThrow ) { CastThrow( a_rThrow, TFALSE, TTRUE ); }

	// Whether a throw at (a_fAimX, a_fAimY) hits the target in a_fTime seconds
	TBOOL WillHitTarget( TINT a_iTarget, TFLOAT a_fAimY, TFLOAT a_fAimX, TFLOAT a_fTime );

	// Knocks the target off the board taking a_pDart with it
	void KnockOffTarget( TINT a_iTarget, ADart* a_pDart );

	// The target of a_iTeam with the highest score, ADartThrow::TARGET_NONE if there is none
	TINT FindBestTarget( TINT a_iTeam );

	// The best scoring target of a_iTeam closest to the aim position
	TINT FindClosestTarget( TINT a_iTeam, TFLOAT a_fAimX, TFLOAT a_fAimY );

	// Position of the target in a_fTime seconds
	Toshi::TVector4& GetTargetPosition( Toshi::TVector4& a_rOutPosition, TINT a_iTarget, TFLOAT a_fTime );

	// The text shown for a throw by a_iTeam
	const TWCHAR* GetHitText( const ADartThrow& a_rcThrow, TINT a_iTeam );

	// $Barnyard: FUNCTION 0051f7c0
	Ring* GetRing( TINT a_iRing ) { return &m_aRings[ a_iRing ]; }

	// $Barnyard: FUNCTION 0051f9d0
	TFLOAT GetRingSpeed( TINT a_iRing ) { return m_aRings[ a_iRing ].fSpeed; }

	void SetScoreMultiplier( TINT a_iMultiplier ) { m_iScoreMultiplier = a_iMultiplier; }

	// The direction the board faces
	static Toshi::TVector4& GetNormal( Toshi::TVector4& a_rOutNormal );

	// The corners of the board (top, right, left, bottom)
	static TBOOL GetCorners( Toshi::TVector4& a_rOutTop, Toshi::TVector4& a_rOutRight, Toshi::TVector4& a_rOutLeft, Toshi::TVector4& a_rOutBottom );

	// Converts a position on the board to an aim (-1..1)
	static void GetAimFromPosition( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rOutAimX, TFLOAT& a_rOutAimY );

private:
	void CreateTargets();
	void CreateCogs();

private:
	TUINT8 PADDING[ 0x100 ];          // 0x0000
	TINT   m_iUnk1;                   // 0x0100
	TINT   m_iUnk2;                   // 0x0104
	Target m_aTargets[ NUM_TARGETS ]; // 0x0108
	Ring   m_aRings[ NUM_RINGS ];     // 0x1468
	Cog    m_aCogs[ NUM_COGS ];       // 0x16f0
	TINT   m_iScoreMultiplier;        // 0x1ab0
};

// NOTE: retail sizes are 0xf8 (Target), 0xd8 (Ring), 0xf0 (Cog) and 0x1ab4 (ADartBoard),
// ACollisionModelSet is 4 bytes smaller here, so the offsets after it are off by 4.
