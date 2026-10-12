#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The event "event" (AInteraction<event>Event) is ("occuring") or isn't going
// on. The weight is the event's interest. Participants can be filtered by
// whether they know about the event ("unitknows") and by the knowledge level
// they have of it ("mutatelevel").
// TClass 0x0078af2c, vtable 0x0072aad4, size 0x60.
//-----------------------------------------------------------------------------
class ARumourRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ARumourRequirement, AInteractionRequirement );

	using UNITKNOWS = TINT;
	enum UNITKNOWS_ : UNITKNOWS
	{
		UNITKNOWS_YES      = 0, // "unitknows" true
		UNITKNOWS_NO       = 1, // "unitknows" false
		UNITKNOWS_DONTCARE = 2,
	};

public:
	// NOTE: inlined into CreateObject
	ARumourRequirement()
	    : m_bOccuring( TTRUE )
	    , m_fKnowledge( 0.0f )
	    , m_pEventClass( TNULL )
	    , m_eUnitKnows( UNITKNOWS_DONTCARE )
	    , m_iMutateLevel( -1 )
	{
		m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;
	}

	~ARumourRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TBOOL                m_bOccuring;    // 0x4c, "occuring"
	TFLOAT               m_fKnowledge;   // 0x50, "knowledge", never used
	const Toshi::TClass* m_pEventClass;  // 0x54, "event"
	UNITKNOWS            m_eUnitKnows;   // 0x58, "unitknows"
	TINT8                m_iMutateLevel; // 0x5c, "mutatelevel", -1 for any
};

TSTATICASSERT( sizeof( ARumourRequirement ) == 0x60 );
