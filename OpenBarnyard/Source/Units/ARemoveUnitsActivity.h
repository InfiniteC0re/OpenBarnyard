#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Makes the first unit of the category leave the sequence.
//-----------------------------------------------------------------------------
class ARemoveUnitsActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ARemoveUnitsActivity, AInteractionActivity );

public:
	ARemoveUnitsActivity();
	~ARemoveUnitsActivity();

	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c
};

TSTATICASSERT( sizeof( ARemoveUnitsActivity ) == 0x8 );
