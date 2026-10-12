#pragma once
#include "AInteractionActivity.h"
#include "Helpers/AGameTimeManager.h"

//-----------------------------------------------------------------------------
// Activity of the game time ("action": "wait", "setto" or "advancephase").
// With "wait" it waits until the day phase is "phase".
//-----------------------------------------------------------------------------
class AGameTimeActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AGameTimeActivity, AInteractionActivity );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_WAIT          = BITFLAG( 0 ), // "wait"
		FLAGS_SET_TO        = BITFLAG( 1 ), // "setto"
		FLAGS_ADVANCE_PHASE = BITFLAG( 2 ), // "advancephase"
	};

public:
	AGameTimeActivity();
	~AGameTimeActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	FLAGS                      m_uiFlags;   // 0x08, "action"
	AGameTimeManager::DAYPHASE m_eDayPhase; // 0x0c, "phase", -1 if unknown
};

TSTATICASSERT( sizeof( AGameTimeActivity ) == 0x10 );
