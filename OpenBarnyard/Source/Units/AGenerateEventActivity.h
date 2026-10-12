#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Throws the interaction event "event" (class AInteraction<event>Event) and
// lets the units of the category know of it. With "forceinteractionperiod"
// the units are forced into the rumour interactions for that long.
//-----------------------------------------------------------------------------
class AGenerateEventActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AGenerateEventActivity, AInteractionActivity );

public:
	AGenerateEventActivity();
	~AGenerateEventActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	const Toshi::TClass* m_pEventClass;             // 0x08, "event", class of the AInteractionEvent
	TFLOAT               m_fForceInteractionPeriod; // 0x0c, "forceinteractionperiod"
	TBOOL                m_bCreateNew;              // 0x10, "createnew", always create a new event
};

TSTATICASSERT( sizeof( AGenerateEventActivity ) == 0x14 );
