#pragma once
#include "ACharacterUnit.h"
#include "FSM/AFunctionTable.h"

#include <Math/TVector4.h>

class AInteriorNetwork;
class AInteriorSteer;

//-----------------------------------------------------------------------------
// A coyote of the Sharp Squirter minigame (unit type "sharp_coyote", spawned at
// the "coyote" goals by APrecisionSquirt). Its FSM ("sharpcoyoteattack") goes
// after the player; once milked it runs off to the "respawn" goal farthest from
// the player and fades out, then comes back from its spawn position.
// TClass 00709150 (parent ACharacterUnit, size 0x17c), vtables 00721890, 00721880.
//-----------------------------------------------------------------------------
class APrecisionCoyote
    : public ACharacterUnit
{
public:
	TDECLARE_CLASS( APrecisionCoyote, ACharacterUnit );

	static constexpr TINT MAX_RESPAWNS = 10;

	enum STATE_ : TINT
	{
		STATE_FSM,    // 0, the FSM (sharpcoyoteattack) is in control
		STATE_MILKED, // 1, "yuk"
		STATE_FLEE,   // 2, runs to the farthest respawn goal
		STATE_FADE,   // 3, fades out
		STATE_UNK4,   // 4
		STATE_HIDDEN, // 5
		STATE_UNK6,   // 6
		STATE_RETURN, // 7, runs back to its spawn position
	};

public:
	APrecisionCoyote();
	~APrecisionCoyote();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable() OVERRIDE;                     // +0x0c
	virtual TBOOL           Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void            UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// FSM "PlayerAttacked"
	void PlayerAttacked();

	void SetState( STATE_ a_eState );

	// Collects the positions of the "respawn" goals of a_pNetwork
	void SetNetwork( AInteriorNetwork* a_pNetwork );

	// Sets the player the coyote goes after and (re)starts its FSM
	void SetPlayer( AUnit* a_pPlayer );

	// Replaces a_rPosition with the respawn position farthest from a_rcFrom (if it's farther). TFALSE if there are none.
	TBOOL FindFarthestRespawn( const Toshi::TVector4& a_rcFrom, Toshi::TVector4& a_rPosition );

	// TTRUE: the coyote has been hit by the milk. TFALSE: puts it back at its spawn position.
	void SetMilked( TBOOL a_bMilked );

	TBOOL IsMilked() const { return m_bMilked; }

public:
	static AFunctionTable ms_oFunctionTable;

public:
	TBOOL             m_bMilked;                   // 0xa0
	TBOOL             m_bStateChanged;             // 0xa1
	TFLOAT            m_fOpacity;                  // 0xa4
	TFLOAT            m_fUnkA8;                    // 0xa8
	TFLOAT            m_fAttackTimer;              // 0xac, cleared by PlayerAttacked
	AInteriorNetwork* m_pNetwork;                  // 0xb0
	AUnit*            m_pPlayer;                   // 0xb4
	AInteriorSteer*   m_pSteer;                    // 0xb8
	STATE_            m_eState;                    // 0xbc
	TINT              m_iNumRespawns;              // 0xc0
	Toshi::TVector4   m_aRespawns[ MAX_RESPAWNS ]; // 0xc4
	Toshi::TVector4   m_vSpawnPosition;            // 0x164
	TINT              m_iSpawnArea;                // 0x174
	TINT              m_iMapIcon;                  // 0x178
};

TSTATICASSERT( sizeof( APrecisionCoyote ) == 0x17c );
