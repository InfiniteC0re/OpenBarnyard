#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Checks the state of the quest "quest": its element "questelement" is done
// ("satisfied"), the quest event of the unit ("event", "satisfied", "thanked"),
// the quest event exists ("exists", "interestweight"), the quest is assigned
// or complete ("given") or the unit gave one of the assigned quests ("givenbyunit").
// TClass 0x0078ae6c, vtable 0x0072a9a0, size 0x5c.
//-----------------------------------------------------------------------------
class AQuestRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AQuestRequirement, AInteractionRequirement );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_SATISFIED       = BITFLAG( 0 ),  // "satisfied" TRUE
		FLAGS_NOT_SATISFIED   = BITFLAG( 1 ),  // "satisfied" FALSE
		FLAGS_THANKED         = BITFLAG( 2 ),  // "thanked" TRUE
		FLAGS_NOT_THANKED     = BITFLAG( 3 ),  // "thanked" FALSE
		FLAGS_EVENT           = BITFLAG( 4 ),  // "event"
		FLAGS_EXISTS          = BITFLAG( 5 ),  // "exists" TRUE
		FLAGS_CHECK_EXISTS    = BITFLAG( 6 ),  // "exists"
		FLAGS_CHECK_GIVEN     = BITFLAG( 7 ),  // "given"
		FLAGS_GIVEN           = BITFLAG( 8 ),  // "given" TRUE
		FLAGS_NOT_GIVEN       = BITFLAG( 9 ),  // "given" FALSE
		FLAGS_INTEREST_WEIGHT = BITFLAG( 10 ), // "interestweight"
		FLAGS_QUEST_ELEMENT   = BITFLAG( 11 ), // "questelement"
		FLAGS_GIVEN_BY_UNIT   = BITFLAG( 12 ), // "givenbyunit"
	};

public:
	AQuestRequirement();
	~AQuestRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	// Lets every unit take every role if a_bAllowed (honours "invert")
	// NOTE: inlined into Evaluate
	void SetAllRoles( TINT a_iNumUnits, TBOOL a_bAllowed );

private:
	Toshi::TPString8 m_strQuest;        // 0x4c, "quest"
	TINT             m_iQuestElement;   // 0x50, "questelement" (1-based)
	FLAGS            m_uiFlags;         // 0x54
	TFLOAT           m_fInterestWeight; // 0x58, "interestweight"
};

TSTATICASSERT( sizeof( AQuestRequirement ) == 0x5c );
