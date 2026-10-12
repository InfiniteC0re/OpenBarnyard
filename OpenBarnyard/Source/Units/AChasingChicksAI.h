#pragma once
#include "AUnitPlayer.h"
#include "AChasingChicksChick.h"
#include "Helpers/AMilkBottlePickups.h"
#include "MiniGame/AChasingChicksGame.h"
#include "Sound/ASoundLegacy.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkPath.h"

class AInteriorNetwork;
class ASimplePlayerControlComponent;

//-----------------------------------------------------------------------------
// Purpose: An AI rival of Chasing Chicks (unit type "chasingchicks_ai").
// It drives its ASimplePlayerControlComponent like the player would: it walks
// along a path of the "chasingchicks" network, catches chicks, carries them to
// a pen and throws them in, and rams/squirts rivals or picks up milk bottles
// depending on its mood (how far it leads or trails).
//
// TClass 0x007092e0 (parent AUnitPlayer, size 0x5e0), vtable 0x00721f58,
// secondary vtable 0x00721f4c.
//-----------------------------------------------------------------------------
class AChasingChicksAI
    : public AUnitPlayer
{
public:
	TDECLARE_CLASS( AChasingChicksAI, AUnitPlayer );

	enum STATE : TINT
	{
		STATE_SEARCH,      // picks a chick, a rival or a milk bottle depending on the mood
		STATE_CHASE_CHICK, // follows a path to m_pTargetChick
		STATE_GRAB_CHICK,  // walks straight to m_pTargetChick
		STATE_CHASE_RIVAL, // follows a path to m_pTargetRival, squirts it when possible
		STATE_RAM_RIVAL,   // walks straight to m_pTargetRival and rams it
		STATE_GO_TO_PEN,   // carries the chick to m_pTargetPen
		STATE_AIM,         // turns to the centre and throws the chick
		STATE_AIM_SQUIRT,  // same as STATE_AIM with the secondary launcher (never entered in retail)
		STATE_GET_MILK,    // follows a path to m_pTargetBottle
		STATE_NUMOF
	};

	enum MOOD : TINT
	{
		MOOD_NORMAL,
		MOOD_AGGRESSIVE, // trails, goes for rivals more
		MOOD_DEFENSIVE,  // leads, attacks any rival
		MOOD_LOW_AMMO,   // looks for milk bottles first
	};

public:
	AChasingChicksAI();
	~AChasingChicksAI();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	// Connects the AI to the network it walks on (the "chasingchicks" network) and starts searching.
	// Called by the minigame once m_pGame and m_pPlayer are set.
	void SetNetwork( AInteriorNetwork* a_pNetwork );

	// Called when a projectile of this AI hits a_rcUnit: taunts the player and goes for the chick they held.
	void OnHitUnit( const Toshi::TRef<AUnitProxy>& a_rcUnit );

	// NOTE: inlined in retail (the minigame writes the fields directly)
	void SetGame( AChasingChicksGame* a_pGame, AChasingChicksPlayer* a_pPlayer )
	{
		m_pGame   = a_pGame;
		m_pPlayer = a_pPlayer;
	}

	AChasingChicksChick* GetTargetChick() const { return m_pTargetChick; }

private:
	TBOOL SetState( STATE a_eState );
	void  UpdateMood( TFLOAT a_fDeltaTime );
	void  UpdateStuck( TFLOAT a_fElapsedTime );

	// Returns the free chick to go for (other AI rivals' targets weigh more) within 40 units.
	// a_rfDistance is set to the distance of the chick if one is found.
	AChasingChicksChick* FindChick( TFLOAT& a_rfDistance );

	// Returns a rival to attack within a_rfDistance (see the NOTE about the retail bug).
	AUnit* FindRival( TFLOAT& a_rfDistance );

	// NOTE: inlined in retail
	AChasingChicksHotSpot* PickPen();

	// NOTE: inlined in retail: copies the next point of the path to m_vNextPoint
	void UpdateNextPoint() { m_vNextPoint = m_oPath.m_oPath.m_vPosition; }

	// Sets the movement input of m_pControl (NOTE: ASimplePlayerControlComponent method 00508060)
	void SetMovementInput( TFLOAT a_fX, TFLOAT a_fY );

	void SendCommand( AInputCommand a_eCommand );

	void UpdateSearch( TFLOAT a_fDeltaTime );
	void UpdateChaseChick( TFLOAT a_fDeltaTime );
	void UpdateGrabChick( TFLOAT a_fDeltaTime );
	void UpdateChaseRival( TFLOAT a_fDeltaTime );
	void UpdateRamRival( TFLOAT a_fDeltaTime );
	void UpdateGoToPen( TFLOAT a_fDeltaTime );
	void UpdateAim( TFLOAT a_fDeltaTime );
	void UpdateGetMilk( TFLOAT a_fDeltaTime );
	void UpdateAimSquirt( TFLOAT a_fDeltaTime );

private:
	STATE                          m_eState;          // 0xb0
	MOOD                           m_eMood;           // 0xb4
	AChasingChicksGame*            m_pGame;           // 0xb8
	AChasingChicksPlayer*          m_pPlayer;         // 0xbc, slot of this AI in m_pGame->aPlayers
	AInteriorNetwork*              m_pNetwork;        // 0xc0
	AInteriorNetworkPath           m_oPath;           // 0xc4
	Toshi::TVector4                m_vNextPoint;      // 0x580, point of the path walked to
	Toshi::TVector4                m_vLastPosition;   // 0x590, see UpdateStuck
	TINT                           m_iNumStuck;       // 0x5a0, consecutive stuck checks (jumps)
	TFLOAT                         m_fStuckCheckTime; // 0x5a4, every 0.5s
	TFLOAT                         m_fStuckElapsed;   // 0x5a8
	AChasingChicksChick*           m_pTargetChick;    // 0x5ac
	AUnit*                         m_pTargetRival;    // 0x5b0
	AChasingChicksHotSpot*         m_pTargetPen;      // 0x5b4
	AMilkBottlePickups::Bottle*    m_pTargetBottle;   // 0x5b8
	TFLOAT                         m_fAttackCooldown; // 0x5bc
	TINT                           m_iUnk5C0;         // 0x5c0, unused
	TFLOAT                         m_fDefensiveness;  // 0x5c4, grows while leading
	TFLOAT                         m_fAggressiveness; // 0x5c8, grows while trailing
	TFLOAT                         m_fDecisionTime;   // 0x5cc, decisions are taken every 0.1s
	ASimplePlayerControlComponent* m_pControl;        // 0x5d0
	TINT                           m_iMapIcon;        // 0x5d4
	ASoundId                       m_iTauntSound;     // 0x5d8
	TINT                           m_iTauntCue;       // 0x5dc
};

TSTATICASSERT( sizeof( AChasingChicksAI ) == 0x5e0 );
