#pragma once
#include "AAnimatedUnit.h"
#include "MiniGame/AVeggiePatchDefenderGame.h"

#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

class AVeggiePatchVeggie;
class AParticleEffectHandle;

//-----------------------------------------------------------------------------
// A raccoon of the Veggie Patch minigame (unit type "veggie_raccoon").
// TClass 00708e90 (parent AAnimatedUnit, size 0x100), vtables 007202a8,
// 00720298 (AGoalCollisionUpdateInterface).
// The ten raccoons are created by AVeggiePatchDefenderGame and live either in
// Patch::oFreeThieves or Patch::oThieves. They come through the gates of the
// fence, grab a veggie and run off with it. Snipers sit on a slingshot
// ("veggie_slingshot") and shoot tomatoes at the player.
//-----------------------------------------------------------------------------
class AVeggiePatchThief
    : public AAnimatedUnit
    , public Toshi::T2DList<AVeggiePatchThief>::Node
{
public:
	TDECLARE_CLASS( AVeggiePatchThief, AAnimatedUnit );

	// AUnit flag set while the raccoon runs around (cleared when it stands, sits on the slingshot or is carried)
	// NOTE: the meaning for AUnit is unknown
	static constexpr FLAGS FLAGS_MOVING = BITFLAG( 9 );

	enum STATE_ : TINT
	{
		STATE_SPAWN,            // 0, placed at "thiefspawn1" outside
		STATE_SEEK_VEGGIE,      // 1
		STATE_CARRY_VEGGIE,     // 2, to the nearest gate
		STATE_WAIT,             // 3, wanders a bit when no open gate was found, then tries again
		STATE_PICKUP,           // 4, "pickup"
		STATE_REMOVED,          // 5, back in oFreeThieves
		STATE_MILKED,           // 6, "yuk_start" / "falloff"
		STATE_SCARED,           // 7, runs away from the player
		STATE_FLEE,             // 8, leaves the patch
		STATE_WANDER,           // 9
		STATE_SEEK_FENCE,       // 10, from outside
		STATE_BREAK_FENCE,      // 11, hits a closed gate
		STATE_OPEN_FENCE,       // 12
		STATE_SLINGSHOT_GETON,  // 13
		STATE_SLINGSHOT_AIM,    // 14
		STATE_SLINGSHOT_FIRE,   // 15
		STATE_SLINGSHOT_GETOFF, // 16
		STATE_PICKED_UP,        // 17, carried by the player
		STATE_THROWN,           // 18
		STATE_PASS_IN,          // 19, jumps through a gate into the patch
		STATE_PASS_OUT_VEGGIE,  // 20, jumps out with a veggie
		STATE_PASS_OUT,         // 21, jumps out
	};

	enum MOVESTATE_ : TINT
	{
		MOVESTATE_IDLE,
		MOVESTATE_WALK,
		MOVESTATE_RUN,
	};

public:
	AVeggiePatchThief();
	~AVeggiePatchThief();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual void           OnEnterSpecialPathArea() OVERRIDE;               // +0x24
	virtual TBOOL          Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void           OnDestroy() OVERRIDE;                            // +0x60
	virtual TBOOL          Think( TFLOAT a_fDeltaTime ) OVERRIDE;           // +0x6c
	virtual Toshi::TClass* GetSteerClass() OVERRIDE;                        // +0x80

	TBOOL SetState( STATE_ a_eState );

	// The raccoon's "pickup" / "putdown" animation breakpoints (AVeggiePatchDefenderGame::OnUnitAction)
	void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent );

	STATE_ GetState() const { return m_eState; }

private:
	void UpdateMoveState( TFLOAT a_fSpeed );
	TINT GetMapIcon();
	void UpdateTomato( TFLOAT a_fDeltaTime );
	void UpdateAnimation( TFLOAT a_fDeltaTime );

	// Finds the nearest gate a raccoon can pass through (iHealth == 0), from the inside or the outside.
	// a_rPosition is the position to start from and receives the position in front of the gate.
	// Without a_bOnlyOpen, a random gate is returned when there is none.
	AVeggiePatchDefenderGame::Fence* FindFence( TBOOL a_bInside, Toshi::TVector4& a_rPosition, TBOOL a_bOnlyOpen );
	AVeggiePatchVeggie*              FindRandomVeggie();

	// Moves the raccoon from the list it is in to a_rList
	void MoveToList( Toshi::T2DList<AVeggiePatchThief>& a_rList );

	// Detaches the slingshot (and stops its skeleton) and the tomato (and hides it) from the raccoon (inlined)
	void DetachSlingshotAndTomato();

	void UpdateSpawn( TFLOAT a_fDeltaTime );
	void UpdateSeekVeggie( TFLOAT a_fDeltaTime );
	void UpdateCarryVeggie( TFLOAT a_fDeltaTime );
	void UpdateWait( TFLOAT a_fDeltaTime );
	void UpdateMilked( TFLOAT a_fDeltaTime );
	void UpdateScared( TFLOAT a_fDeltaTime );
	void UpdateFlee( TFLOAT a_fDeltaTime );
	void UpdateWander( TFLOAT a_fDeltaTime );
	void UpdateSeekFence( TFLOAT a_fDeltaTime );
	void UpdateBreakFence( TFLOAT a_fDeltaTime );
	void UpdateOpenFence( TFLOAT a_fDeltaTime );
	void UpdateSlingshotGetOn( TFLOAT a_fDeltaTime );
	void UpdateSlingshotAim( TFLOAT a_fDeltaTime );
	void UpdateSlingshotFire( TFLOAT a_fDeltaTime );
	void UpdateSlingshotGetOff( TFLOAT a_fDeltaTime );
	void UpdateThrown( TFLOAT a_fDeltaTime );

	// The player unit (Patch::oPlayer)
	AUnit* GetPlayerUnit() const;

	// The veggie held at "cb_rlocator" is m_pVeggie
	TBOOL IsCarryingVeggie();

public:
	// 0x98: T2DList<AVeggiePatchThief>::Node
	AVeggiePatchDefenderGame::Patch* m_pPatch;          // 0xa0
	AVeggiePatchDefenderGame*        m_pGame;           // 0xa4
	STATE_                           m_eState;          // 0xa8
	MOVESTATE_                       m_eMoveState;      // 0xac
	AVeggiePatchVeggie*              m_pVeggie;         // 0xb0, carried
	AVeggiePatchDefenderGame::Fence* m_pFence;          // 0xb4
	TINT                             m_iMapIcon;        // 0xb8
	AUnit*                           m_pTarget;         // 0xbc, of the slingshot
	AAnimatableObject*               m_pSlingshot;      // 0xc0, "veggie_slingshot"
	AUnit*                           m_pTomato;         // 0xc4, "tomato"
	TBOOL                            m_bTomatoDone;     // 0xc8
	Toshi::TVector4                  m_vTomatoVelocity; // 0xcc
	TFLOAT                           m_fTimer;          // 0xdc
	Toshi::TVector4                  m_vVelocity;       // 0xe0, when thrown by the player
	TINT                             m_iHeadBone;       // 0xf0, "c_head"
	TINT                             m_iStarsEffect;    // 0xf4, "KO_stars"
	AParticleEffectHandle*           m_pStars;          // 0xf8
	TINT                             m_iSplashEffect;   // 0xfc, "tomatosplash"
};

TSTATICASSERT( sizeof( AVeggiePatchThief ) == 0x100 );
