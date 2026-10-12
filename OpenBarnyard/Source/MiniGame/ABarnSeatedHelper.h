#pragma once
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: animates the animals seated in the night barn (FSMMap "seated"):
// they idle, drink or talk, and look at the player when they're close.
// Only the ones of the floor the player is on are updated.
// Created by ABarnExploreMinigameState::OnInsertion at night.
// NOTE: the original name is unknown. Size 0x660.
//-----------------------------------------------------------------------------
class ABarnSeatedHelper
{
public:
	enum ANIM
	{
		ANIM_DRINK,
		ANIM_IDLE,
		ANIM_TALK,
		ANIM_LOOKL,
		ANIM_LOOKR,
		ANIM_NUMOF
	};

	static constexpr TINT MAX_NUM_SEATED = 50;

	struct Seated
	{
		using FLAGS = TUINT8;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE             = 0,
			FLAGS_LOOKED_AT_PLAYER = BITFLAG( 0 ), // it looked at the player
		};

		using STATE = TINT;
		enum STATE_ : STATE
		{
			STATE_START    = 0,
			STATE_IDLE     = 1, // may look at the player once
			STATE_DRINKING = 2,
			STATE_TALKING  = 3,
		};

		STATE  iState;  // 0x00
		AUnit* pUnit;   // 0x04
		TFLOAT fTimer;  // 0x08
		FLAGS  uiFlags; // 0x0c
	};

public:
	ABarnSeatedHelper( AUnit* a_pPlayer );

	void Update( TFLOAT a_fDeltaTime );
	void AddUnit( AUnit* a_pUnit );
	void Reset();

private:
	void UpdateSeated( TFLOAT a_fDeltaTime, Seated& a_rSeated, TBOOL a_bNear );

private:
	Toshi::T2Vector<Seated, MAX_NUM_SEATED> m_vecLoft;              // 0x000, y > 0.1
	Toshi::T2Vector<Seated, MAX_NUM_SEATED> m_vecGround;            // 0x324
	Toshi::TPString8                        m_aAnims[ ANIM_NUMOF ]; // 0x648
	AUnit*                                  m_pPlayer;              // 0x65c
};

TSTATICASSERT( sizeof( ABarnSeatedHelper ) == 0x660 );
