#pragma once
#include "GameInterface/AGameState.h"
#include "HUD/AButtonSlamHudElement.h"
#include "FSM/AFunctionTable.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TPString8.h>
#include <Toshi/TRefCounted.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: the player kicks a tree (the fruits fall out of it and leaves fall
// from its branches), a crate (it breaks and its content is thrown out) or the
// henhouse (an egg is thrown out). Added as a child of the current state by the
// script function "PushState" of the class: -1 kicks what is in front of the
// player, 1 kicks the henhouse.
// TClass 0078392c, vtable 00723250, size 0x5b8.
//-----------------------------------------------------------------------------
class AShakeTreeMicroGame
    : public AGameState
{
public:
	TDECLARE_CLASS( AShakeTreeMicroGame, AGameState );

	enum STATE : TINT
	{
		STATE_PUTTING_AWAY = 0, // waits until the item in the hand is put away
		STATE_KICKING      = 3, // the kick animation plays
		STATE_KICKED       = 4, // the kick landed, waits for the animation to end
	};

	enum MODE : TINT
	{
		MODE_TREE,     // a tree with fruits (ATreeManager::TreeInstance)
		MODE_CRATE,    // a crate (ARegrowthManager::GrowthObject)
		MODE_HENHOUSE, // the henhouse, throws out an egg
	};

public:
	AShakeTreeMicroGame();
	~AShakeTreeMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// NOTE: ProcessInput (+0x0c) is a thunk to AGameState::ProcessInput (0043fc30)
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x68

	// Script native: a_iType -1 kicks the tree (or the crate) in front of the player, 1 the henhouse
	static void PushState( TINT a_iType );

	// Sets up what is kicked. a_pTarget is a ATreeManager::TreeInstance* (MODE_TREE) or an
	// ARegrowthManager::GrowthObject* (MODE_CRATE). a_rcUnitType is the unit spawned for each fruit.
	void Setup( void* a_pTarget, const Toshi::TPString8& a_rcUnitType, TINT a_iNumRegrow, TINT a_iNumSpawn, TFLOAT a_fTargetPower, MODE a_eMode, TINT a_iRegrowthType );

private:
	// Starts the kick (the interact button is pressed again before the item is put away)
	void StartKick();

	// The kick lands: the leaves fall, the crate breaks or the egg is thrown out
	void OnKick();

	// Sets up the spawning of m_iNumSpawn units with a_iNumSpawns over 0.7 seconds
	void StartSpawning( TINT a_iNumSpawns );

	// Spawns one fruit (or the egg) and throws it
	void SpawnUnit();

	// Puts the item in the hand of the player away, returns TTRUE if it is
	TBOOL PutAwayHandItem();

	AUnit* GetPlayer() const { return m_pPlayer ? m_pPlayer->GetUnit() : TNULL; }

private:
	static AFunctionTable ms_oFunctionTable; // 0x0078395c

private:
	Toshi::TPString8        m_strUnitType;    // 0xa4, the fruit
	Toshi::TRef<AUnitProxy> m_pPlayer;        // 0xa8
	STATE                   m_eState;         // 0xac
	MODE                    m_eMode;          // 0xb0
	TFLOAT                  m_fUnkB4;         // 0xb4, never set (see StartKick)
	TFLOAT                  m_fUnkB8;         // 0xb8, 1.5f, never read
	TFLOAT                  m_fPower;         // 0xbc
	TFLOAT                  m_fTargetPower;   // 0xc0
	TFLOAT                  m_fUnkC4;         // 0xc4, -3.0f, never read
	TFLOAT                  m_fSpawnInterval; // 0xc8
	TFLOAT                  m_fSpawnTimer;    // 0xcc
	TINT                    m_iNumToSpawn;    // 0xd0, units left to spawn
	TINT                    m_iNumRegrow;     // 0xd4, fruits the tree regrows once it's shaken
	TINT                    m_iNumSpawn;      // 0xd8
	void*                   m_pTarget;        // 0xdc
	TBOOL                   m_bSpawned;       // 0xe0, a unit was spawned
	TBOOL                   m_bKicked;        // 0xe1, the player can move again
	TINT                    m_iRegrowthType;  // 0xe4
	AButtonSlamHudElement   m_oPowerBar;      // 0xe8
	TINT                    m_iLeavesEffect;  // 0x5b4, "leavesfall"
};

TSTATICASSERT( sizeof( AShakeTreeMicroGame ) == 0x5b8 );
