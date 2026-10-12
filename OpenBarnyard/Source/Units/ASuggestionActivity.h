#pragma once
#include "AInteractionActivityBoolean.h"

class PPropertyValueArray;

//-----------------------------------------------------------------------------
// Suggests the FSM "fsm" (with "fsmparams") to the first unit of the category
// through its drive ("drivestrength" sets the "suggestion" attribute of an
// animal). Plays the "true" block if the suggestion was taken, the "false"
// block otherwise.
//-----------------------------------------------------------------------------
class ASuggestionActivity
    : public AInteractionActivityBoolean
{
public:
	TDECLARE_CLASS( ASuggestionActivity, AInteractionActivityBoolean );

public:
	ASuggestionActivity();
	~ASuggestionActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8     m_strFsm;         // 0x58, "fsm"
	PPropertyValueArray* m_pFsmParams;     // 0x5c, "fsmparams", owned
	TFLOAT               m_fDriveStrength; // 0x60, "drivestrength"
};

TSTATICASSERT( sizeof( ASuggestionActivity ) == 0x64 );
