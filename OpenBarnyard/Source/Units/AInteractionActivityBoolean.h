#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Base of activities that branch: plays the activity set of the "true" or the
// "false" block (each with its own "postconditions") depending on a condition
// the subclass checks (see ASuggestionActivity).
//-----------------------------------------------------------------------------
class AInteractionActivityBoolean
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AInteractionActivityBoolean, AInteractionActivity );

public:
	AInteractionActivityBoolean();
	~AInteractionActivityBoolean();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual void   UpdateTrue( AInteractionSequence* a_pSequence );       // +0x3c
	virtual void   UpdateFalse( AInteractionSequence* a_pSequence );      // +0x40

protected:
	Toshi::TArray<AInteractionActivity*> m_aTrueActivities;      // 0x08, "true" / "activities"
	ALatchRefList                        m_aTruePostconditions;  // 0x18, "true" / "postconditions"
	Toshi::TArray<AInteractionActivity*> m_aFalseActivities;     // 0x28, "false" / "activities"
	ALatchRefList                        m_aFalsePostconditions; // 0x38, "false" / "postconditions"
	AInteractionActivityState            m_oActivityState;       // 0x48, progress through the activities of the block
};

TSTATICASSERT( sizeof( AInteractionActivityBoolean ) == 0x58 );
