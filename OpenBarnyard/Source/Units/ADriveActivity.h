#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Changes a drive ("attribute") of the AAnimalAttributes of the (non player)
// units of the category: "set" it, "change" it by a value or at a "rate" per
// second, or start an FSM of the unit's behaviour ("suggestfsm", "commandfsm",
// "schedulefsm"). Lasts "wait" seconds or, with "block", until the FSM is over.
//-----------------------------------------------------------------------------
class ADriveActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ADriveActivity, AInteractionActivity );

	// The drives of AAnimalAttributes (same values as AAnimalAttributes::ATTRIBUTE)
	using ATTRIBUTE = TINT;
	enum ATTRIBUTE_ : ATTRIBUTE
	{
		ATTRIBUTE_NONE = -1,
		ATTRIBUTE_HAPPINESS,  // "happiness"
		ATTRIBUTE_SUGGESTION, // "suggestion"
		ATTRIBUTE_COMMAND,    // "command"
		ATTRIBUTE_SCHEDULE,   // "schedule"
		ATTRIBUTE_NUMOF,
	};

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE         = 0,
		FLAGS_SET          = BITFLAG( 0 ), // "set", or one of the FSMs
		FLAGS_CHANGE       = BITFLAG( 1 ), // "change"
		FLAGS_RATE         = BITFLAG( 2 ), // "rate", changes the drive every update
		FLAGS_COMMAND_FSM  = BITFLAG( 4 ), // "commandfsm"
		FLAGS_SUGGEST_FSM  = BITFLAG( 5 ), // "suggestfsm"
		FLAGS_SCHEDULE_FSM = BITFLAG( 6 ), // "schedulefsm"
		FLAGS_BLOCK        = BITFLAG( 7 ), // "block"
	};

public:
	ADriveActivity();
	~ADriveActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	ATTRIBUTE        m_eAttribute; // 0x08, "attribute"
	FLAGS            m_uiFlags;    // 0x0c
	TFLOAT           m_fValue;     // 0x10, "set", "change" or "rate"
	TFLOAT           m_fWait;      // 0x14, "wait"
	Toshi::TPString8 m_strFSM;     // 0x18, "suggestfsm", "commandfsm" or "schedulefsm"
};

TSTATICASSERT( sizeof( ADriveActivity ) == 0x1c );
