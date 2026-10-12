#pragma once
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// A scarecrow of the Sharp Squirter minigame (unit type "precision_scarecrow",
// spawned in pairs at the "scarecrow" goals by APrecisionSquirt). The standing
// scarecrow carries a pumpkin ("precision_pumpkin") on its head; when the milk
// hits it, it's swapped for the other (knocked over) scarecrow and the pumpkin
// falls off.
// TClass 00709230 (parent AAnimatedUnit, size 0xc4), vtables 00721b88, 00721b7c.
//-----------------------------------------------------------------------------
class APrecisionScarecrow
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( APrecisionScarecrow, AAnimatedUnit );

public:
	APrecisionScarecrow();
	~APrecisionScarecrow();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void  OnDestroy() OVERRIDE;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// The scarecrow keeps facing a_pPlayer
	void SetPlayer( AUnit* a_pPlayer );

	// Puts a_pPumpkin (hidden) on the scarecrow
	void SetPumpkin( AUnit* a_pPumpkin );

	// The scarecrow that replaces this one once it's hit (hidden until then)
	void SetKnockedOver( AUnit* a_pScarecrow );

	// TTRUE: swaps the scarecrow for the knocked over one and drops the pumpkin. TFALSE: shows this one again.
	void SetHit( TBOOL a_bHit );

	TBOOL IsHit() const { return m_bHit; }

public:
	TBOOL           m_bHit;           // 0x98
	TINT            m_iUnk9c;         // 0x9c, unused
	AUnit*          m_pKnockedOver;   // 0xa0
	AUnit*          m_pPlayer;        // 0xa4
	AUnit*          m_pPumpkin;       // 0xa8
	TINT            m_iUnkAC;         // 0xac, unused
	Toshi::TVector4 m_vSpawnPosition; // 0xb0, set by APrecisionSquirt
	TINT            m_iUnkC0;         // 0xc0, unused
};

TSTATICASSERT( sizeof( APrecisionScarecrow ) == 0xc4 );
