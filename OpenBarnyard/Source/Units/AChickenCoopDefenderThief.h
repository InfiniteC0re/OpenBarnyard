#pragma once
#include "AAnimatedUnit.h"
#include "MiniGame/AChickenCoopDefenderGame.h"
#include "Animation/AAnimationBreakpointUnitAction.h"

#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AParticleEffectHandle;

// NOTE: unknown object of AChickenCoopDefenderThief::m_pUnke0. Retail never sets it, but
// OnDestroy destroys it through its vtable (+0x0c, then the deleting destructor at +0x08).
class AChickenCoopDefenderThiefUnk
{
public:
	virtual void Unk_0x00()                 = 0;
	virtual void Unk_0x04()                 = 0;
	virtual ~AChickenCoopDefenderThiefUnk() = default; // +0x08
	virtual void Destroy()                  = 0;       // +0x0c
};

//-----------------------------------------------------------------------------
// A thief of Chicken Coop Defender: a raccoon (unit type "chickencoop_raccoon",
// steals eggs), a coyote ("chickencoop_coyote", steals hens and attacks the
// player) or Freddy in his dream ("chickencoop_evilfreddy", steals roast hens).
// Ten of each kind are created by AChickenCoopDefender and wait in the free
// lists of AChickenCoopDefenderGame until a wave spawns them. A thief picks up
// a loose hen/egg, or walks into the coop and comes out with one, and runs back
// to "thiefspawn1". Squirted with milk it drops what it carries, gets stunned
// and runs away; the player can pick a stunned raccoon up and throw it away.
//
// TClass 00709560 (parent AAnimatedUnit, size 0x108), vtables 00722b20,
// 00722b14 (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AChickenCoopDefenderThief
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AChickenCoopDefenderThief, AAnimatedUnit );

	// AUnit flag set while the thief is around (cleared when it's hidden, stunned or carried)
	// NOTE: the meaning for AUnit is unknown, see AVeggiePatchThief::FLAGS_MOVING
	static constexpr FLAGS FLAGS_MOVING = BITFLAG( 9 );

	enum KIND_ : TINT
	{
		KIND_RACCOON, // 0, "chickencoop_raccoon"
		KIND_COYOTE,  // 1, "chickencoop_coyote"
		KIND_FREDDY,  // 2, "chickencoop_evilfreddy"
		KIND_NUMOF
	};

	enum STATE_ : TINT
	{
		STATE_SPAWN,       // 0, placed at "thiefspawn1"
		STATE_SEEK_LOOSE,  // 1, goes to the closest loose hen (egg for the raccoons)
		STATE_CHASE,       // 2, runs to it
		STATE_PICKUP,      // 3, "pickup"
		STATE_SEEK_COOP,   // 4, goes into the coop ("Goal0")
		STATE_IN_COOP,     // 5, hidden in the coop for 5 seconds
		STATE_ESCAPE,      // 6, runs back to "thiefspawn1" with a hen/egg
		STATE_FLEE,        // 7, runs back to "thiefspawn1" with nothing
		STATE_GONE,        // 8, back in the free list (hidden)
		STATE_MILKED,      // 9, stunned
		STATE_MILKED_DROP, // 10, stunned, drops the hen/egg
		STATE_PICKED_UP,   // 11, carried by the player (raccoons)
		STATE_THROWN,      // 12, thrown away by the player
		STATE_SCARED,      // 13, runs away from the player
		STATE_STALK,       // 14, the coyote goes for the player
		STATE_ATTACK,      // 15, "attack"
		STATE_IDLE,        // 16
	};

	enum SPEED_ : TINT
	{
		SPEED_IDLE,   // 0, the animation is left alone
		SPEED_STAND,  // 1, idle_lp
		SPEED_WALK,   // 2, walk_lp
		SPEED_RUN,    // 3, run_lp
		SPEED_SCARED, // 4, scared
	};

public:
	AChickenCoopDefenderThief();
	~AChickenCoopDefenderThief();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  OnDestroy() override;                            // +0x60
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	TBOOL SetState( STATE_ a_eState );

	// Hit by milk: stunned (drops the hen/egg it carries)
	void OnMilked();

	// The thief's "pickup" and the coyote's "attack" animation breakpoints (AChickenCoopDefender::OnUnitAction)
	void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent );

	// Sets the game up and the bones the hens/eggs are carried with
	void SetGame( AChickenCoopDefenderGame* a_pGame );

	// Velocity of the thrown thief
	// $Barnyard: FUNCTION 00515c40
	void SetThrowVelocity( Toshi::TVector4 a_vVelocity ) { m_vVelocity = a_vVelocity; }

	// Icon of the thief on the minimap (created on demand)
	TINT GetMapIcon();

	STATE_ GetState() const { return m_eState; }
	KIND_  GetKind() const { return m_eKind; }
	AUnit* GetCarried() const { return m_pCarried; }

	void SetNetwork( AInteriorNetwork* a_pNetwork ) { m_pNetwork = a_pNetwork; }

	// Returns TTRUE if the line from a_rcFrom to a_rcTo isn't blocked by the terrain or a collision object
	static TBOOL IsLineClear( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo );

private:
	void SetSpeed( SPEED_ a_eSpeed );

	// Walks or runs depending on the speed of the steer
	void UpdateSpeed( ASteer* a_pSteer );

	void UpdateAnimation( TFLOAT a_fDeltaTime );
	void UpdateSound( TFLOAT a_fDeltaTime );

	// m_fScare grows while the player is close and looking at the thief
	void UpdateScare( TFLOAT a_fDeltaTime );

	// The list of loose hens/eggs this thief goes for
	TBOOL IsTargetLoose();

	// STATE_SEEK_LOOSE, or STATE_SEEK_COOP if there's nothing loose
	void SeekSomething();

	void UpdateSpawn( TFLOAT a_fDeltaTime );
	void UpdateSeekLoose( TFLOAT a_fDeltaTime );
	void UpdateChase( TFLOAT a_fDeltaTime );
	void UpdatePickup( TFLOAT a_fDeltaTime );
	void UpdateSeekCoop( TFLOAT a_fDeltaTime );
	void UpdateInCoop( TFLOAT a_fDeltaTime );
	void UpdateEscape( TFLOAT a_fDeltaTime );
	void UpdateFlee( TFLOAT a_fDeltaTime );
	void UpdateMilked( TFLOAT a_fDeltaTime );
	void UpdateMilkedDrop( TFLOAT a_fDeltaTime );
	void UpdateThrown( TFLOAT a_fDeltaTime );
	void UpdateScared( TFLOAT a_fDeltaTime );
	void UpdateStalk( TFLOAT a_fDeltaTime );
	void UpdateAttack( TFLOAT a_fDeltaTime );

	const AChickenCoopDefenderThiefData& GetData() const { return m_pGame->pThiefData[ m_eKind ]; }

	// The player unit (AChickenCoopDefenderGame::oPlayer)
	AUnit* GetPlayerUnit() const;

public:
	KIND_                         m_eKind;          // 0x98
	TFLOAT                        m_fTimer;         // 0x9c
	STATE_                        m_eState;         // 0xa0
	SPEED_                        m_eSpeed;         // 0xa4
	AInteriorNetwork*             m_pNetwork;       // 0xa8, "chickencoop"
	AChickenCoopDefenderGame*     m_pGame;          // 0xac
	Toshi::TVector4               m_vCoopPosition;  // 0xb0, where it went into the coop
	AUnit*                        m_pCarried;       // 0xc0, hen/egg
	AUnit*                        m_pTarget;        // 0xc4, loose hen/egg
	TFLOAT                        m_fScare;         // 0xc8
	TFLOAT                        m_fSoundTimer;    // 0xcc
	TINT                          m_iSoundCue;      // 0xd0
	TINT                          m_iHitsLeft;      // 0xd4, AChickenCoopDefenderThiefData::iNumHits
	TINT                          m_iMapIcon;       // 0xd8
	TINT                          m_iUnkdc;         // 0xdc
	AChickenCoopDefenderThiefUnk* m_pUnke0;         // 0xe0, never set
	Toshi::TPString8              m_strCarryBone;   // 0xe4
	Toshi::TPString8              m_strCarriedBone; // 0xe8
	Toshi::TVector4               m_vVelocity;      // 0xec, STATE_THROWN
	TINT                          m_iHeadBone;      // 0xfc, "c_head" of the raccoons (BARNYARD_LATEST: and coyotes; Freddy "c_pelvis")
	TINT                          m_iStarsEffect;   // 0x100, "KO_stars" (BARNYARD_LATEST: Freddy "Freddygone", -1 without the Wii particle data)
	AParticleEffectHandle*        m_pStars;         // 0x104
};

TSTATICASSERT( sizeof( AChickenCoopDefenderThief ) == 0x108 );
