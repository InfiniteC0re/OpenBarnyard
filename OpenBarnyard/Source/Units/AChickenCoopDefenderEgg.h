#pragma once
#include "AAnimatedUnit.h"
#include "MiniGame/AChickenCoopDefenderGame.h"

//-----------------------------------------------------------------------------
// An egg of Chicken Coop Defender (unit type "chickencoop_egg"). The fifteen
// eggs are created by AChickenCoopDefender and wait in
// AChickenCoopDefenderGame::oEggPool (hidden) until a raccoon takes one out of
// the coop. Dropped, the egg bounces on the ground until the player picks it
// up and throws it back into the coop.
//
// TClass 007094a0 (parent AAnimatedUnit, size 0xbc), vtables 007226e8,
// 007226dc (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AChickenCoopDefenderEgg
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AChickenCoopDefenderEgg, AAnimatedUnit );

	enum STATE_ : TINT
	{
		STATE_IDLE,     // 0, in the pool, carried or lying on the ground
		STATE_THROWN,   // 1, flies to the coop ("Goal1")
		STATE_BOUNCING, // 2, dropped by a raccoon
	};

public:
	AChickenCoopDefenderEgg();
	~AChickenCoopDefenderEgg();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override; // +0x6c

	// Shows/hides the icon of the egg on the minimap (created on demand)
	void SetMapIconVisible( TBOOL a_bVisible );

	// Drops the egg at a_rcPosition, it bounces away in a random direction
	void Drop( const Toshi::TVector4& a_rcPosition );

	// Throws the egg at a_rcVelocity
	void Throw( const Toshi::TVector4& a_rcVelocity );

	STATE_ GetState() const { return m_eState; }

	void SetGame( AChickenCoopDefenderGame* a_pGame ) { m_pGame = a_pGame; }
	void SetValue( TINT a_iValue ) { m_iValue = a_iValue; }

private:
	// Is the camera of the player following this egg?
	TBOOL IsFollowedByCamera();

	// Moves the egg from the carried eggs to a_rTo
	void MoveFromCarried( AChickenCoopDefenderGame::UnitList15& a_rTo );

public:
	AChickenCoopDefenderGame* m_pGame;     // 0x98
	TINT                      m_iMapIcon;  // 0x9c
	STATE_                    m_eState;    // 0xa0
	Toshi::TVector4           m_vVelocity; // 0xa4
	TINT                      m_iValue;    // 0xb4, of the hot spot it was thrown from
	TFLOAT                    m_fTimer;    // 0xb8, the camera keeps looking at the coop after the egg landed in it
};

TSTATICASSERT( sizeof( AChickenCoopDefenderEgg ) == 0xbc );
