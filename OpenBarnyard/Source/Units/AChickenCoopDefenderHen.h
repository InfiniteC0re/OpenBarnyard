#pragma once
#include "AAnimatedUnit.h"
#include "MiniGame/AChickenCoopDefenderGame.h"

class AInteriorNetwork;

//-----------------------------------------------------------------------------
// A hen of Chicken Coop Defender (unit types "chickencoop_hen" and, in Freddy's
// dream, "chickencoop_roast"). The fifteen hens are created by
// AChickenCoopDefender and wait in AChickenCoopDefenderGame::oHenPool (hidden,
// STATE_HOME) until a coyote (or Freddy) takes one out of the coop. Dropped,
// the hen runs away from the thieves around the "chickencoop" network until the
// player picks it up and throws it back into the coop.
//
// TClass 00709460 (parent AAnimatedUnit, size 0xb8), vtables 00722608,
// 007225fc (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AChickenCoopDefenderHen
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AChickenCoopDefenderHen, AAnimatedUnit );

	enum STATE_ : TINT
	{
		STATE_HELD,     // 0, carried by a thief or the player
		STATE_WANDER,   // 1, wanders around the network
		STATE_FLEE,     // 2, runs away from the closest thief (loose)
		STATE_HOME,     // 3, back in the pool (hidden)
		STATE_SCATTER,  // 4, wanders for 10 seconds, then flees
		STATE_THROWN,   // 5, thrown by the player (AChickenCoopDefenderThrowProjectile)
		STATE_LANDED,   // 6, missed the coop: back on the network, loose (switches to STATE_WANDER)
		STATE_POCKETED, // 7, landed in the coop, scores (switches to STATE_HOME)
	};

public:
	AChickenCoopDefenderHen();
	~AChickenCoopDefenderHen();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	TBOOL SetState( STATE_ a_eState );

	// Shows/hides the icon of the hen on the minimap (created on demand)
	void SetMapIconVisible( TBOOL a_bVisible );

	STATE_ GetState() const { return m_eState; }

	void SetNetwork( AInteriorNetwork* a_pNetwork ) { m_pNetwork = a_pNetwork; }
	void SetGame( AChickenCoopDefenderGame* a_pGame ) { m_pGame = a_pGame; }
	void SetValue( TINT a_iValue ) { m_iValue = a_iValue; }

private:
	void UpdateAnimation( TFLOAT a_fDeltaTime );

	// Finds the closest thief within a_fRadius (m_pThreat)
	void FindThreat( TFLOAT a_fRadius );

	void UpdateWander( TFLOAT a_fDeltaTime );
	void UpdateFlee( TFLOAT a_fDeltaTime );
	void UpdateScatter( TFLOAT a_fDeltaTime );
	void UpdateThrown( TFLOAT a_fDeltaTime );

	// Moves the hen from a_rFrom to the pool (STATE_HOME)
	void ReturnToPool( AChickenCoopDefenderGame::UnitList15& a_rFrom );

	// Is the camera of the player following this hen?
	TBOOL IsFollowedByCamera();

public:
	AInteriorNetwork*         m_pNetwork;      // 0x98, "chickencoop"
	STATE_                    m_eState;        // 0x9c
	AChickenCoopDefenderGame* m_pGame;         // 0xa0
	TFLOAT                    m_fTimer;        // 0xa4, STATE_FLEE, STATE_SCATTER, STATE_POCKETED
	TINT                      m_iMapIcon;      // 0xa8
	AUnit*                    m_pThreat;       // 0xac, the closest thief
	TINT                      m_iValue;        // 0xb0, of the hot spot it was thrown from
	TINT                      m_iPocketEffect; // 0xb4, "chickpocket"
};

TSTATICASSERT( sizeof( AChickenCoopDefenderHen ) == 0xb8 );
