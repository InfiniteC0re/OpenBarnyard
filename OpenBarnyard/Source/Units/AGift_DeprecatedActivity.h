#pragma once
#include "AInteractionActivityBoolean.h"

//-----------------------------------------------------------------------------
// Old gift activity (AGiftComponent): the units of the category give their
// "object" gift away, with "swap" the first two units of the sequence swap it
// (playing the "true" block if the receiver took it, the "false" block
// otherwise) and with "remove" the gift of the unit is deleted. With
// "suggestfsm" the receiver is suggested the FSM and forced into the
// "forcetrigger" / "overridetrigger" interactions.
//-----------------------------------------------------------------------------
class AGift_DeprecatedActivity
    : public AInteractionActivityBoolean
{
public:
	TDECLARE_CLASS( AGift_DeprecatedActivity, AInteractionActivityBoolean );

public:
	AGift_DeprecatedActivity();
	~AGift_DeprecatedActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	// Suggests m_strSuggestFsm to the receiver of the gift and forces it into the trigger interactions.
	void SuggestFsm( AUnit* a_pUnit );

private:
	Toshi::TPString8 m_strObject;          // 0x58, "object"
	TBOOL            m_bSwap;              // 0x5c, "swap"
	TBOOL            m_bRemove;            // 0x5d, "remove"
	TBOOL            m_bSuggestFsm;        // 0x5e, "suggestfsm" is set
	Toshi::TPString8 m_strSuggestFsm;      // 0x60, "suggestfsm"
	Toshi::TPString8 m_strForceTrigger;    // 0x64, "forcetrigger"
	Toshi::TPString8 m_strOverrideTrigger; // 0x68, "overridetrigger"
};

TSTATICASSERT( sizeof( AGift_DeprecatedActivity ) == 0x6c );
