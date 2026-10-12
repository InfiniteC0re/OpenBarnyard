#pragma once
#include "AAnimatedUnit.h"
#include "MiniGame/AChasingChicksGame.h"

class AInteriorNetwork;
class AParticleEffectHandle;

//-----------------------------------------------------------------------------
// A chick of the Chasing Chicks minigame (unit type "chasingchicks_chick").
// It wanders around the farmyard network and runs away from the players. A
// player picks it up, carries it and throws it into Hanna's basket in the
// centre (the pen). Chicks landing on a "jump_start" goal jump off the fence.
// Created by AChasingChicksMiniGame, which sets m_pNetwork ("chasingchicks"),
// m_pGame and m_iValue after changing the steer to AInteriorSteer.
//
// TClass 00709320 (parent AAnimatedUnit, size 0xd8), vtables 00722068,
// 0072205c (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AChasingChicksChick
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AChasingChicksChick, AAnimatedUnit );

	enum STATE : TINT
	{
		STATE_FREE,     // 0, wanders around the network
		STATE_FLEEING,  // 1, runs away from m_iPlayer
		STATE_HELD,     // 2, carried by m_iPlayer
		STATE_THROWN,   // 3, flies towards the centre (AThrowItemSteer)
		STATE_JUMPING,  // 4, jumps off a "jump_start" goal or lands after a throw (AThrowItemSteer)
		STATE_LANDED,   // 5, back on the ground (switches to STATE_FREE)
		STATE_POCKETED, // 6, landed in the basket, scored for m_iPlayer
		STATE_RESPAWN,  // 7, respawns at a "spawn" goal (switches to STATE_FREE)
	};

public:
	AChasingChicksChick();
	~AChasingChicksChick();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  OnDestroy() override;                            // +0x60
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	// Switches to a_eState. Returns TFALSE if the chick is respawning.
	TBOOL SetState( STATE a_eState );

	// The player a_iPlayer picks the chick up
	TBOOL PickUp( TINT a_iPlayer );

	// Fades out the trail of a thrown chick
	void StopTrail();

	// Removes the trail of a thrown chick immediately
	void KillTrail();

	STATE GetState() const { return m_eState; }
	TINT  GetPlayer() const { return m_iPlayer; }

private:
	// Plays the trail effect on the tail
	void StartTrail();

	// Jumps off the "jump_start" goal of area a_iArea (if any)
	TBOOL TryJump( TINT a_iArea );

	void UpdateAnimation( TFLOAT a_fDeltaTime );
	void UpdateFree( TFLOAT a_fDeltaTime );
	void UpdateFleeing( TFLOAT a_fDeltaTime );
	void UpdateThrown( TFLOAT a_fDeltaTime );

public:
	TFLOAT                 m_fTimer;        // 0x98, chuckle sound delay in STATE_THROWN, delay before respawning in STATE_POCKETED
	TFLOAT                 m_fFleeTime;     // 0x9c
	AInteriorNetwork*      m_pNetwork;      // 0xa0, set by the minigame
	STATE                  m_eState;        // 0xa4
	TFLOAT                 m_fCheckTimer;   // 0xa8, the free/fleeing chick looks for players every 0.1s
	TINT                   m_iPlayer;       // 0xac, index of the player the chick runs from/is held or thrown by (0: the player)
	AChasingChicksGame*    m_pGame;         // 0xb0, set by the minigame
	AParticleEffectHandle* m_pTrail;        // 0xb4, "chickthrow"
	TINT                   m_iValue;        // 0xb8, score value, set by the minigame (see AChasingChicksGameData::aChickKinds)
	TINT                   m_iMapIcon;      // 0xbc
	TBOOL                  m_bShowOnMap;    // 0xc0
	TINT                   m_iChuckleCue;   // 0xc4
	TINT                   m_iTailBone;     // 0xc8, "CB_Chick_Tail"
	TINT                   m_iThrowEffect;  // 0xcc, "chickthrow"
	TINT                   m_iPickupEffect; // 0xd0, "chickpickup"
	TINT                   m_iPocketEffect; // 0xd4, "chickpocket"
};

TSTATICASSERT( sizeof( AChasingChicksChick ) == 0xd8 );
