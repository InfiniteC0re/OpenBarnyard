#pragma once
#include "AUnitPlayer.h"

#include <Math/TVector4.h>

class AInteriorSteer;

//-----------------------------------------------------------------------------
// A counted array of positions used by the Sharp Squirter minigame (the
// waypoints of the cows, the target positions of APrecisionSquirt).
// NOTE: the name is invented, retail has one copy of Add for every size.
//-----------------------------------------------------------------------------
template <TINT MAX_SIZE>
struct APrecisionPositionList
{
	// $Barnyard: FUNCTION 00503020
	void Add( const Toshi::TVector4& a_rcPosition )
	{
		aPositions[ iCount++ ] = a_rcPosition;
	}

	TINT            iCount;                 // 0x0
	Toshi::TVector4 aPositions[ MAX_SIZE ]; // 0x4
};

//-----------------------------------------------------------------------------
// A Jersey cow of the Sharp Squirter minigame (unit type "sharp_jersy", spawned
// at the "cow1".."cow4" goals by APrecisionSquirt). It runs between its goals,
// stops to idle or yawn, and squirts milk at the player with its aimed launcher.
// When the player squirts it, it "yuk"s for a while.
// NOTE: the TClass (007091b0, size 0x130) names ACharacterUnit as the parent
// class, but the object is built on AUnitPlayer (it needs its launchers).
// vtables 00721998, 0072198c.
//-----------------------------------------------------------------------------
class APrecisionJersyCow
    : public AUnitPlayer
{
public:
	TDECLARE_CLASS( APrecisionJersyCow, AUnitPlayer );

	static constexpr TINT MAX_WAYPOINTS = 5;

	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_RUN,    // 0, runs to the next waypoint
		STATE_IDLE,   // 1, idles or yawns
		STATE_MILKED, // 2, "yuk_start" ... "yuk_end"
		STATE_SQUIRT, // 3, squirts at the player
		STATE_UNK4,   // 4
	};

	// Progress of STATE_MILKED (UpdateMilked)
	using MILKSTATE = TINT;
	enum MILKSTATE_ : MILKSTATE
	{
		MILKSTATE_START = 0, // plays "yuk_start"
		MILKSTATE_YUK   = 2, // waits 16 seconds
		MILKSTATE_END   = 3, // plays "yuk_end" until it's over
	};

public:
	APrecisionJersyCow();
	~APrecisionJersyCow();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void  OnDestroy() OVERRIDE;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Set by AMilkPrecisionProjectile when the milk of the cow has hit the player
	void SetHitPlayer( TBOOL a_bHit );

	void SetState( STATE_ a_eState );

	// The player squirted the cow
	void SetMilked( TBOOL a_bMilked );

	// Sets the player the cow squirts at
	void SetPlayer( AUnit* a_pPlayer );

	// Plays "yuk_start" and "yuk_end", TTRUE once it's done
	TBOOL UpdateMilked( TFLOAT a_fDeltaTime );

	// Idles for a while, TTRUE once it's done
	TBOOL UpdateIdle( TFLOAT a_fDeltaTime );

	// Faces the player and squirts at him every now and then
	void UpdateSquirt( TFLOAT a_fDeltaTime );

public:
	TFLOAT                                m_fIdleTime;     // 0xb0
	TFLOAT                                m_fMilkedTime;   // 0xb4
	TFLOAT                                m_fSquirtTimer;  // 0xb8
	TBOOL                                 m_bMilked;       // 0xbc
	TBOOL                                 m_bStateChanged; // 0xbd
	TBOOL                                 m_bHitPlayer;    // 0xbe
	AUnit*                                m_pPlayer;       // 0xc0
	AInteriorSteer*                       m_pSteer;        // 0xc4
	TINT                                  m_iWaypoint;     // 0xc8
	STATE_                                m_eState;        // 0xcc
	MILKSTATE                             m_iMilkedState;  // 0xd0
	APrecisionPositionList<MAX_WAYPOINTS> m_oWaypoints;    // 0xd4, the goals of the same cow index
	TINT                                  m_iMapIcon;      // 0x128
	TINT                                  m_iCowIndex;     // 0x12c, "cow1".."cow4"
};

TSTATICASSERT( sizeof( APrecisionJersyCow ) == 0x130 );
