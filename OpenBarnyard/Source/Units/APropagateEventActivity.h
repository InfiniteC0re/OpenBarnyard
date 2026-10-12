#pragma once
#include "AInteractionActivity.h"

class AInteractionEvent;

//-----------------------------------------------------------------------------
// Lets the units of the "unit" category know about the active interaction
// event of the class "ainteraction<event>event". They learn it at the level a
// unit of the sequence knows it at, which can "mutate" (go up a level, or to
// "forcemutatelevel") with the chance "mutatechance".
//-----------------------------------------------------------------------------
class APropagateEventActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( APropagateEventActivity, AInteractionActivity );

public:
	APropagateEventActivity();
	~APropagateEventActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	// Level a unit of the sequence knows a_pEvent at (see AUnitGameEventComponent), 0 if none knows it.
	TINT8 GetKnowledgeLevel( AInteractionEvent* a_pEvent, AInteractionSequence* a_pSequence );

private:
	AInteractionSequence::CATEGORY m_eUnitCategory;     // 0x08, "unit", category of the units to notify
	const Toshi::TClass*           m_pEventClass;       // 0x0c, "event"
	AInteractionEvent*             m_pEvent;            // 0x10, only set while Update runs
	TFLOAT                         m_fMutateChance;     // 0x14, "mutatechance", 0..1
	TINT8                          m_iForceMutateLevel; // 0x18, "forcemutatelevel", -1 if not set
};

TSTATICASSERT( sizeof( APropagateEventActivity ) == 0x1c );
