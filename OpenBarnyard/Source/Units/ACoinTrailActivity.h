#pragma once
#include "AInteractionActivity.h"
#include "Helpers/ASpline.h"
#include "HUD/ATimeLeftHudElement.h"

#include <File/TTRB.h>

class AUnit;

//-----------------------------------------------------------------------------
// Spawns "count" precision coins along the coin spline
// (Data\treasure\coinspline1.trb) that the player has to collect in "time"
// seconds, showing the time left on the HUD.
//-----------------------------------------------------------------------------
class ACoinTrailActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ACoinTrailActivity, AInteractionActivity );

	static constexpr TINT MAX_NUM_COINS = 100;

public:
	ACoinTrailActivity();
	~ACoinTrailActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c
	virtual TBOOL  IsOverrideable() OVERRIDE { return TFALSE; }           // +0x34, RETURNS_FALSE

private:
	// Removes the coins that are left and the HUD element, restores the HUD flags.
	void Finish();

private:
	TINT                m_iNumCoins;                // 0x08
	AUnit*              m_apCoins[ MAX_NUM_COINS ]; // 0x0c
	TINT                m_iNumCoinsLeft;            // 0x19c, coins that haven't been collected yet
	TINT                m_iCount;                   // 0x1a0, "count"
	TFLOAT              m_fTimeLeft;                // 0x1a4
	TFLOAT              m_fTime;                    // 0x1a8, "time"
	ATimeLeftHudElement m_oTimeLeftHud;             // 0x1ac
	ASpline             m_aSplines[ 2 ];            // 0x3e0, only the first one is used
	Toshi::TTRB         m_oSplineTRB;               // 0x400
	TUINT               m_uiHUDFlags;               // 0x540, flags of the HUD of the current game state before OnStart
};

TSTATICASSERT( sizeof( ACoinTrailActivity ) == 0x544 );
