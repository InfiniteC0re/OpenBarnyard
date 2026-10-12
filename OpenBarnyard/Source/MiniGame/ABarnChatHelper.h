#pragma once
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: the three animals chatting on the ground floor of the night barn (FSMMap
// "jersey", Eddy's group): they look at each other and talk, and chat with the
// player (AVOStream "chat" sets) when they're close to Eddy.
// Created by ABarnExploreMinigameState::OnInsertion at night.
// NOTE: the original name is unknown. Size 0x64.
//-----------------------------------------------------------------------------
class ABarnChatHelper
{
public:
	enum ANIM
	{
		ANIM_LOOKAT,
		ANIM_TALK,
		ANIM_IDLE_LP,
		ANIM_NUMOF
	};

	static constexpr TINT MAX_NUM_CHATTERS = 3;

	struct Chatter
	{
		TINT   iState;      // 0x00
		AUnit* pUnit;       // 0x04
		TFLOAT fTimer;      // 0x08
		TCHAR  szName[ 8 ]; // 0x0c, the type name without "nb_" (its VO set)
	};

public:
	ABarnChatHelper( AUnit* a_pPlayer );

	void Update( TFLOAT a_fDeltaTime );
	void AddUnit( AUnit* a_pUnit );
	void Reset();

private:
	void UpdateChatter( TFLOAT a_fDeltaTime, Chatter& a_rChatter, TBOOL a_bNear );

private:
	Toshi::T2Vector<Chatter, MAX_NUM_CHATTERS> m_vecChatters;          // 0x00
	Toshi::TPString8                           m_aAnims[ ANIM_NUMOF ]; // 0x40
	TBOOL                                      m_bTalking;             // 0x4c
	Toshi::TVector4                            m_vecEddyPosition;      // 0x50
	AUnit*                                     m_pPlayer;              // 0x60
};

TSTATICASSERT( sizeof( ABarnChatHelper ) == 0x64 );
