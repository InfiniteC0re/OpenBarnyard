#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Clears the list at 0x150 of the main player profile (see
// APlayerSimProfile::ClearSquirtedUnits).
//-----------------------------------------------------------------------------
class ASquirtActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASquirtActivity, AInteractionActivity );

public:
	ASquirtActivity();
	~ASquirtActivity();

	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c
};

TSTATICASSERT( sizeof( ASquirtActivity ) == 0x8 );
