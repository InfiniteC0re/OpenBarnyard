#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Waits "period" seconds (or a "random" time in a range). With "quest" it waits
// for the quest to reach "queststate", with "tier" only while the quest
// manager is at that tier. Ends early if a unit of the type "unitexempt" is in
// the category.
//-----------------------------------------------------------------------------
class AWaitActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AWaitActivity, AInteractionActivity );

	using QUESTSTATE = TUINT8;
	enum QUESTSTATE_ : QUESTSTATE
	{
		QUESTSTATE_COMPLETE = 0, // "complete"
		QUESTSTATE_ASSIGNED = 1, // "assigned"
		QUESTSTATE_FAIL     = 2, // "fail"
	};

	static constexpr TUINT8 TIER_ANY = 0xff;

public:
	AWaitActivity();
	~AWaitActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TFLOAT           m_fPeriod;       // 0x08, "period" / "random"
	Toshi::TPString8 m_strUnitExempt; // 0x0c, "unitexempt"
	Toshi::TPString8 m_strQuest;      // 0x10, "quest"
	QUESTSTATE       m_eQuestState;   // 0x14, "queststate"
	TUINT8           m_uiTier;        // 0x15, "tier", TIER_ANY if not set
};

TSTATICASSERT( sizeof( AWaitActivity ) == 0x18 );
