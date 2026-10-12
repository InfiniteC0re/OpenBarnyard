#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Assigns the quest "name" to the player, given by the unit of the category,
// or with "forcestate" completes or fails the assigned quest of that name (or
// given by the unit). "thank" / "thankonbehalf" quests aren't assigned.
//-----------------------------------------------------------------------------
class AQuestActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AQuestActivity, AInteractionActivity );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_THANK           = BITFLAG( 0 ), // "thank"
		FLAGS_THANK_ON_BEHALF = BITFLAG( 1 ), // "thankonbehalf"
	};

	using FORCESTATE = TUINT8;
	enum FORCESTATE_ : FORCESTATE
	{
		FORCESTATE_COMPLETED = 0, // "completed"
		FORCESTATE_FAILED    = 1, // "failed"
		FORCESTATE_NONE      = 3,
	};

public:
	AQuestActivity();
	~AQuestActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

	// TRUE if the activity assigns its quest (doesn't force a state nor thank)
	// $Barnyard: FUNCTION 005bbbd0
	TBOOL IsAssigningQuest() const { return m_eForceState == FORCESTATE_NONE && !( m_eFlags & ( FLAGS_THANK | FLAGS_THANK_ON_BEHALF ) ); }

private:
	Toshi::TPString8 m_strName;     // 0x08, "name"
	FLAGS            m_eFlags;      // 0x0c
	FORCESTATE       m_eForceState; // 0x0d, "forcestate"
};

TSTATICASSERT( sizeof( AQuestActivity ) == 0x10 );
