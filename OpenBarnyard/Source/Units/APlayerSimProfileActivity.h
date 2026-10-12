#pragma once
#include "AInteractionActivity.h"

class AGameState;

//-----------------------------------------------------------------------------
// Changes a value of the main player profile (APlayerSimProfile): adds "value"
// to the "energy" (stamina), "energylimit" (max stamina) or "money" (pick ups,
// showing the barn reward screen), or fills the player's milk ("powerup_full_milk").
//-----------------------------------------------------------------------------
class APlayerSimProfileActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( APlayerSimProfileActivity, AInteractionActivity );

	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_ADD     = 0, // "add"
		MODE_UNKNOWN = 1, // any other "mode", the activity does nothing
	};

public:
	APlayerSimProfileActivity();
	~APlayerSimProfileActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	MODE m_eMode; // 0x08, "mode"
	union
	{
		TFLOAT m_fValue; // 0x0c, "value" of "energy", "energylimit" and "powerup_full_milk"
		TINT   m_iValue; // 0x0c, "value" of "money"
	};
	Toshi::TPString8 m_strType;      // 0x10, "type"
	AGameState*      m_pRewardState; // 0x14, ABarnRewardState pushed for "money", the activity runs while it's on
};

TSTATICASSERT( sizeof( APlayerSimProfileActivity ) == 0x18 );
