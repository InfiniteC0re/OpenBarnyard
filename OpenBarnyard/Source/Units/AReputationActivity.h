#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Adds ("action" "add") or removes ("remove") the reputation "flags" (see
// AUnitFlagSetsManager::FLAGSET_REPUTATIONS) to / from the AUnitProfileComponent
// of the units of the category, to their second reputation mask with "playerreputation".
//-----------------------------------------------------------------------------
class AReputationActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AReputationActivity, AInteractionActivity );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE              = 0,
		FLAGS_ADD               = BITFLAG( 0 ), // "action" "add"
		FLAGS_REMOVE            = BITFLAG( 1 ), // "action" "remove"
		FLAGS_PLAYER_REPUTATION = BITFLAG( 2 ), // "playerreputation"
	};

public:
	AReputationActivity();
	~AReputationActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TUINT m_uiReputations; // 0x08, "flags"
	FLAGS m_eFlags;        // 0x0c
};

TSTATICASSERT( sizeof( AReputationActivity ) == 0x10 );
