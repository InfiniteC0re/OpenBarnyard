#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Adds ("action" "add") or removes ("action" "remove") the interest "flags"
// (AUnitFlagSetsManager::FLAGSET_INTERESTS) to the AUnitProfileComponent of the
// units of the category, "playerinterest" picks the player interest mask.
//-----------------------------------------------------------------------------
class AInterestActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AInterestActivity, AInteractionActivity );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE           = 0,
		FLAGS_ADD            = BITFLAG( 0 ), // "action" "add"
		FLAGS_REMOVE         = BITFLAG( 1 ), // "action" "remove"
		FLAGS_PLAYERINTEREST = BITFLAG( 2 ), // "playerinterest"
	};

public:
	AInterestActivity();
	~AInterestActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TUINT m_uiInterests; // 0x08, "flags", mask of AUnitFlagSetsManager::FLAGSET_INTERESTS
	FLAGS m_uiFlags;     // 0x0c
};

TSTATICASSERT( sizeof( AInterestActivity ) == 0x10 );
